#include <stdio.h>
#include <unistd.h>
#include <asm-generic/unistd.h>
#pragma GCC diagnostic ignored "-Wunused-result"
int main()
{
    long val;
	long key_data,key_data_old=0;
	int i;
	printf("input value = ");
	scanf("%x",&val);

	if(i<0)
	{
			perror("syscall");
			return 1;
	}
	key_data_old = val;

	do {
		usleep(100000);     // cpu 사용률 낮추기 위해, 다른 프로세스에 권한을 주기 위해
		key_data = syscall(__NR_mysyscall,key_data_old);  //폴링 방식이기 떄문에 cpu가 계속 일을 해야한다.
		printf("key_data_old : %ld\n ",key_data_old);
		if(key_data != key_data_old)
		{
			if(key_data)
			{
				key_data_old = key_data;
				syscall(__NR_mysyscall,key_data_old);
	
				puts("0:1:2:3:4:5:6:7");
				for(i=0;i<8;i++)
				{
					if(key_data & (0x01 << i))
						putchar('O');
					else
						putchar('X');
					if(i != 7 )
						putchar(':');
					else
						putchar('\n');
				}
				putchar('\n');
			}
			if(key_data == 0x80)
			break;
		}

	}while(1);
        printf("mysyscall return value = %#04x\n",key_data);
        return 0;
}
