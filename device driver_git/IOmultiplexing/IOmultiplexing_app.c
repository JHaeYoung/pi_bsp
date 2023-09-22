#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>

#define DEVICE_FILENAME "/dev/ledkey_dev"

int main(int argc, char *argv[])
{
        int dev;
        char buff;
        int ret;
        int num = 1;
        struct pollfd Events[2];
        char keyStr[80];

    if(argc != 2)
    {
        printf("Usage : %s [led_data(0x00~0xff)]\n",argv[0]);
        return 1;
    }
    buff = (char)strtoul(argv[1],NULL,16);
    if((buff < 0x00) || (0xff < buff))
    {
        printf("Usage : %s [led_data(0x00~0xff)]\n",argv[0]);
        return 2;
    }

//  dev = open(DEVICE_FILENAME, O_RDWR | O_NONBLOCK);
	dev = open(DEVICE_FILENAME, O_RDWR );
	if(dev < 0)
	{
			perror("open");
			return 2;
	}
	write(dev,&buff,sizeof(buff));

	fflush(stdin);  // 키보드 입력 버퍼 초기화
	memset( Events, 0, sizeof(Events));
	Events[0].fd = fileno(stdin); //stdin 파일 포인터를 fileno로 디스크립트 넘버로 변경
	Events[0].events = POLLIN;  // 입력에 대한 이벤트를 체크함 | POLLOUT 으로 추가해도 된다.
	Events[1].fd = dev;  //읽기에 대한 이벤트 파일 디스크립트 저장
	Events[1].events = POLLIN;
	while(1)
	{
		ret = poll(Events, 2, 2000); // 장치, 장치파일 갯수, 2초마다 타임아웃
		if(ret<0)
		{
				perror("poll");
				exit(1);
		}
		else if(ret==0)
		{
				printf("poll time out : %d Sec\n",2*num++);
				continue;
		}
		if(Events[0].revents & POLLIN)  //stdin return events
		{
			fgets(keyStr,sizeof(keyStr),stdin);
			if(keyStr[0] == 'q')
					break;
			keyStr[strlen(keyStr)-1] = '\0'; //마지막 엔터키에 널문자를 입력해서 엔터키를 제거하자
			printf("STDIN : %s\n",keyStr);
			buff = (char)atoi(keyStr);
			write(dev,&buff,sizeof(buff));
		}
		else if(Events[1].revents & POLLIN) //ledkey
		{
			ret = read(dev,&buff,sizeof(buff));
			printf("key_no : %d\n",buff);
			buff = 1 << buff-1;
			write(dev,&buff,sizeof(buff));
			if(buff == 0x80)
				break;
		}
	}
	close(dev);
	return 0;
}
//fgets 함수를 만나면 엔터키 치기 전까지 슬립모드로 되기 때문에
//잠들어버리면 데이터가 아무리 들어와도 읽을 수 없기 떄문에
//입출력 다중화를 해서 