#include <stdio.h>
#include <unistd.h>
#include <asm-generic/unistd.h>
#include <stdlib.h>
#pragma GCC diagnostic ignored "-Wunused-result"

int main(int argc, char *argv[])
{
	if(argc<0)
	{
		perror("syscall");
		return 1;
	}	

	long key_data,key_data_old=0;
	long shift_data = 0x01;
	int i;
	

	
	if(argc ==2){
		char *input = argv[1];
		char *endptr;
		long val = strtol(input, &endptr, 16);		

		// strtol 함수는 변환 실패 시 0을 반환하므로 에러 처리가 필요할 수 있습니다.
		if (*endptr != '\0') {
			printf("Invalid input: %s is not a valid number.\n", input);
			return 1;
		}
		printf("Converted value: 0x%lx\n", val);		
		
		key_data = syscall(__NR_mysyscall,val); 
        if(val<0)
        {
			perror("syscall");
			return 1;
        }	
        printf("mysyscall return value = 0x%lx\n",val);
		puts("0:1:2:3:4:5:6:7");
		for(i=0;i<8;i++)
		{
			if(val & (0x01 << i))
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
	else if(argc==1){
		do
		{			
			for(i=0;i<7;i++){
				key_data = syscall(__NR_mysyscall,shift_data);
				usleep(200000);
				shift_data = shift_data <<1;			
				puts("0:1:2:3:4:5:6:7");
				for(int j=0;j<8;j++){
					if(shift_data & (0x01<<j))
						putchar('O');
					else
						putchar('X');
					if(j != 7 )
						putchar(':');
					else
						putchar('\n');			
				}	
				putchar('\n');					
			}
			for(i=7;i>0;i--){				
				usleep(200000);
				shift_data = shift_data >>1;	
				key_data = syscall(__NR_mysyscall,shift_data);
				puts("0:1:2:3:4:5:6:7");
				for(int j=0;j<8;j++){
					if(shift_data & (0x01<<j))
						putchar('O');
					else
						putchar('X');
					if(j != 7 )
						putchar(':');
					else
						putchar('\n');			
				}	
				putchar('\n');					
			}
			if(key_data == 0x80)
				break;
		}while(1);
	}
		
    return 0;
}


	/*do {
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
        printf("mysyscall return value = %#04x\n",key_data); */
		
