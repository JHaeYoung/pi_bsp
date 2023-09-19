#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#define OFF 0
#define ON 1
#define GpioName(a,b) #a#b     //"led""0" == "led0"
#define GPIOLEDCNT 8
#define GPIOKEYCNT 8

int GpioLed[GPIOLEDCNT] = {6,7,8,9,10,11,12,13};
int gpioKey[GPIOKEYCNT] = {16,17,18,19,20,21,22,23};

int GpioLedInit(void);
void GpioLedSet(long);
void GpioLedFree(void);

int gpioKeyInit(void);
int gpioKeyGet(void);
void gpioKeyFree(void);

static int hello_init(void)
{
	printk("Hello, world \n");
	GpioLedInit();
	gpioKeyInit();
	
	GpioLedSet(gpioKeyGet());
	return 0;
}
static void hello_exit(void)
{
	printk("Goodbye, world \n");
	GpioLedSet(0x00);
	GpioLedFree();
	gpioKeyFree();
}

int     GpioLedInit(void)
{
	int i;
	int ret = 0;
	for(i=0;i<GPIOLEDCNT;i++)
	{
		ret = gpio_request(GpioLed[i], GpioName(led,i));
		if(ret < 0) {
				printk("Failed Request Gpio%d error\n", 6);
				return ret;
		}
	}
	for(i=0;i<GPIOLEDCNT;i++)
	{
		ret = gpio_direction_output(GpioLed[i], OFF);
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
			gpio_set_value(GpioLed[i], (val>>i) & 0x01);
	}
}
void GpioLedFree(void)
{
	int i;
	for(i=0;i<GPIOLEDCNT;i++)
	{
			gpio_free(GpioLed[i]);
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


module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
