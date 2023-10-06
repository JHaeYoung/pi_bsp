#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define DEVICE_FILENAME "/dev/illumiLCD"

int main() {
	
    int fd = open(DEVICE_FILENAME, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device file");
        return -1;
    }

    int data;
	int oldData=0;
	do{
		ssize_t bytes_read = read(fd, &data, sizeof(data));
		if (bytes_read != sizeof(data)) {
			perror("Failed to read data from device");
			close(fd);
			return -1;
		}
		if(oldData < data-5 || oldData > data+5)
		{
			printf("Data read from bh1750 sensor: %d\n", data);
		}
		
		ssize_t bytes_write = write(fd, &data, sizeof(data));
		if (bytes_read != sizeof(data)) {
			perror("Failed to read data from device");
			close(fd);
			return -1;
		}		
		
		oldData = data;		
	}while(1);
	
	
    close(fd);
    return 0;
}
