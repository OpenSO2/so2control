/*
 *
 *
 * This file forks twice:
 * - once to enter into a non-blocking loop to accept() incomming connections
 * - for each connection to handle the request
 *
 * TODO:
 * - expiresAt
 * - exit forks
 * - cleanup
 * - windows
 * - close port (?)
 *
 * valid buffernames are
 * - top - top camera
 * - bot - bot camera
 * - cam - webcam
 * - spc - spectrum
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h> /* For O_* constants */

#include "timehelpers.h"
#include "comm.h"
#include "log.h"


int handle_socket(int newsockfd);
int send_all(int socket, char * ptr, size_t length);
int send_buf(int newsockfd, char * cmd, int size);
int init(int sockfd);
char * get_buf(char * cmd, int size);

typedef struct {
	int topSize;
	long topExpiresAt;

	int botSize;
	long botExpiresAt;

	int camSize;
	long camExpiresAt;

	int spcSize;
	long spcExpiresAt;

	int readLock;
	int writeLock;
} buffersStruct;

static buffersStruct *buffers;

char * buffers_top;
char * buffers_bot;
char * buffers_cam;
char * buffers_spc;

int send_all(int socket, char * ptr, size_t length)
{
	int i;
	while (length > 0) {
		log_debug("send_all while %i on socket %i with pointer %i; %i %i", (int)length, socket, ptr, ptr[30], ptr[40]);
		i = send(socket, ptr, length, 0);
		log_debug("send accomplished! Return value: %i", (int)i);

		if (i < 1) return -1;

		ptr += i;
		length -= i;
	}
	return 0;
}

int recv_all(int socket, char * ptr, size_t length);
int recv_all(int socket, char * ptr, size_t length)
{
	while (length > 0) {
		int i = recv(socket, ptr, length, 0);
		if (i < 0) return -1;
		ptr += i;
		length -= i;
	}
	return 0;
}

char * get_buf(char * cmd, int size)
{
	int fd;
	char file[13];
	char * base = "/buffers.";
	memset(file, '\0', sizeof(file));
	strncat(file, base, 9);
	strncat(file, cmd, 3);

	fd = shm_open(file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	ftruncate(fd, size * sizeof(char));
	return (char *) mmap(NULL, size * sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
}

int comm_init(sConfigStruct * config)
{
	pid_t pid;
	int fd;
	int sockfd;
	struct sockaddr_in serv_addr;

	buffers = (buffersStruct *) mmap(NULL, sizeof *buffers, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	fd = shm_open("/buffers.top", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	buffers_top = (char *) mmap(NULL, 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	fd = shm_open("/buffers.bot", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	buffers_bot = (char *) mmap(NULL, 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	fd = shm_open("/buffers.cam", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	buffers_cam = (char *) mmap(NULL, 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	fd = shm_open("/buffers.spc", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	buffers_spc = (char *) mmap(NULL, 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	buffers->topSize = 1;
	buffers->botSize = 1;
	buffers->camSize = 1;
	buffers->spcSize = 1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		perror("ERROR opening socket");
		return -1;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(config->comm_port);

#pragma GCC diagnostic ignored "-Wstrict-aliasing"
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
#pragma GCC diagnostic warning "-Wstrict-aliasing"
		log_error("ERROR on binding to port %i, maybe that port is occupied or restricted?", config->comm_port);
		return -1;
	}

	listen(sockfd, 5);

	// do communication in own process and return to main thread
	if((pid = fork()) < 0) {
		log_error("Could not fork process: %s", strerror(errno));
		return -1;
	} else if(pid == 0) {
		init(sockfd);
		exit(0);
	}

	return 0;
}

int init(int sockfd)
{
	int newsockfd;
	socklen_t clilen;
	struct sockaddr_in cli_addr;
	pid_t pid;

	clilen = sizeof(cli_addr);

#pragma GCC diagnostic ignored "-Wstrict-aliasing"
	while( (newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) ){
#pragma GCC diagnostic warning "-Wstrict-aliasing"
		if (newsockfd < 0){
			log_error("ERROR on accepting connection");
			exit(-1);
		}

		// spawn a process for every request
		if((pid = fork()) < 0) {
			log_error("Error while forking %s", strerror(errno));
		} else if(pid == 0) {
			handle_socket(newsockfd);
			exit(0);
		}
	}

	return 0;
}

int comm_set_buffer(char * cmd, char * buffer, int size)
{
	char * buf;

	// wait until all reads are completed
	//~ while(buffers->readLock){
		//~ log_debug("wait for readlock");
		//~ sleep(.01);
	//~ }

	// set a write Lock to prevent others to read a buffer while we are
	// updating its content
	buffers->writeLock = 1;

	/* // FIXME comment
	 * for each type of buffer, reallocate buffer if size has changed
	 * memcpy buffer and set expire date in ms
	 */
	if(strcmp(cmd, "top") == 0){
		buffers->topSize = size;
		buffers->topExpiresAt = getTimeStamp() + 100;
	} else if(strcmp(cmd, "bot") == 0){
		buffers->botSize = size;
		buffers->botExpiresAt = getTimeStamp() + 100;
	} else if(strcmp(cmd, "cam") == 0){
		buffers->camSize = size;
		buffers->camExpiresAt = getTimeStamp() + 100;
	} else if(strcmp(cmd, "spc") == 0){
		buffers->spcSize = size;
		buffers->spcExpiresAt = getTimeStamp() + 100;
	} else {
		fprintf(stderr, "incorrect buffer name\n");
		buffers->writeLock = 0;
		return -1;
	}

	buf = get_buf(cmd, size);
	memcpy(buf, buffer, size);

	// release lock
	buffers->writeLock = 0;

	return 0;
}

int send_buf(int newsockfd, char * cmd, int size)
{
	int n;
	char * buf;

	log_debug("Sending Picture Size (%i)", size);
	write(newsockfd, &size, sizeof(size));

	buf = get_buf(cmd, size);
	n = send_all(newsockfd, buf, size);
	if (n < 0){
		log_error("ERROR writing to socket");
		return -1;
	}
	return 0;
}

int handle_socket(int newsockfd)
{
	int n = 0, size = 0;
	char cmd[3];
	memset(cmd, '\0', sizeof(cmd));

	while( 1 ){
		n = recv(newsockfd, &size, sizeof(int), 0);
		if (n < 0){
			log_error("ERROR reading from socket");
			return -1;
		}

		if(size != 3){
			continue;
		}

		n = recv(newsockfd, cmd, size, 0);
		if (n < 0){
			log_error("ERROR reading from socket");
			return -1;
		}

		log_debug("received command '%s' of length %i", cmd, size);

		buffers->readLock = 1;

		if(strcmp(cmd, "top") == 0){
			send_buf(newsockfd, cmd, buffers->topSize);
		} else if(strcmp(cmd, "bot") == 0){
			send_buf(newsockfd, cmd, buffers->botSize);
		} else if(strcmp(cmd, "cam") == 0){
			send_buf(newsockfd, cmd, buffers->camSize);
		} else if(strcmp(cmd, "spc") == 0){
			send_buf(newsockfd, cmd, buffers->spcSize);
		} else {
			log_message("I did not understand this command: %s", cmd);
		}

		// clear read lock
		buffers->readLock = 0;
	}

	close(newsockfd);

	return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int comm_uninit(sConfigStruct * config)
{
	/* FIXME:
	 *   cleanup processes
	 *   delete shm files?
	 */

	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"
