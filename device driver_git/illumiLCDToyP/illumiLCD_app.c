#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ioctl.h"

#define DEVICE_FILENAME "/dev/illumiLCD"

int main() {
	
	unsigned long data;
	int oldData=0;
	int fd, ret;
	int loopFlag = 1;
	int pwm_val =0;
	char key_no;	
	char inputString[80];
	struct pollfd Events[2];
	illumiLCD_data info;
	
    fd = open(DEVICE_FILENAME, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device file");
        return -1;
    }
	printf("Author:JHY\n");
    memset( Events, 0, sizeof(Events));

    Events[0].fd = fd;
    Events[0].events = POLLIN;
    Events[1].fd = fileno(stdin);
    Events[1].events = POLLIN;	
	
    while(loopFlag)
    {
        ret = poll(Events, 2, 1000);
        if(ret==0)
        {
//          printf("poll time out : %d\n",cnt++);
            continue;
        }
		ssize_t bytes_read = read(fd, &data, sizeof(data));
		if (bytes_read != sizeof(data)) {
			perror("Failed to read data from device");
			close(fd);
			return -1;
		}		
		if(Events[0].revents & POLLIN)  //fd : keyled
        {			
			ioctl(fd, KEYVAL_READ, &info);
            printf("key_no : %d\n",info.key_no);
            switch(info.key_no)
            {
                case 1:
            printf("TIMER STOP! \n");
            ioctl(fd,TIMER_STOP);
			printf("Enter Manual PWM value! \n");  //수동 pwm 모드
                    break;
                case 2:
            printf("auto PWM mode \n");				
            ioctl(fd,TIMER_START);
                    break;
                case 3:
            printf("APP CLOSE ! \n");
            ioctl(fd,TIMER_STOP);
            loopFlag = 0;
                break;
            }
        }
        else if(Events[1].revents & POLLIN) //keyboard
        {
			fflush(stdin); //입력 버퍼를 비워주는 역할
            fgets(inputString,sizeof(inputString),stdin);
            if((inputString[0] == 'q') || (inputString[0] == 'Q'))
                break;
            inputString[strlen(inputString)-1] = '\0';		

            if(info.key_no == 1) //timer value
            {
                pwm_val = atoi(inputString);
				info.pwm_val=pwm_val;
                ioctl(fd,PWM_VALUE,&info);
            }
            info.key_no = 0;
        }
		
		if(oldData < data-5 || oldData > data+5)
		{
			printf("Data read from bh1750 sensor: %ld\n", data);
		}		
		ssize_t bytes_write = write(fd, &data, sizeof(data));
		if (bytes_read != sizeof(data)) {
			perror("Failed to read data from device");
			close(fd);
			return -1;
		}				
		oldData = data;	
		info.key_no = 0;	
	}	
	
    close(fd);
    return 0;
}
