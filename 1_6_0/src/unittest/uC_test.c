#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define COMM_MESSAGE_SIZE 7
#define COMM_BUFFER_SIZE 7*1024
int serial_port_fd = -1;
int serial_raw_read_last = 0;
int serial_raw_read_cur = 0;
char serial_read_buf[COMM_MESSAGE_SIZE + 1];


char * trimStr(char * s) {
    int i;
    char f[1000];
    int k=0;
    for (i=0;i<strlen(s);i++) {
        if (isspace(s[i])) continue;
        f[k++] = s[i];
    }
    f[k] = '\0';
    char *ff = f;
    return ff;
}

int open_serial_port() {
    printf("Entering: %s\n",__FUNCTION__);
    int fd = open("/dev/ttymxc2", O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1) {
        printf("Could not open UART\n");
        exit(0);
    }
    struct termios serial;
    bzero(&serial, sizeof(serial));

    if (tcgetattr(fd, &serial) < 0) {
        printf("Getting configuration\n");
        return -1;
    }
    // Set up Serial Configuration
    speed_t spd = B19200;
    cfsetospeed(&serial, (speed_t) spd);
    cfsetispeed(&serial, (speed_t) spd);

    cfmakeraw(&serial);

    serial.c_cc[VMIN] = 7;
    serial.c_cc[VTIME] = 0;

    serial.c_cflag &= ~CSTOPB;
    serial.c_cflag &= ~CRTSCTS; // no HW flow control
    serial.c_cflag |= CLOCAL | CREAD;
    if (tcsetattr(fd, TCSANOW, &serial) < 0) { // Apply configuration
        printf("Setting configuration failed\n");
        exit(0);
    }
    return fd;
}

int write_command_to_serial_port(char *comm) {
    printf("Entering: %s command: %s\n",__FUNCTION__, comm);
    int wcount = write(serial_port_fd, comm, strlen(comm));
    //printf("Sent [%d] characters strlen %d [%s]\n", wcount, strlen(comm), trimStr(comm));
    if (wcount < 0) {
        printf("Write to UART failed, trying again.\n");
        return -1;
    } else {
        if (wcount == strlen(comm)) {
            return 0;
        }
        printf("Write to UART verification error\n");
        return -1;
    }
}

int main(int argc, char **argv)
{
    int i;
    if(argc < 2)
    {
	printf("\tUsage: uC_test send UPS\n");
	printf("\tUsage: uC_test read\n");
    	return 0;
    }

    serial_port_fd = open_serial_port();
    if(serial_port_fd == -1)
    {
	printf("Failed to open serial port\n");
	return 0;
    }
    if(strstr("send", argv[1]))
    {
	char *cmd;
	cmd = argv[2];
        strcat(cmd,"\r\n");
	write_command_to_serial_port(cmd);
	//flush(serial_port_fd);

	sleep(1);
        char read_buf[8];
	int cnt;
	while(1)
        {
	    bzero((char *) &read_buf,8);
            cnt = read(serial_port_fd, &read_buf, 7);
	    if(cnt <= 0)
		break;
	    else
	        printf("Read: %d back at value %s", cnt, (char *) &read_buf);
	}

    }
    close(serial_port_fd);
}

