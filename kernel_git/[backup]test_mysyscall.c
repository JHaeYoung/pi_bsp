#include <linux/kernel.h>
#include <linux/gpio.h>
#define OFF 0
#define ON 1

asmlinkage long sys_mysyscall(long val)
{
	//printk(KERN_INFO "Welcome to KCCI's Embedded system!! app value=%ld\n",val);
	//return val*val;
	int ret = 0;
	ret = gpio_request(6, "led0");
	if(ret < 0) {
		printk("Failed Request gpio%d error\n", 6);
		return ret;
	}
	ret = gpio_direction_output(6, OFF);
	if(ret < 0) {
		printk("Failed direction_output gpio%d error\n", 6);
        return ret;
	}
	gpio_set_value(6, (int)val);
    
	gpio_free(6);
	return val;
}
