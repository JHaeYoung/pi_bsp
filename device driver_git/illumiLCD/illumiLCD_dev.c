#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/pwm.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include "ioctl.h"

#define DEBUG 1
#define DRIVER_NAME "illumiLCD"
#define DRIVER_CLASS "illumiLCDClass"

/* Defines for device identification */ 
#define I2C_BUS_AVAILABLE	1		/* The I2C Bus available on the raspberry */
#define SLAVE_DEVICE_NAME	"bh1750"	/* Device and Driver Name */
#define bh1750_SLAVE_ADDRESS	0x23		/* bh1750 I2C address */

#define BH1750_POWER_DOWN 0x00
#define BH1750_POWER_ON 0x01
#define BH1750_RESET 0x07
#define BH1750_CONTINUOUS_MEASUREMENT 0x10
#define Continuously_H_Resolution_Mode 0x10
#define Continuously_L_Resolution_Mode 0x13

#define MAXDATA 65535
#define INITPWM 500
#define PERIOD 1000

DECLARE_WAIT_QUEUE_HEAD(WaitQueue_Read);


/* i2c init */
static struct i2c_adapter * bh1750_i2c_adapter = NULL;
static struct i2c_client * bh1750_i2c_client = NULL;
static const struct i2c_device_id bh1750_id[] = {
		{ SLAVE_DEVICE_NAME, 0 }, 
		{ }
};
static struct i2c_driver bh1750_driver = {
	.driver = {
		.name = SLAVE_DEVICE_NAME,
		.owner = THIS_MODULE
	},
	.id_table = bh1750_id,
};
static struct i2c_board_info bh1750_i2c_board_info = {
	I2C_BOARD_INFO(SLAVE_DEVICE_NAME, bh1750_SLAVE_ADDRESS)	
};
MODULE_DEVICE_TABLE(i2c, bh1750_id);

/* dev init */
static dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

/* PWM init */
struct pwm_device *pwm0 = NULL;
static unsigned long pwm_value;
static int pwmCal =0;
/* timer_val init */
static int timer_val = 100; //f=100HZ, T=1/100 = 10ms, 100*10ms = 1Sec
struct timer_list timerLed;
static void kerneltimer_registertimer(unsigned long timeover);
static void kerneltimer_func(struct timer_list *t);

/* lcd_work init*/
static struct workqueue_struct *lcd_workqueue;
static struct work_struct lcd_work;

/* gpio_irq init*/
static int sw_irq[3] = {0};
#define GPIOKEYCNT 3
#define gpioName(a,b) #a#b     //"led""0" == "led0"
static int gpio_key[GPIOKEYCNT] = {16,20,21};
static char sw_no = 0;

unsigned int gpio_lcd[] = {
	24, /* Enable Pin */
	18, /* Register Select Pin */
	6, /* Data Pin 0*/
	17, /* Data Pin 1*/
	27, /* Data Pin 2*/
	22, /* Data Pin 3*/
	10, /* Data Pin 4*/
	9, /* Data Pin 5*/
	11, /* Data Pin 6*/
	5, /* Data Pin 7*/
};
#define REGISTER_SELECT gpio_lcd[1]

void lcd_enable(void) {
	gpio_set_value(gpio_lcd[0], 1);
	msleep(5);
	gpio_set_value(gpio_lcd[0], 0);
}
/*
 * @brief set the 8 bit data bus
 * @param data: Data to set
 */

static int gpioKeyInit(void)
{
    int i;
    int ret=0;;
    for(i=0;i<GPIOKEYCNT;i++)
    {
        ret = gpio_request(gpio_key[i], gpioName(key,i));
        if(ret < 0) {
            printk("Failed Request gpio%d error\n", 6);
            return ret;
        }
    }
    for(i=0;i<GPIOKEYCNT;i++)
    {
        ret = gpio_direction_input(gpio_key[i]);
        if(ret < 0) {
            printk("Failed direction_output gpio%d error\n", 6);
     return ret;
        }
    }
    return ret;
}

static void gpioKeyFree(void)
{
    int i;
    for(i=0;i<GPIOKEYCNT;i++)
    {
        gpio_free(gpio_key[i]);
    }
}

static void gpioKeyToIrq(void)
{
    int i;
    for (i = 0; i < GPIOKEYCNT; i++) {
    sw_irq[i] = gpio_to_irq(gpio_key[i]);
    }
}

static void gpioKeyFreeIrq(void)
{
    int i;
    for (i = 0; i < GPIOKEYCNT; i++){
        free_irq(sw_irq[i],NULL);
    }
}

void lcd_send_byte(char data) {
	int i;
	for(i=0; i<8; i++)
		gpio_set_value(gpio_lcd[i+2], ((data) & (1<<i)) >> i);
	lcd_enable();
	msleep(5);
}

void lcd_command(uint8_t data) {
 	gpio_set_value(REGISTER_SELECT, 0);	/* RS to Instruction */
	lcd_send_byte(data);
}

void lcd_data(uint8_t data) {
 	gpio_set_value(REGISTER_SELECT, 1);	/* RS to data */
	lcd_send_byte(data);
}

static int bh1750_init(struct i2c_client *client)
{
	int ret;	
    //Send the power-on command to the BH1750 sensor
    char cmd_power_on = BH1750_POWER_ON;
	char cmd_set_mode = Continuously_H_Resolution_Mode;
	//char cmd_power_on = BH1750_POWER_DOWN;
    ret = i2c_master_send(client, &cmd_power_on, 1);
    if (ret < 0) {
        printk(KERN_ERR "Error sending power-on command to BH1750 sensor\n");
        return ret;
    }
    // Set the measurement mode to high-resolution mode    
	ret = i2c_master_send(client, &cmd_set_mode, 1);
    if (ret < 0) {
        printk(KERN_ERR "Error setting measurement mode of BH1750 sensor\n");
        return ret;
    }
    return 0;
}

irqreturn_t sw_isr(int irq, void *unuse)
{
    int i;
	for(i=0;i<GPIOKEYCNT;i++)
    {
        if(irq == sw_irq[i])
        {
            sw_no = i+1;			
            break;
        }
    }
    printk("IRQ : %d, sw_no : %d\n",irq,sw_no);	
    return IRQ_HANDLED;
}

static int requestIrqInit(struct file *file)
{
	int result= 0;
	
    for(int i=0;i<GPIOKEYCNT;i++)
    {
        result = request_irq(sw_irq[i],sw_isr,IRQF_TRIGGER_RISING,gpioName(key,i),NULL);		 
        if(result)
        {
            printk("#### FAILED Request irq %d. error : %d \n", sw_irq[i], result);
            break;
        }
    }	
	return 0;
}

static int illumiLCD_open(struct inode *inode, struct file *file)
{
	int *bh1750_data=NULL;	
	bh1750_data = kmalloc(sizeof(char),GFP_KERNEL);	
	if(!bh1750_data) 
		return -ENOMEM;	
	file->private_data = bh1750_data;
	
	gpioKeyInit();
	gpioKeyToIrq();
	requestIrqInit(file);
	kerneltimer_registertimer(timer_val);
	return 0;
}


static ssize_t bh1750_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int ret;
    char data[2];	
	int *bh1750_data = file->private_data;
    // BH1750 센서로부터 데이터 읽기
	
    ret = i2c_master_recv(bh1750_i2c_client, data, 2);
    if (ret < 0) {
        printk(KERN_ERR "Error reading data from BH1750 sensor\n");
        return ret;
    }	
	// Wait for conversion to complete
    msleep(180);

    // 데이터를 16비트로 변환하여 bh1750_data에 저장
    *bh1750_data = (data[0] << 8) | data[1];
	pwm_value = *bh1750_data;
    // 사용자 공간으로 데이터 전송
    ret = copy_to_user(buf, bh1750_data, sizeof(bh1750_data));
    if (ret != 0) {
        printk(KERN_ERR "Failed to copy data to user space\n");
        return -EFAULT;
    }	
	printk("bh1750_data : %d\n",*bh1750_data);
	
	
    return sizeof(bh1750_data);
}

char *custom_strcat(const char *str1, const char *str2) {
    size_t len1 = 0;
	size_t len2 = 0;
	size_t sumlen = 0;
	char *result;
    while (str1[len1] != '\0') {
        len1++;
    }
    
    while (str2[len2] != '\0') {
        len2++;
    }
	sumlen = len1+len2;
    result = kmalloc(len1 + len2 + 1, GFP_KERNEL); 
    if (!result) {
        return NULL;
    }
    for (size_t i = 0; i < len1; i++) {
        result[i] = str1[i];
    }
    for (size_t i = 0; i < len2; i++) {
        result[len1 + i] = str2[i];
    }
    result[len1 + len2] = '\0';

    return result;
}

char custom_strlen(const char *str) {
    size_t length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}


static void lcd_work_func(struct work_struct *work) {
	
	const  char *sub = "PWM :";
	const  char *per = "%";
	char *result;
	char pwm_per;
	char pwm_str[10];
	int len;	
	
	pwm_per = ((pwm_value*2*100)/(MAXDATA))/2;
	printk("pwm_value: %d\n", pwm_value);
	printk("pwm_per: %d%%\n", pwm_per);
	/* lcd_write */	
	lcd_command(0x1);
	snprintf(pwm_str, sizeof(pwm_str), "%d", pwm_per);
	pwm_str[sizeof(pwm_str)-1] = '\0';	
	
	result= custom_strcat(sub, pwm_str);
	result= custom_strcat(result, per);
	/*
    if (!result) {
        printk(KERN_ERR "Memory allocation error\n");
        return -ENOMEM;
    }	*/
	len = custom_strlen(result);
	/* Set the new data to the display */
	for(int i=0; i<len; i++)
		lcd_data(result[i]);	

}

static int illumiLCD_release(struct inode *inode, struct file *file)
{	
	gpioKeyFree();
	gpioKeyFreeIrq();
	if(file->private_data) 
		kfree(file->private_data);
	if(timer_pending(&timerLed)) //타이머가 실행하고 있는지 확인 
        del_timer(&timerLed);
	printk("illumiLCD closed.\n");
	return 0;
}

static unsigned int illumiLCD_poll(struct file * file, struct poll_table_struct * wait)
{
	unsigned int mask = 0;   //내가 리턴해야할 값
	printk("_key : %u \n",(wait->_key & POLLIN));
	if(wait->_key & POLLIN)
		poll_wait(file, &WaitQueue_Read, wait); //이 친구도 블로킹 함수
	if(sw_no > 0)
		mask = POLLIN;
	return mask;
}

static long illumiLCD_ioctl (struct file *file, unsigned int cmd, unsigned long arg)
{
	illumiLCD_data info = {0};
#if DEBUG
    //printk( "ledkeydev ioctl -> cmd : %08X, arg : %08X \n", cmd, (unsigned int)arg );
#endif
	int err, size;
	
	if( _IOC_TYPE( cmd ) != IOCTLTEST_MAGIC ) return -EINVAL;
	if( _IOC_NR( cmd ) >= IOCTLTEST_MAXNR ) return -EINVAL;
	size = _IOC_SIZE( cmd );
    if( size )
    {
        err = 0;
        if( _IOC_DIR( cmd ) & _IOC_READ ) // 방향 즉 read인지 write인지 확인하는 함수
//              err = access_ok( VERIFY_WRITE, (void *) arg, size );
            err = access_ok( (void *) arg, size ); // 아무 문제 없으면 0보다 큰값이 리턴
        if( _IOC_DIR( cmd ) & _IOC_WRITE )
//              err = access_ok( VERIFY_READ , (void *) arg, size );
            err = access_ok( (void *) arg, size );
			// 이 주소부터 size만큼의 공간에 접근 가능한지 체크하는 함수           
        if( !err ) return err;
    }		
	switch( cmd )
    {
    case TIMER_START :
		if(!timer_pending(&timerLed))
			kerneltimer_registertimer(timer_val);		
        break;
    case TIMER_STOP :
		if(timer_pending(&timerLed)) //타이머가 실행하고 있는지 확인 
			del_timer(&timerLed);	
        break;
	case PWM_VALUE :				
		err = copy_from_user((void *)&info,(void *)arg,(unsigned long)sizeof(info));	
		pwmCal= info.pwm_val;		
		pwm_config(pwm0, pwmCal , PERIOD);	
		pwm_value = pwmCal*MAXDATA/PERIOD;
		queue_work(lcd_workqueue, &lcd_work);
		break;	
	case KEYVAL_READ :
		info.key_no = sw_no;
		err = copy_to_user((void *)arg,(const void *)&info,(unsigned long)sizeof(info));
		break;	
	default:
		err =-E2BIG;
		break;
	}	
	
    return err;
}


static void kerneltimer_registertimer(unsigned long timeover)
{
    timer_setup( &timerLed,kerneltimer_func,0);
    timerLed.expires = get_jiffies_64() + timeover;  //10ms *100 = 1sec
    add_timer( &timerLed );
}
static void kerneltimer_func(struct timer_list *t )
{	
	pwmCal = (pwm_value*2*PERIOD)/MAXDATA/2;
	
#if DEBUG
	printk("pwmCal : %d\n",pwmCal);
#endif
	pwm_config(pwm0, pwmCal , PERIOD);
	
	queue_work(lcd_workqueue, &lcd_work);
	
    mod_timer(t,get_jiffies_64() + timer_val);
		
	
}

static struct file_operations fops =
{
  .owner          = THIS_MODULE,
  .open			  = illumiLCD_open,
  .read           = bh1750_read,
  //.write 		  = pwm_lcd_write,	
  .unlocked_ioctl = illumiLCD_ioctl,
  .poll     	  = illumiLCD_poll,
  .release        = illumiLCD_release,
};



static int __init illumiLCDInit(void) {	
	int ret =0;	
	int i;
	char *names[] = {"ENABLE_PIN", "REGISTER_SELECT", "DATA_PIN0", "DATA_PIN1", "DATA_PIN2", "DATA_PIN3", "DATA_PIN4", "DATA_PIN5", "DATA_PIN6", "DATA_PIN7"};
	/*Allocating Major number*/
	if((alloc_chrdev_region(&dev, 0, 1, "bh1750")) <0){
		pr_err("Cannot allocate major number\n");
	return -1;
	}
	pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

	/*Creating cdev structure*/
	cdev_init(&etx_cdev,&fops);

	/*Adding character device to the system*/
	if((cdev_add(&etx_cdev,dev,1)) < 0){
		pr_err("Cannot add the device to the system\n");
	return -1;
	}
	/*Creating struct class*/
	if((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL){
		pr_err("Cannot create the struct class\n");
	return -1;
	}
	/* create device file */
	if(device_create(dev_class, NULL, dev, NULL, DRIVER_NAME) == NULL) {
		printk("Can not create device file!\n");
		goto FileError;
	}	
	printk("device_create\n");	
	/* Request pwm0 */
	pwm0 = pwm_request(0, "my-pwm");
	if (IS_ERR(pwm0)) {
		printk(KERN_ERR "Failed to request PWM device\n");
		return PTR_ERR(pwm0);
	}
	pwm_config(pwm0, INITPWM, 1000);
	pwm_enable(pwm0);	
	
	/* bh1750_i2c_adapting i2c */
    bh1750_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
    if (bh1750_i2c_adapter == NULL) {
        printk(KERN_ERR "Failed to get I2C adapter\n");
        goto ExitError;
    }
    bh1750_i2c_client = i2c_new_client_device(bh1750_i2c_adapter, &bh1750_i2c_board_info);
    if (bh1750_i2c_client == NULL) {
        printk(KERN_ERR "Failed to create BH1750 I2C client\n");
        goto ExitError;
    }
    // BH1750 I2C 드라이버 등록
    ret = i2c_add_driver(&bh1750_driver);
    if (ret < 0) {
        printk(KERN_ERR "Failed to add BH1750 I2C driver\n");
        goto ExitError;
    }
	printk("BH1750 Driver added!\n");
	
    // BH1750 센서 초기화 및 설정	
    ret = bh1750_init(bh1750_i2c_client);
    if (ret < 0) {
        printk(KERN_ERR "Failed to initialize BH1750 sensor\n");
        goto ExitError;
    }
	
	/* Initialize LCD_GPIOs */
	printk("lcd-driver - GPIO Init\n");
	for(i=0; i<10; i++) {
		if(gpio_request(gpio_lcd[i], names[i])) {
			printk("lcd-driver - Error Init GPIO %d\n", gpio_lcd[i]);
			goto GpioInitError;
		}
	}	
	printk("lcd-driver - Set GPIOs to output\n");
	for(i=0; i<10; i++) {
		if(gpio_direction_output(gpio_lcd[i], 0)) {
			printk("lcd-driver - Error setting GPIO %d to output\n", i);
			goto GpioDirectionError;
		}
	}	
	/* Init the display */
	lcd_command(0x30);	/* Set the display for 8 bit data interface */

	lcd_command(0xf);	/* Turn display on, turn cursor on, set cursor blinking */

	lcd_command(0x1);	
	
	char text[] = "Hello World!";
	for(i=0; i<sizeof(text)-1;i++)
		lcd_data(text[i]);
	
	lcd_workqueue = create_workqueue("lcd_workqueue");
	INIT_WORK(&lcd_work, lcd_work_func);
    return 0;
	
GpioDirectionError:
	i=9;	
GpioInitError:
	for(;i>=0; i--)
	pwm_disable(pwm0);
	pwm_free(pwm0); 
	i2c_unregister_device(bh1750_i2c_client);
    i2c_del_driver(&bh1750_driver);
	device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);
FileError:
	class_destroy(dev_class);	
//AddError:
	//device_destroy(dev_class, dev);
ExitError:   
	pwm_disable(pwm0);
	pwm_free(pwm0); 
	i2c_unregister_device(bh1750_i2c_client);
    i2c_del_driver(&bh1750_driver);
	device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);
	
    return -1;
}

static void __exit illumiLCDExit(void) {
	int i;
	printk("MyDeviceDriver - Goodbye, Kernel!\n");	
    flush_workqueue(lcd_workqueue);
    destroy_workqueue(lcd_workqueue);	
	lcd_command(0x1);	/* Clear the display */
	for(i=0; i<10; i++){
		gpio_set_value(gpio_lcd[i], 0);
		gpio_free(gpio_lcd[i]);
	}
	pwm_disable(pwm0);
	pwm_free(pwm0);
	if(timer_pending(&timerLed)) //타이머가 실행하고 있는지 확인 
		del_timer(&timerLed);
	i2c_unregister_device(bh1750_i2c_client);
    i2c_del_driver(&bh1750_driver);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);
}

module_init(illumiLCDInit);
module_exit(illumiLCDExit);
MODULE_AUTHOR("JHY");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A driver for reading out Light sensor and control lcd brightness");