#include <common.h>  
#include <command.h>  //u-boot cmd 구조체 들어있는 헤더
#include <asm/io.h>

#define BCM2711_GPIO_GPFSEL0 0XFE200000
#define BCM2711_GPIO_GPFSEL1 0xFE200004
#define BCM2711_GPIO_GPFSEL2 0xFE200008
#define BCM2711_GPIO_GPSET0  0XFE20001C
#define BCM2711_GPIO_GPCLR0  0xFE200028
#define BCM2711_GPIO_GPLEV0  0xFE200034

#define GPIO6_9_SIG_OUTPUT 0x09240000 
#define GPIO10_13_SIG_OUTPUT 0x00012249 //txd,rxd
#define GPIO6_13_SET_CLR_BIT 0x00003fc0

void led_init_key(void)
{
	unsigned long temp; //부호 없는 4바이트, 초기 값을 읽기 위한 변수
	temp = readl(BCM2711_GPIO_GPFSEL0);	
	//temp = temp & 0xc003ffff;  // 수정할 비트 자리를 0으로 초기화
	temp = temp & ~0x3FFC0000; //이게 더 보기 좋음
	temp= temp | GPIO6_9_SIG_OUTPUT; // 수정할 값과 or 연산
	writel(temp,BCM2711_GPIO_GPFSEL0);

	temp = readl(BCM2711_GPIO_GPFSEL1);	
	//temp= temp & 0xfffff000; 
	temp= temp & ~0x00000fff; 
	temp= temp | GPIO10_13_SIG_OUTPUT;
	writel(temp ,BCM2711_GPIO_GPFSEL1);
	
	
}
//unsigned long key_init(){


void key_read(unsigned long *key_data){	
	*key_data = readl(BCM2711_GPIO_GPLEV0);		
}
void led_write_key(unsigned long led_data)
{
	writel(GPIO6_13_SET_CLR_BIT , BCM2711_GPIO_GPCLR0); //led all off
	led_data = led_data << 6;
	writel(led_data, BCM2711_GPIO_GPSET0); //ledX on
}
static int do_KCCI_LED(struct cmd_tbl *cmdtp, int flag, int argc, char* const argv[])
{	
	unsigned long key_data;
	unsigned long prev_key_data = 0;
	unsigned long extracted_bits;
	//unsigned long led_data;
	if(argc !=1)
	{
		cmd_usage(cmdtp);
		return 1;
	}
	printf("*LED TEST START\n");
	led_init_key();	
	//led_data = simple_strtoul(argv[1],NULL, 16);
	//led_write(key_data != prev_key_data) {
	do{				
		key_read(&key_data);
		key_data = key_data >> 16;			
		extracted_bits = (key_data & 0xFF); 		
		//printf("key_data : %lx",key_data);
		
		if(extracted_bits != prev_key_data) {
			led_write_key(extracted_bits);		
			printf("led_key : 0x%lx\n",extracted_bits);					
			if(extracted_bits == 0x01) printf("O:X:X:X:X:X:X:X\n\n");		
			else if(extracted_bits == 0x02) printf("X:O:X:X:X:X:X:X\n\n");
			else if(extracted_bits == 0x04) printf("X:X:O:X:X:X:X:X\n\n");
			else if(extracted_bits == 0x08) printf("X:X:X:O:X:X:X:X\n\n");
			else if(extracted_bits == 0x10) printf("X:X:X:X:O:X:X:X\n\n");
			else if(extracted_bits == 0x20) printf("X:X:X:X:X:O:X:X\n\n");
			else if(extracted_bits == 0x40) printf("X:X:X:X:X:X:O:X\n\n");
			else if(extracted_bits == 0x80) printf("X:X:X:X:X:X:X:O\n\n");
			prev_key_data = extracted_bits;
	
		}
	}while(extracted_bits != 0x80);
	
	printf("*LED_KEY TEST END\n\n ");
	// %#04x 앞에 4자리가 아니면 0으로 채워라
	return 0;
}
U_BOOT_CMD(
	ledkey,1,0,do_KCCI_LED, //led(명령어), 2(argv 갯수), flag에 보내줄 변수,함수
	"kcci LED_KEY Test",
	"number -Input argument is only one.(led [0x00~0xff])\n"); //사용법
 