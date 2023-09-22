#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#define KTIMER_DEV_NAME "kerneltimer_dev"
#define KTIMER_DEV_MAJOR 230

#define gpioName(a,b) #a#b     //"led""0" == "led0"
#define GPIOLEDCNT 8
#define GPIOKEYCNT 8
#define OFF 0
#define ON 1
#define DEBUG 1

static int gpioLed[GPIOLEDCNT] = {6,7,8,9,10,11,12,13};
static int gpioKey[GPIOKEYCNT] = {16,17,18,19,20,21,22,23};

static int timerVal = 100;      //f=100HZ, T=1/100 = 10ms, 100*10ms = 1Sec
module_param(timerVal,int ,0);

static int ledVal = 0;
//module_param(ledVal,int ,0);

struct timer_list timerLed;

static int sw_irq[8] = {0};
static char sw_no = 0;

void kerneltimer_timeover(unsigned long arg);
void kerneltimer_func(struct timer_list *t );

int gpioLedInit(void);
void gpioLedSet(long);
void gpioLedFree(void);
int gpioKeyInit(void);
int gpioKeyGet(void);
void gpioKeyFree(void);

int gpioLedInit(void)
{
    int i;
    int ret = 0;
    for(i=0;i<GPIOLEDCNT;i++)
    {
        ret = gpio_request(gpioLed[i], gpioName(led,i));
        if(ret < 0) {
            printk("Failed Request gpio%d error\n", 6);
            return ret;
        }
    }
    for(i=0;i<GPIOLEDCNT;i++)
    {
        ret = gpio_direction_output(gpioLed[i], OFF);
        if(ret < 0) {
            printk("Failed direction_output gpio%d error\n", 6);
     return ret;
        }
    }
    return ret;
}

void gpioLedSet(long val)
{
    int i;
    for(i=0;i<GPIOLEDCNT;i++)
    {
        gpio_set_value(gpioLed[i], (val>>i) & 0x01);
    }
}
void gpioLedFree(void)
{
    int i;
    for(i=0;i<GPIOLEDCNT;i++)
    {
        gpio_free(gpioLed[i]);
    }
}

int gpioKeyInit(void)
{
    int i;
    int ret=0;;
    for(i=0;i<GPIOKEYCNT;i++)
    {
        ret = gpio_request(gpioKey[i], gpioName(key,i));
        if(ret < 0) {
            printk("Failed Request gpio%d error\n", 6);
            return ret;
        }
    }
    for(i=0;i<GPIOKEYCNT;i++)
    {
        ret = gpio_direction_input(gpioKey[i]);
        if(ret < 0) {
            printk("Failed direction_output gpio%d error\n", 6);
     return ret;
        }
    }
    return ret;
}
int gpioKeyGet(void)
{
    int i;
    int ret;
    int keyData=0;
    for(i=0;i<GPIOKEYCNT;i++)
    {
        ret=gpio_get_value(gpioKey[i]) << i;
        keyData |= ret;
    }
    return keyData;
}
void gpioKeyFree(void)
{
    int i;
    for(i=0;i<GPIOKEYCNT;i++)
    {
        gpio_free(gpioKey[i]);
    }
}

static void gpioKeyToIrq(void)
{
    int i;
    for (i = 0; i < GPIOKEYCNT; i++) {
    sw_irq[i] = gpio_to_irq(gpioKey[i]);
    }
}

static void gpioKeyFreeIrq(void)
{
    int i;
    for (i = 0; i < GPIOKEYCNT; i++){
        free_irq(sw_irq[i],NULL);
    }
}

static int ktimer_open(struct inode *inode, struct file *filp)
{
	int num = MINOR(inode->i_rdev);
	printk("ktimer open -> minor : %d\n", num);
	num = MAJOR(inode->i_rdev);
	printk("ktimer open -> major : %d\n", num);

	return 0;

}

static ssize_t ktimer_read(struct file *filp, char *buff, size_t count, loff_t *f_pos)
{
//      char kbuf;
    int ret;
#if DEBUG
    //printk( "ledkeydev read -> buff : %08X, count : %08X \n", (unsigned int)buff, count );
#endif
//      kbuf = gpioKeyGet();
    ret=copy_to_user(buff,&sw_no,count);
    sw_no = 0;
    if(ret < 0)
        return -ENOMEM;
    return count;
}
static ssize_t ktimer_write(struct file *filp, const char *buff, size_t count, loff_t *f_pos)
{
    //char kbuf;
    int ret;
#if DEBUG
    //printk( "ledkeydev write -> buf : %08X, count : %08X \n", (unsigned int)buf, count );
#endif
    ret=copy_from_user(&ledVal,buff,count);
    if(ret < 0)
		return -ENOMEM;	
	//gpioLedSet(kbuf);	
	
    return count;
}

static long ktimer_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{

#if DEBUG
    //printk( "ledkeydev ioctl -> cmd : %08X, arg : %08X \n", cmd, (unsigned int)arg );
#endif
    return 0;
}

static int ktimer_release(struct inode *inode, struct file *filp)
{

	printk("LED & Key closed.\n");
	return 0;
}

struct file_operations ktimer_fops =
{
	.owner = THIS_MODULE,
	.read   = ktimer_read,
	.write  = ktimer_write,
	.open   = ktimer_open,
	.unlocked_ioctl = ktimer_ioctl,
	.release  = ktimer_release,
};

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

void kerneltimer_registertimer(unsigned long timeover)
{
    timer_setup( &timerLed,kerneltimer_func,0);
    timerLed.expires = get_jiffies_64() + timeover;  //10ms *100 = 1sec
    add_timer( &timerLed );
}

void kerneltimer_func(struct timer_list *t)
{
    gpioLedSet(ledVal);
#if DEBUG
    printk("ledVal : %#04x\n",(unsigned int)(ledVal));
#endif
    ledVal = ~ledVal & 0xff;
    mod_timer(t,get_jiffies_64() + timerVal);
}

int kerneltimer_init(void)
{
	int result;
 	result = register_chrdev(KTIMER_DEV_MAJOR,KTIMER_DEV_NAME,&ktimer_fops);
	if(result <0) return result;

	gpioLedInit();
	gpioKeyInit();
    gpioKeyToIrq();
	
    for(int i=0;i<GPIOKEYCNT;i++)
    {
        result = request_irq(sw_irq[i],sw_isr,IRQF_TRIGGER_RISING,gpioName(key,i),NULL);
		printk("sw_irq[%d] : %d \n", i,sw_irq[i]);
        if(result)
        {
            printk("#### FAILED Request irq %d. error : %d \n", sw_irq[i], result);
            break;
        }
    }	
	kerneltimer_registertimer(timerVal);
	printk("ktimer ktimer_init \n");
	
#if DEBUG
    printk("timerVal : %d , sec : %d \n",timerVal,timerVal/HZ );
#endif

		
    return 0;
}
void kerneltimer_exit(void)
{
    if(timer_pending(&timerLed))
        del_timer(&timerLed);
    gpioLedSet(0);
    gpioLedFree();
    gpioKeyFree();
    gpioKeyFreeIrq();	
	printk("ktimer ktimer_exit \n");
	unregister_chrdev(KTIMER_DEV_MAJOR,KTIMER_DEV_NAME);
    if(timer_pending(&timerLed)) //타이머가 실행하고 있는지 확인 
        del_timer(&timerLed);	
}
module_init(kerneltimer_init);
module_exit(kerneltimer_exit);
MODULE_AUTHOR("KCCI-AIOT KSH");
MODULE_DESCRIPTION("led key test module");
MODULE_LICENSE("Dual BSD/GPL");
