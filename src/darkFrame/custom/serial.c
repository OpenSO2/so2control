#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

int number,n;
char * device = "/dev/pts/11";
void main(void){
	open_port("message", 7);
}

int open_port(char * message, int messageLength)
{
	int fd; /* File descriptor for the port */
	fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1){
		perror("open_port: Unable to open /dev/ttyACM0 - ");
	} else {
		printf("Port Opened successfully\n");
		n = write(fd, message, messageLength);
		if (n < 0) fputs("write() of 4 bytes failed!\n", stderr);
		else fputs("send successfully\n", stdout);
	}
}
