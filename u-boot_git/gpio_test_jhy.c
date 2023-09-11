#include <linux/kernel.h>
#include <linux/gpio.h>

#define OFF 0
#define ON 1
#define GPIO_ST 6
#define MAXLED 8


asmlinkage long sys_mysyscall(long val)
{
	int ret=0;
	int led[] = {0,1,2,3,4,5,6,7};
	//int key[] = {16,17,18,19,20,21,22,23};
	printk("val : %lx",val);

	for(int i=0;i<MAXLED;i++){
		char gpio_name[32]; // 충분한 크기의 문자열 버퍼를 할당
		// 문자열 포맷을 사용하여 이름을 동적으로 생성합니다.
		snprintf(gpio_name, sizeof(gpio_name), "gpio_led%d", i+GPIO_ST);

		ret = gpio_request(i+GPIO_ST,gpio_name);  //pin map, 커널에 등록할 이름
		//gpio가 다른데서 쓰이는지 확인
		if(ret<0)
		{
			printk("Failed Request gpio%d error\n",i+GPIO_ST);
			return 1;
		}
	}

	for(int i=0;i<MAXLED;i++){
		ret = gpio_direction_output(i+GPIO_ST,OFF);  //gpio pin, ON or OFF GPIO6을 출력으로 설정하고 OFF 하겠다.
		if(ret<0)
		{
			printk("Failed Direction Output gpio%d error\n",i+GPIO_ST);
			return 2;
		}
	}

	for(int i=0;i<MAXLED;i++){
		if((val >> i)& 0x01)
				gpio_set_value(i+GPIO_ST,ON);  //pin map, 내가 입력할 숫자에 따라 led 출력
	}

	for(int i=0;i<MAXLED;i++){
		gpio_free(i+GPIO_ST);
	}
	return 0;
}
