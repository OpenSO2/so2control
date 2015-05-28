/*
-> sssync AA 0D 00 00 00 00 (bis zu 60 mal, 5ms+1ms*i interval)
<- ACK AA 0E 01 xx 00 00
-> initial RAW, VGA   AA 01 00 06 zz
<- ACK
-> Snapshot (uncompressed)  AA 05 01 00 00 00
<- ACK
-> GET PICTURE AA 04 01 00 00 00
<- ACK
<- DATA AA 0A 01 .. .. .. (image size)
-> ACK AA 0E 00 00 00 00 (package ID 0000h)
<- Image Data Package 512 chars ID 0001h
<- ACK AA 0E 00 00 01 00 (package ID 0001h)
.
.
<- last data package
-> ACK AA 0E 00 00 F0 F0 (package ID F0F0h)
*/

//~ BAUT 115200
//                     1  2  3  4  5  6
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <fcntl.h>
#include <termios.h>

static int number,n;
// FIXME: in config
static char * device = "/dev/ttyUSB0"; // "/dev/pts/11";
static int fd; /* File descriptor for the port */
speed_t baud = B115200; /* baud rate */
               //~ B115200
//~ speed_t baud = B57600; /* baud rate */

//~ static char * ACK   = "AA 0E 01 xx 00 00";
static char ack[6] = {0xAA, 0x0E, 0x0D, 0x0, 0x00, 0x00};
//~ static char * sssync  = "AA 0D 00 00 00 00";
static char sssync[6] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
//~ aa 0e 01 00 00 00
int isACK( char msg[6] ){
	//~ printf("compare msg and ack: %i\n", strcmp(msg, ack));

	if(    msg[0] == ack[0]
		&& msg[1] == ack[1]
		//~ && msg[2] == ack[2]
		//~ && msg[3] == ack[3]
		&& msg[4] == ack[4]
		&& msg[5] == ack[5]
	) return 0;

	return 1;
	//~ return strcmp(msg, ack);
}

int isSYNC( char msg[6] ){
	//~ printf("compare msg and ack: %i\n", strcmp(msg, ack));
	return strcmp(msg, sssync);
}

/**
 *
 */
int send(char * message, int messageLength){
	int n = write(fd, message, messageLength);
	return messageLength == n ? 0 : 1;
}

/**
 *
 */
int receive(char *buffer, int buffer_size)
{
	int j;
	int n = read(fd, buffer, buffer_size);
	printf("read %i bytes from port \n", n);

	if(n > 0){
		printf("received: ");

		for(j = 0; j < buffer_size; j++){
			printf("%02x ", buffer[j]);
		}
		printf("\n");
	}
	return n > 0 ? 0 : 1;
}

/**
 *
 */
int webcam_init()
{
	#define MSG_LENGTH 6
	int i = 0;
	int j = 0;
	int max_tries = 60;
	char ack[6] = {0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00};
	char msg[MSG_LENGTH];

	fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	//~ fd = open(device, O_NDELAY);
	if (fd == -1){
		printf("webcam_init: Unable to open %s\n", device);
	} else {
		printf("Port Opened successfully\n");
	}

	/* set the other settings (in this case, 9600 8N1) */
	struct termios settings;
	tcgetattr(fd, &settings);

	cfsetispeed(&settings, baud);
	cfsetospeed(&settings, baud);
	settings.c_cflag &= ~PARENB;      /* no parity */
	settings.c_cflag &= ~CSTOPB;      /* 1 stop bit */
	settings.c_cflag &= ~CSIZE;
	settings.c_cflag |= CS8 | CLOCAL; /* 8 bits */
	settings.c_lflag = ~ICANON;        /* canonical mode */
	settings.c_oflag &= ~OPOST;       /* raw output */

	tcsetattr(fd, TCSANOW, &settings); /* apply the settings */
	tcflush(fd, TCOFLUSH);

	for (i = 0; i<max_tries; i++){
		if( send(sssync, 6) == 0 ){
			printf("successfully send\n");
		} else {
			printf("failed to send\n");
		}

		usleep(1000*(5 + i));

		if( receive(msg, MSG_LENGTH) == 0 ){
			printf("received message\n");
		} else {
			printf("no response\n");
		}

		if(isACK(msg) == 0){
			printf("received ACK after %i tries\n", i);
			break;
		}
	}

	usleep(1000*15);

	// SYNC send and ACK received. Camera should now have send SYNC and we need to return ACK.
	if( receive(msg, MSG_LENGTH) == 0 ){
		if(isSYNC(msg) == 0){
			printf("communication established!\n");
		} else {
			printf("no sync received!\n");
		}

		send(ack, 6);
	}

	// setup             AA    05    01    00    00    00
	//~ char ini[6] = {0xAA, 0x05, 0x01, 0x00, 0x00, 0x00};

	                 //~ AA    01    00    07    07    07 //JPEG Snapshot Picture (640 x 480 resolution)
	//~ char ini[6] = {0xAA, 0x01, 0x00, 0x07, 0x07, 0x07};

	                 //~ AA    05    01    00    00    00  // Snapshot Picture (160 x 120 resolution, 16bit colour, uncompressed/RAW picture)
	//~ char ini[6] = {0xAA, 0x05, 0x01, 0x00, 0x00, 0x00};
//~ AA 01 00 06 03 zz
//~ AA 01 00 06 07 zz
	//~              AA    01    00    06    03    zz (01, 03, 05, 07)
	char ini[6] = {0xAA, 0x01, 0x00, 0x06, 0x07, 0x07};
	if( send(ini, 6) == 0 ){
		printf("successfully send init\n");

		usleep(1000*1500);

		if( receive(msg, MSG_LENGTH) == 0 ){
			if(isACK(msg) == 0){
				printf("initialized!\n");
			} else {
				printf("no ack for init received!\n");
			}
		} else {
			printf("no ack for init received\n");
		}

	} else {
		printf("failed to send\n");
	}

printf("webcam init finished\n");

	return 0;
//~ -> initial RAW, VGA   AA 01 00 06 zz

}

int webcam_uninit(){
	close(fd);
	printf("Port closed successfully\n");
}

/**
 *
 */
int request(){
	// request snapshot
	//               AA    05    00    00    00    00
	char msg[6] = {0xAA, 0x05, 0x00, 0x00, 0x00, 0x00}; // SNAPSHOT

	printf("SNAPSHOT\n");

	if( send(msg, 6) == 0 ){
		printf("successfully send SNAPSHOT\n");

		usleep(1000*15);

		if( receive(msg, MSG_LENGTH) == 0 ){
			if(isACK(msg) == 0){
				printf("SNAPSHOT acknowledged\n");
			} else {
				printf("SNAPSHOT not acknowledged\n");
			}
		} else {
			printf("SNAPSHOT ack not received\n");
		}
	} else {
		printf("SNAPSHOT not send\n");
	}

	usleep(1000*15);

	//                AA    04    01    00    00    00
	char msg2[6] = {0xAA, 0x04, 0x01, 0x00, 0x00, 0x00}; // GET PICTURE

	printf("GET PICTURE\n");

	if( send(msg2, 6) == 0 ){
		printf("successfully send GET PICTURE\n");

		usleep(1000*1500);

		if( receive(msg2, MSG_LENGTH) == 0 ){
			if(isACK(msg2) == 0){
				printf("GET PICTURE acknowledged\n");
			} else {
				printf("GET PICTURE not acknowledged\n");
			}
		} else {
			printf("GET PICTURE ack not received\n");
		}
	} else {
		printf("GET PICTURE not send\n");
	}
}

/**
 *
 */
int waitForData(){
	#define PACKAGE_SIZE 512
	char data[PACKAGE_SIZE];

	printf("waitForData\n");

	receive(data, PACKAGE_SIZE);


	//~ if( receive(msg, MSG_LENGTH) == 0 ){
		//~ if(isACK(msg) == 0){
			//~ printf("communication established!\n");
		//~ } else {
			//~ printf("no ack received!\n");
		//~ }
	//~ } else {
		//~ printf("nothing received\n");
	//~ }

}

int webcam_get(){
	printf("get\n");
	usleep(1000*2000);

	request();

	usleep(1000*3000);

	waitForData();

}

void main(void){
        webcam_init();
        webcam_get();
        webcam_uninit();
}


