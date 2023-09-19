#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/gpio.h>

#define CALL_DEV_NAME "ledkey_dev"
#define CALL_DEV_MAJOR 231
#define OFF 0
#define ON 1
#define GpioName(a,b) #a#b     //"led""0" == "led0"
#define GPIOLEDCNT 8
#define GPIOKEYCNT 8


#define DRIVER_CLASS "MyModuleClass"

/* Variables for device and device class */

static struct class *my_class;



int gpioLed[GPIOLEDCNT] = {6,7,8,9,10,11,12,13};
int gpioKey[GPIOKEYCNT] = {16,17,18,19,20,21,22,23};

int GpioLedInit(void);
void GpioLedSet(const char*);
void GpioLedFree(void);

int gpioKeyInit(void);
int gpioKeyGet(void);
void gpioKeyFree(void);

static int call_open(struct inode *inode, struct file *filp)
{
	int num = MINOR(inode->i_rdev);
	printk("call open -> minor : %d\n", num);
	num = MAJOR(inode->i_rdev);
	printk("call open -> major : %d\n", num);	
	GpioLedInit();
	gpioKeyInit();
	return 0;
	
}

static ssize_t call_read(struct file *filp, char *buff, size_t count, loff_t *f_pos)
{
	*buff = gpioKeyGet();
	return 0;
}
static ssize_t call_write(struct file *filp, const char *buff, size_t count, loff_t *f_pos)
{
    GpioLedSet(buff);
	return 0;
}

static int call_release(struct inode *inode, struct file *filp)
{
	gpioKeyFree();
	GpioLedFree();	
	printk("LED & Key closed.\n");
	return 0;
}	

struct file_operations call_fops = 
{
	.owner = THIS_MODULE,
	.read   = call_read,
	.write  = call_write,
	.open   = call_open,
	.release  = call_release,
};

static int call_init(void)
{
	int result;
	printk("call call_init \n");
	result = register_chrdev(CALL_DEV_MAJOR,CALL_DEV_NAME,&call_fops);
	//register_chrdev 함수를 사용하여 문자 디바이스를 등록했지만, 이로 인해 /dev/ 디렉토리에 직접 파일이 생성되는 것은 아니다.
	//커널 모듈 내에서 calldev라는 문자 디바이스를 등록하는 것, 커널 공간에서만 존재
	if(result <0) return result;
	
	/* Create device class */
	if((my_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
		printk("Device class can not be created!\n");
		goto ClassError;
	}

	/* create device file */
	if(device_create(my_class, NULL, MKDEV(CALL_DEV_MAJOR, 0), NULL, CALL_DEV_NAME) == NULL) {
		printk("Can not create device file!\n");
		goto FileError;
	}
	
	return 0;


FileError:
	class_destroy(my_class);
ClassError:
	unregister_chrdev_region(CALL_DEV_MAJOR, 1);
	return -1;	
	
	
}
static void call_exit(void)
{
	printk("call call_exit \n");
	unregister_chrdev(CALL_DEV_MAJOR,CALL_DEV_NAME);
	device_destroy(my_class, CALL_DEV_MAJOR);
	class_destroy(my_class);	

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

void GpioLedSet(const char *buff)
{
	int i;
	long val = *buff;
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
		ret = gpio_request(gpioKey[i], GpioName("key",i));		          //GPIO KEY 설정
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

module_init(call_init);
module_exit(call_exit);

MODULE_AUTHOR("HaeYoung Jung");
MODULE_DESCRIPTION("Module Parameter Test Module");
MODULE_LICENSE("Dual BSD/GPL");