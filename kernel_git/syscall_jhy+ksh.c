#include <linux/kernel.h>
#include <linux/gpio.h>

#define OFF 0
#define ON 1
#define gpioName(a,b) #a#b //"led""0"  ==> "led0" 문자열 인자

int gpioLed[] = { 6,7,8,9,10,11,12,13};
int gpioKey[] = {16,17,18,19,20,21,22,23};

asmlinkage long sys_mysyscall(long val)
{
	int ret = 0;
	int i=0;
	int gpioLedCnt = sizeof(gpioLed)/sizeof(gpioLed[0]);
	int gpioKeyCnt = sizeof(gpioKey)/sizeof(gpioKey[0]);
	char gpio_name[32];
	long keyData =0;
	
			
	for(i=0;i<gpioLedCnt;i++)
	{	
		snprintf(gpio_name, sizeof(gpio_name), "led%d",i);
		
		ret = gpio_request(gpioLed[i], gpio_name);		
		if(ret < 0) {
			printk("Failed Request gpio%d error\n", i);
			return ret;
		}
	}
	
	for(i=0;i<gpioLedCnt;i++)
	{			
		ret = gpio_direction_output(gpioLed[i], OFF);
		if(ret < 0) {
			printk("Failed Request gpio%d error\n", i);
			return ret;
		}
	}	
	for(i=0;i<gpioLedCnt;i++)
	{	
		gpio_set_value(gpioLed[i],(val>>i) & 0x01);
	}	
	
	for(i=0;i<gpioLedCnt;i++){
		gpio_free(gpioLed[i]);
	}   

// GPIO KEY SETTING 
 	

	
	for(i=0;i<gpioKeyCnt;i++)
	{	
		snprintf(gpio_name, sizeof(gpio_name), "key%d",i);
		
		ret = gpio_request(gpioKey[i], gpio_name);		          //GPIO KEY 설정
		if(ret < 0) {
			printk("Failed Request gpio%d error\n", i);
			return ret;
		}
	}
	
	for(i=0;i<gpioKeyCnt;i++)
	{			
		ret = gpio_direction_input(gpioKey[i]);                  //GPIO INPUT, OUTPUT 설정
		if(ret < 0) {
			printk("Failed Request gpio%d error\n", i);
			return ret;
		}
	}	
	for(i=0;i<gpioKeyCnt;i++)
	{	
		ret = gpio_get_value(gpioKey[i]) << i ;                  //gpio 8개 값을 8비트로 변경
		keyData |=  ret ;
	}	
	
	for(i=0;i<gpioKeyCnt;i++){
		gpio_free(gpioKey[i]);
	}   

	return keyData;
}
