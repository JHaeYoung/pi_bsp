#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>


#define DEVICE_FILENAME  "/dev/kerneltimer_dev"

int main(int argc, char* argv[])
{
    int dev;
    int ret=0;
    int cnt = 0;
    char key=0;
    int old_key=0;

    dev = open( DEVICE_FILENAME, O_RDWR|O_NDELAY );
    if(dev < 0)
    {
        perror("open");
        return 1;
    }
    while(1)
    {
        ret = read(dev,&key, sizeof(key));
        key = 1 << (key -1);
        if(key == 0)
            continue;
        if(key != old_key)
        {
			printf( "key = %.0f (%d)\n", log2(key)+1,cnt++);
			ret = write(dev,&key,sizeof(key));
			if(key == 0x80)
				break;
			old_key = key;
        }
    }
    close(dev);
    return 0;
}
