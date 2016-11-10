/*
 * This file forks twice:
 * - once to enter into a non-blocking loop to accept() incomming connections
 * - for each connection to handle the request
 * Childprocesses are killed on sigterm.
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
#include <sys/stat.h>		/* For mode constants */
#include <fcntl.h>		/* For O_* constants */
#include <signal.h>
#include <sys/wait.h>

#include "timehelpers.h"
#include "comm.h"
#include "log.h"

int handle_socket(int newsockfd);
int send_all(int socket, char *ptr, size_t length);
int send_buf(int newsockfd, char *cmd, int size);
int init(int sockfd);
int recv_all(int socket, char *ptr, size_t length);
void kill_socket_processes(int reason);
char *get_buf(char *cmd, int size);

pid_t *socket_pids;
int socket_pids_length = 0;
int *sockets;
int sockets_length;

typedef struct {
	int topSize;
	int botSize;
	int camSize;
	int spcSize;
	int readLock;
	int writeLock;
} buffersStruct;

static buffersStruct *buffers;

char *buffers_top;
char *buffers_bot;
char *buffers_cam;
char *buffers_spc;

void kill_socket_processes(int reason)
{
	int i, status;
	pid_t result;
	log_debug("there are %i socket processes to kill for reason %i", socket_pids_length, reason);

	// close sockets
	for (i = 0; i < sockets_length; i++) {
		log_message("close socket %i", sockets[i]);
		close(sockets[i]);
	}

	// kill subprocesses
	for (i = 0; i < socket_pids_length; i++) {
		log_message("kill socket process with pid %i", socket_pids[i]);
		result = waitpid(socket_pids[i], &status, WNOHANG);
		if (result == 0) {
			kill(socket_pids[i], SIGTERM);
		} else {
			log_debug("... nevermind, process is dead already");
		}
	}
}

int send_all(int socket, char *ptr, size_t length)
{
	int i;
	while (length > 0) {
		log_debug("send_all while %i on socket %i with pointer %i; %i %i", (int)length, socket, ptr, ptr[30], ptr[40]);
		i = send(socket, ptr, length, 0);
		log_debug("send accomplished! Return value: %i", (int)i);

		if (i < 1)
			return -1;

		ptr += i;
		length -= i;
	}
	return 0;
}

int recv_all(int socket, char *ptr, size_t length)
{
	while (length > 0) {
		int i = recv(socket, ptr, length, 0);
		if (i < 0)
			return -1;
		ptr += i;
		length -= i;
	}
	return 0;
}

char *get_buf(char *cmd, int size)
{
	int fd;
	char file[13];
	char *base = "/buffers.";
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
	if (sockfd < 0) {
		perror("ERROR opening socket");
		return -1;
	}

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(config->comm_port);

#pragma GCC diagnostic ignored "-Wstrict-aliasing"
	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
#pragma GCC diagnostic warning "-Wstrict-aliasing"
		log_error("ERROR on binding to port %i, maybe that port is occupied or restricted?", config->comm_port);
		return -1;
	}

	listen(sockfd, 5);
	log_message("Listening to tcp socket on port %i", config->comm_port);

	// do communication in own process and return to main thread
	if ((pid = fork()) < 0) {
		log_error("Could not fork process: %s", strerror(errno));
		return -1;
	} else if (pid == 0) {	// child
		// replace signal handler inherited from parent process with my
		// own, which will clean up the processes that this process will
		// fork
		struct sigaction sa, osa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_handler = &kill_socket_processes;
		sigaction(SIGINT, &sa, &osa);
		sigaction(SIGTERM, &sa, &osa);

		init(sockfd);
		exit(0);
	} else {		// parent
		close(sockfd);
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
	while ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen))) {
#pragma GCC diagnostic warning "-Wstrict-aliasing"
		if (newsockfd < 0) {
			log_error("ERROR on accepting connection: %s", strerror(errno));
			return -1;
		}

		log_debug("new connection established for socket %i, new socket %i", sockfd, newsockfd);

		// spawn a process for every request
		if ((pid = fork()) < 0) {
			log_error("Error while forking %s", strerror(errno));
		} else if (pid == 0) {	// child
			// ignore signals in this process to avoid calling the
			// cleanup (uninit) functions multiple times
			signal(SIGTERM, SIG_DFL);
			signal(SIGINT, SIG_DFL);

			handle_socket(newsockfd);
			exit(0);
		} else {	// parent
			// remember all child pid to make sure to kill them later
			socket_pids_length++;
			socket_pids = realloc(socket_pids, socket_pids_length * sizeof(pid));
			socket_pids[socket_pids_length - 1] = pid;

			sockets_length++;
			sockets = realloc(sockets, sockets_length * sizeof(newsockfd));
			sockets[sockets_length - 1] = newsockfd;
		}
	}

	return 0;
}

int comm_set_buffer(char *cmd, char *buffer, int size)
{
	char *buf;

	// wait until all reads are completed
	while (buffers->readLock) {
		log_debug("wait for readlock");
		sleepMs(10);
	}

	// set a write Lock to prevent others to read a buffer while we are
	// updating its content
	buffers->writeLock = 1;

	// set size for each type of buffer
	if (strncmp(cmd, "top", 3) == 0) {
		buffers->topSize = size;
	} else if (strncmp(cmd, "bot", 3) == 0) {
		buffers->botSize = size;
	} else if (strncmp(cmd, "cam", 3) == 0) {
		buffers->camSize = size;
	} else if (strncmp(cmd, "spc", 3) == 0) {
		buffers->spcSize = size;
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

int send_buf(int newsockfd, char *cmd, int size)
{
	int n;
	char *buf;

	log_debug("Sending Picture Size (%i)", size);
	write(newsockfd, &size, sizeof(size));

	buf = get_buf(cmd, size);
	n = send_all(newsockfd, buf, size);
	if (n < 0) {
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

	while (1) {
		n = recv(newsockfd, &size, sizeof(int), 0);
		if (n < 0) {
			log_error("ERROR reading from socket");
			return -1;
		} else if (n == 0) {
			log_message("socket %i was closed by peer", newsockfd);
			return 0;
		}

		n = recv(newsockfd, cmd, size, 0);
		if (n < 0) {
			log_error("ERROR reading from socket");
			return -1;
		} else if (n == 0) {
			log_message("socket %i was closed by peer", newsockfd);
			return 0;
		}

		log_debug("received command '%s' of length %i", cmd, size);

		// wait until all buffers are written
		while (buffers->writeLock) {
			log_debug("wait for writelock");
			sleepMs(10);
		}

		// set readlock to avoid someone changing the buffer while we are reading it
		buffers->readLock = 1;
		if (strncmp(cmd, "top", 3) == 0) {
			send_buf(newsockfd, cmd, buffers->topSize);
		} else if (strncmp(cmd, "bot", 3) == 0) {
			send_buf(newsockfd, cmd, buffers->botSize);
		} else if (strncmp(cmd, "cam", 3) == 0) {
			send_buf(newsockfd, cmd, buffers->camSize);
		} else if (strncmp(cmd, "spc", 3) == 0) {
			send_buf(newsockfd, cmd, buffers->spcSize);
		} else {
			log_message("I did not understand this command: %s", cmd);
		}

		// clear read lock
		buffers->readLock = 0;
	}

	return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int comm_uninit(sConfigStruct * config)
{
	// cleanup mapped memory
	shm_unlink("/buffers.top");
	shm_unlink("/buffers.bot");
	shm_unlink("/buffers.cam");
	shm_unlink("/buffers.spc");

	free(socket_pids);

	return 0;
}

#pragma GCC diagnostic warning "-Wunused-parameter"
