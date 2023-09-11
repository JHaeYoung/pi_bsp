#include <linux/kernel.h>
#include <linux/gpio.h>

#define OFF 0
#define ON 1
#define gpioName(a,b) #a#b //"led""0"  ==> "led0" 문자열 인자
#define GPIOLEDCNT 8
#define GPIOKEYCNT 8

int gpioLed[GPIOLEDCNT] = { 6,7,8,9,10,11,12,13};
int gpioKey[GPIOKEYCNT] = {16,17,18,19,20,21,22,23};

int gpioLedInit(void);
void gpioLedSet(long);
void gpioledFree(void);

int gpioKeyInit(void);
int gpioKeyGet(void);
void gpioKeyFree(void);

asmlinkage long sys_mysyscall(long val)
{
	long keyData=0;
	
	gpioLedInit();
	gpioLedSet(val);
	gpioLedFree();
	
	gpioKeyInit();
	keyData =gpioKeyGet();
	gpioKeyFree();
	
	return keyData;

}

int gpioLedInit(){
	
	int ret = 0;
	int i=0;
	

	for(i=0;i<GPIOLEDCNT;i++)
	{			
		ret = gpio_request(gpioLed[i], gpioName("led",i));		
		if(ret < 0) {
			printk("Failed Request gpio%d error\n", i);
			return ret;
		}
	}
	for(i=0;i<GPIOLEDCNT;i++)
	{			
		ret = gpio_direction_output(gpioLed[i], OFF);
		if(ret < 0) {
			printk("Failed Request gpio%d error\n", i);
			return ret;
		}
	}	
	return ret;
}

void gpioLedSet(long val)
{
	int ret = 0;
	int i=0;
	for(i=0;i<GPIOLEDCNT;i++)
	{	
		gpio_set_value(gpioLed[i],(val>>i) & 0x01);
	}	
}

void gpioledFree(void)
{
	int i=0;
	for(i=0;i<GPIOLEDCNT;i++){
		gpio_free(gpioLed[i]);
	}   
}
	
int gpioKeyInit(void){

	int ret = 0;
	int i=0;	

	for(i=0;i<GPIOKEYCNT;i++)
	{			
		ret = gpio_request(gpioKey[i], gpioName("key",i));		          //GPIO KEY 설정
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
