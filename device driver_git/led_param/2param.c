#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/moduleparam.h>

static int onevalue =1;
static char *twostring = NULL;

module_param(onevalue, int,0);
module_param(twostring, charp,0);

static int hello_init(void)
{
        printk("Hello I'm Haeyoung [onevalue=%d:twostring=%s]\n",onevalue,twostring);
        return 0;
}

static void hello_exit(void)
{
        printk("Goodbye \n");
}
module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("HaeYoung Jung");
MODULE_DESCRIPTION("Module Parameter Test Module");
MODULE_LICENSE("Dual BSD/GPL");

