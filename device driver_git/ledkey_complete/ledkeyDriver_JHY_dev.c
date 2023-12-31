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
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include "ioctl.h"

#define LEDKEY_DEV_NAME "kerneltimer_dev"
#define LEDKEY_DEV_MAJOR 230

#define gpioName(a,b) #a#b     //"led""0" == "led0"
#define GPIOLEDCNT 8
#define GPIOKEYCNT 8
#define OFF 0
#define ON 1
#define DEBUG 1
#define GPIOKEYGET 0

DECLARE_WAIT_QUEUE_HEAD(WaitQueue_Read);

static int gpioLed[GPIOLEDCNT] = {6,7,8,9,10,11,12,13};
static int gpioKey[GPIOKEYCNT] = {16,17,18,19,20,21,22,23};

static int ledVal = 0;
module_param(ledVal,int ,0);

static int timerVal = 100;     //f=100HZ, T=1/100 = 10ms, 100*10ms = 1Sec
module_param(timerVal,int ,0);

//keyled_data ctrl_info = { .timer_val = 0 };

struct timer_list timerLed;

static int sw_irq[8] = {0};
//static char sw_no = 0;

static int gpioLedInit(void);
static void gpioLedSet(long);
static void gpioLedFree(void);
static int gpioKeyInit(void);
#if GPIOKEYGET
static int gpioKeyGet(void);
#endif
static void gpioKeyFree(void);
static int requestIrqInit(struct file *filp);
static void kerneltimer_registertimer(unsigned long timeover);
static void kerneltimer_func(struct timer_list *t);
static void kerneltimer_delitetimer(struct timer_list *t);
irqreturn_t sw_isr(int irq, void *privateData);

static int gpioLedInit(void)
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

static void gpioLedSet(long val)
{
    int i;
    for(i=0;i<GPIOLEDCNT;i++)
    {
        gpio_set_value(gpioLed[i], (val>>i) & 0x01);
    }
}
static void gpioLedFree(void)
{
    int i;
    for(i=0;i<GPIOLEDCNT;i++)
    {
        gpio_free(gpioLed[i]);
    }
}

static int gpioKeyInit(void)
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
#if GPIOKEYGET
static int gpioKeyGet(void)
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
#endif
static void gpioKeyFree(void)
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

static void gpioKeyFreeIrq(struct file *filp)
{
    int i;
    for (i = 0; i < GPIOKEYCNT; i++){
        free_irq(sw_irq[i],filp->private_data);
    }
}

irqreturn_t sw_isr(int irq, void *privateData)
{
    int i;
    char *pSw_no = (char*)privateData;
	for(i=0;i<GPIOKEYCNT;i++)
    {
        if(irq == sw_irq[i])
        {
            *pSw_no = i+1;			
            break;
        }
    }
    printk("IRQ : %d, sw_no : %d\n",irq,*pSw_no);
	wake_up_interruptible(&WaitQueue_Read);
    return IRQ_HANDLED;
}

static int ledkeydev_open(struct inode *inode, struct file *filp)
{
	char * pSw_no=NULL;
	int num = MINOR(inode->i_rdev);	
	int result=0;
	printk("ledkeydev open -> minor : %d\n", num);
	num = MAJOR(inode->i_rdev);
	printk("ledkeydev open -> major : %d\n", num);

	
	
	result = gpioLedInit();
	if(result < 0)
  		return result;     /* Device or resource busy */

	result = gpioKeyInit();
	if(result < 0)
  		return result;     /* Device or resource busy */
	gpioKeyToIrq();

	pSw_no = kmalloc(sizeof(char),GFP_KERNEL);
	if(!pSw_no) 
		return -ENOMEM;
	
	//kerneltimer_registertimer(timerVal);
	filp->private_data = pSw_no;
	requestIrqInit(filp);
	return 0;
}

static ssize_t ledkeydev_read(struct file *filp, char *buff, size_t count, loff_t *f_pos)
{
    int ret;
	char *pSw_no = filp->private_data;
#if DEBUG
    //printk( "ledkeydev read -> buff : %08X, count : %08X \n", (unsigned int)buff, count );
#endif
	if(!(filp->f_flags & O_NONBLOCK))  //BLOCK Mode
	{
		if(*pSw_no == 0)
			wait_event_interruptible(WaitQueue_Read,*pSw_no);
	}
    ret=copy_to_user(buff,pSw_no,count);
    *pSw_no = 0;
    if(ret < 0)
        return -ENOMEM;
    return count;
}
static ssize_t ledkeydev_write(struct file *filp, const char *buff, size_t count, loff_t *f_pos)
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

static long ledkeydev_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
	keyled_data ctrl_info = {0};
	char *pSw_no = filp->private_data;
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
			kerneltimer_registertimer(timerVal);		
        break;
    case TIMER_STOP :
		if(timer_pending(&timerLed)) //타이머가 실행하고 있는지 확인 
			del_timer(&timerLed);			
        break;
	case TIMER_VALUE :				
		err = copy_from_user((void *)&ctrl_info,(void *)arg,(unsigned long)sizeof(ctrl_info));		
		
		timerVal = ctrl_info.timer_val;
		break;				
	default:
		err =-E2BIG;
		break;
	}	
	
    return err;
}

static int ledkeydev_release(struct inode *inode, struct file *filp)
{
	gpioLedFree();
    gpioKeyFreeIrq(filp);
    gpioKeyFree();
	if(filp->private_data) 
		kfree(filp->private_data);
	if(timer_pending(&timerLed)) //타이머가 실행하고 있는지 확인 
        del_timer(&timerLed);
	printk("LED & Key closed.\n");
	return 0;
}

static unsigned int ledkeydev_poll(struct file * filp, struct poll_table_struct * wait)
{
	char *pSw_no = filp->private_data;
	unsigned int mask = 0;   //내가 리턴해야할 값
	printk("_key : %u \n",(wait->_key & POLLIN));
	if(wait->_key & POLLIN)
		poll_wait(filp, &WaitQueue_Read, wait); //이 친구도 블로킹 함수
	if(*pSw_no > 0)
		mask = POLLIN;
	return mask;
}

struct file_operations ledkeydev_fops =
{
	.owner = THIS_MODULE,
	.read   = ledkeydev_read,
	.write  = ledkeydev_write,
	.open   = ledkeydev_open,
	.unlocked_ioctl = ledkeydev_ioctl,
	.poll     = ledkeydev_poll,
	.release  = ledkeydev_release,
};

static int requestIrqInit(struct file *filp)
{
	int result= 0;
	
    for(int i=0;i<GPIOKEYCNT;i++)
    {
        result = request_irq(sw_irq[i],sw_isr,IRQF_TRIGGER_RISING,gpioName(key,i),filp->private_data);		 
        if(result)
        {
            printk("#### FAILED Request irq %d. error : %d \n", sw_irq[i], result);
            break;
        }
    }	
	return 0;
}

static void kerneltimer_registertimer(unsigned long timeover)
{
    timer_setup( &timerLed,kerneltimer_func,0);
    timerLed.expires = get_jiffies_64() + timeover;  //10ms *100 = 1sec
    add_timer( &timerLed );
}
static void kerneltimer_func(struct timer_list *t )
{	
    gpioLedSet(ledVal);
#if DEBUG
    printk("ledVal : %#04x\n",(unsigned int)(ledVal));
	printk("timerVal : %d\n",(unsigned int)(timerVal));
#endif
    ledVal = ~ledVal & 0xff;
    mod_timer(t,get_jiffies_64() + timerVal);
}

int ledkeydev_init(void)
{
	int result;
	int i;
 	result = register_chrdev(LEDKEY_DEV_MAJOR,LEDKEY_DEV_NAME,&ledkeydev_fops);
	if(result <0) return result;
	
	//kerneltimer_registertimer(timerVal);
	printk("
	ledkeydev_init \n");	
#if DEBUG
    printk("timerVal : %d , sec : %d \n",timerVal,timerVal/HZ );
#endif
		
    return 0;
}
void ledkeydev_exit(void)
{
	unregister_chrdev(LEDKEY_DEV_MAJOR,LEDKEY_DEV_NAME);
}
module_init(ledkeydev_init);
module_exit(ledkeydev_exit);
MODULE_AUTHOR("KCCI-AIOT JHY");
MODULE_DESCRIPTION("led key test module");
MODULE_LICENSE("Dual BSD/GPL");
