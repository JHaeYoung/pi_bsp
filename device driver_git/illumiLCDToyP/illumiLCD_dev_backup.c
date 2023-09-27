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
#define Continuously_H_Resolution_Mode2 0x11
#define Continuously_L_Resolution_Mode 0x13
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
u32 pwm_on_time = 500000000;

/* LCD char buffer */
static char lcd_buffer[17];
unsigned int gpios[] = {
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
#define REGISTER_SELECT gpios[1]

void lcd_enable(void) {
	gpio_set_value(gpios[0], 1);
	msleep(5);
	gpio_set_value(gpios[0], 0);
}
/*
 * @brief set the 8 bit data bus
 * @param data: Data to set
 */
void lcd_send_byte(char data) {
	int i;
	for(i=0; i<8; i++)
		gpio_set_value(gpios[i+2], ((data) & (1<<i)) >> i);
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
	char cmd_set_mode = Continuously_L_Resolution_Mode;
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

static ssize_t bh1750_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int ret;
    char data[2];
	static int bh1750_data = 0;
    // BH1750 센서로부터 데이터 읽기
    ret = i2c_master_recv(bh1750_i2c_client, data, 2);
    if (ret < 0) {
        printk(KERN_ERR "Error reading data from BH1750 sensor\n");
        return ret;
    }
	// Wait for conversion to complete (depends on mode)
    msleep(180); // You may need to adjust this based on your mode selection

    // 데이터를 16비트로 변환하여 bh1750_data에 저장
    bh1750_data = (data[0] << 8) | data[1];

    // 사용자 공간으로 데이터 전송
    ret = copy_to_user(buf, &bh1750_data, sizeof(bh1750_data));
    if (ret != 0) {
        printk(KERN_ERR "Failed to copy data to user space\n");
        return -EFAULT;
    }
	printk("read\n");
    return sizeof(bh1750_data);
}

static ssize_t pwm_lcd_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta;
	unsigned long pwm_value;
	printk("pwm_write\n");
	/* Get amount of data to copy */
	to_copy = min(count, sizeof(pwm_value));
	printk("to_copy\n");
	/* Copy data to user */
	not_copied = copy_from_user(&pwm_value, user_buffer, to_copy);

	/* Set PWM on time */
	//if(pwm_value < 0 || pwm_value > 65536)
		//printk("Invalid Value\n");
	//else
	//pwm_config(pwm0, 1000000000 * ((65535-pwm_value)/65535), 1000000000);
	pwm_config(pwm0, 1000 * 0.5, 1000);
	printk("pwm_value : %ld\n",pwm_value);
	
	/* Set the new data to the display */
	lcd_command(0x1);
	
	for(int i=0; i<to_copy; i++)
		lcd_data(pwm_value);	
	
	/* Calculate data */
	delta = to_copy - not_copied;

	return delta;
}

static struct file_operations fops =
{
  .owner          = THIS_MODULE,
  .read           = bh1750_read,
  .write 		  = pwm_lcd_write,	
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
	pwm_config(pwm0, pwm_on_time, 1000);
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
		if(gpio_request(gpios[i], names[i])) {
			printk("lcd-driver - Error Init GPIO %d\n", gpios[i]);
			goto GpioInitError;
		}
	}	
	printk("lcd-driver - Set GPIOs to output\n");
	for(i=0; i<10; i++) {
		if(gpio_direction_output(gpios[i], 0)) {
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
	printk("MyDeviceDriver - Goodbye, Kernel!\n");
	int i;
	lcd_command(0x1);	/* Clear the display */
	for(i=0; i<10; i++){
		gpio_set_value(gpios[i], 0);
		gpio_free(gpios[i]);
	}
	pwm_disable(pwm0);
	pwm_free(pwm0);
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