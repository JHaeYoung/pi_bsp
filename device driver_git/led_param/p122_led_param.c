#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/moduleparam.h>
#define OFF 0
#define ON 1
#define gpioName(a,b) #a#b     //"led""0" == "led0"
#define GPIOLEDCNT 8
#define GPIOKEYCNT 8

static long onevalue =1;
static char *twostring = NULL;

module_param(onevalue, long,0);
module_param(twostring, charp,0);

int gpioLed[GPIOLEDCNT] = {6,7,8,9,10,11,12,13};

int  gpioLedInit(void);
void gpioLedSet(long onevalue);
void gpioLedFree(void);


static int hello_init(void)
{
	printk("Hello, world \n");
	gpioLedInit();
	gpioLedSet(onevalue);
	return 0;
}
static void hello_exit(void){
	printk("Goodbye, world\n");
	gpioLedSet(0x00);
	gpioLedFree();
}

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

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("Dual BSD/GPL");
