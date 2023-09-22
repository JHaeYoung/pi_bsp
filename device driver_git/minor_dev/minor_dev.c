#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/gpio.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#define   MINOR_DEV_NAME        "minordev"
#define   MINOR_DEV_MAJOR            230
#define   MINOR_WRITE_ADDR        0x0378
#define   MINOR_READ_ADDR         0x0379

#define OFF 0
#define ON 1
#define GpioName(a,b) #a#b     //"led""0" == "led0"
#define GPIOLEDCNT 8
#define GPIOKEYCNT 8

static int gpioLed[GPIOLEDCNT] = {6,7,8,9,10,11,12,13};
static int gpioKey[GPIOKEYCNT] = {16,17,18,19,20,21,22,23};

int GpioLedInit(void);
void GpioLedSet(long);
void GpioLedFree(void);

int gpioKeyInit(void);
int gpioKeyGet(void);
void gpioKeyFree(void);

int minor0_open (struct inode *inode, struct file *filp)
{
    printk( "call minor0_open\n" );
    return 0;
}

ssize_t minor0_write (struct file *filp, const char *buff, size_t count, loff_t *f_pos)
{
    //버퍼에서 데이터를 읽고 처리
    char kbuf; // 커널 공간 버퍼
	int ret;
//  get_user(kbuf,buff); // buff의 주소의 1바이트만 가져옴
	ret = copy_from_user(&kbuf, buff,count);
	// LED 설정
    GpioLedSet(kbuf);
	// 읽어온 값인 count 를 리턴
    return count;
}

int minor0_release (struct inode *inode, struct file *filp)
{
    printk( "call minor0_release\n" );
    return 0;
}

int minor1_open (struct inode *inode, struct file *filp)
{
    printk( "call minor1_open\n" );
    return 0;
}

ssize_t minor1_read(struct file *filp, char *buff, size_t count, loff_t *f_pos)
{
    char kbuf; // 커널 공간 버퍼
	int ret;
	kbuf = gpioKeyGet();
	ret = copy_to_user(buff, &kbuf,count);
	
    return count;
}

int minor1_release (struct inode *inode, struct file *filp)
{
    printk( "call minor1_release\n" );
    return 0;
}

struct file_operations minor0_fops =
{
    .owner    = THIS_MODULE,
    .write    = minor0_write,
    .open     = minor0_open,
    .release  = minor0_release,
};

struct file_operations minor1_fops =
{
    .owner    = THIS_MODULE,
    .read     = minor1_read,
    .open     = minor1_open,
    .release  = minor1_release,
};

int minor_open (struct inode *inode, struct file *filp)
{
    printk( "call minor_open\n" );
    switch (MINOR(inode->i_rdev))
    {
    case 0: filp->f_op = &minor0_fops; break; // fop를 minor로  다시 갱신함
    case 1: filp->f_op = &minor1_fops; break;
    default : return -ENXIO;
    }

    if (filp->f_op && filp->f_op->open)
        return filp->f_op->open(inode,filp);

    return 0;
}

struct file_operations minor_fops =
{
    .owner    = THIS_MODULE,
    .open     = minor_open,
};

int minor_init(void)
{
    int result;

    result = register_chrdev( MINOR_DEV_MAJOR, MINOR_DEV_NAME, &minor_fops);
    if (result < 0) return result;

    return 0;
}

void minor_exit(void)
{
    unregister_chrdev( MINOR_DEV_MAJOR, MINOR_DEV_NAME );
}

int GpioLedInit(void)
{
        int i;
        int ret = 0;
        for(i=0;i<GPIOLEDCNT;i++)
        {
                ret = gpio_request(gpioLed[i], GpioName(led,i));
                if(ret < 0) {
                                printk("Failed Request Gpio%d error\n", 6);
                                return ret;
                }
        }
        for(i=0;i<GPIOLEDCNT;i++)
        {
                ret = gpio_direction_output(gpioLed[i], OFF);
                if(ret < 0) {
                                printk("Failed direction_output Gpio%d error\n", 6);
                return ret;
                }
        }
        return ret;
}

void GpioLedSet(long val)
{
        int i;
        for(i=0;i<GPIOLEDCNT;i++)
        {
                        gpio_set_value(gpioLed[i], (val>>i) & 0x01);
        }
}
void GpioLedFree(void)
{
        int i;
        for(i=0;i<GPIOLEDCNT;i++)
        {
                        gpio_free(gpioLed[i]);
        }
}

int gpioKeyInit(void){

        int ret = 0;
        int i=0;

        for(i=0;i<GPIOKEYCNT;i++)
        {
                ret = gpio_request(gpioKey[i], GpioName("key",i));                       //GPIO KEY 설정
                if(ret < 0) {
                        printk("Failed Request gpio%d error\n", i);
                        return ret;
                }
        }

        for(i=0;i<GPIOKEYCNT;i++)
        {
                ret = gpio_direction_input(gpioKey[i]);                  //GPIO INPUT, OUTPUT 설정
                if(ret < 0) {
                        printk("Failed Request gpio%d error\n", i);
                        return ret;
                }
        }
        return 0;
}

int gpioKeyGet(void){
        int ret = 0;
        int i=0;
        long keyData=0;
        for(i=0;i<GPIOKEYCNT;i++)
        {
                ret = gpio_get_value(gpioKey[i]) << i ;                  //gpio 8개 값을 8비트로 변경
                keyData |=  ret ;
        }
        return keyData;
}

void gpioKeyFree(void){

        int i=0;
        for(i=0;i<GPIOKEYCNT;i++){
                gpio_free(gpioKey[i]);
        }
}

module_init(minor_init);
module_exit(minor_exit);

MODULE_LICENSE("Dual BSD/GPL");
