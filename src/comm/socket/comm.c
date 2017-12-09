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

int handle_socket(int);
int send_spectrum(int, double *, size_t);
int send_image(int, char *, size_t);
int send_buf(int, char *);
int init(int sockfd);
int recv_all(int, char *, size_t);
void kill_socket_processes(int);

pid_t *socket_pids;
int socket_pids_length = 0;
int *sockets;
int sockets_length;
pid_t accept_pid = 0;

typedef struct {
	int topSize;
	int botSize;
	int camSize;
	int spcSize;
	int readLock;
	int writeLock;
	char *top;
	char *bot;
	char *cam;
	double *spc;
} buffersStruct;

static buffersStruct *buffers;

void kill_socket_processes(int reason)
{
	int i, status;
	pid_t result;
	log_debug("there are %i socket processes to kill for reason %i", socket_pids_length, reason);

	/* close sockets */
	for (i = 0; i < sockets_length; i++) {
		log_message("close socket %i", sockets[i]);
		close(sockets[i]);
	}

	/* kill subprocesses */
	for (i = 0; i < socket_pids_length; i++) {
		log_message("kill socket process with pid %i", socket_pids[i]);
		result = waitpid(socket_pids[i], &status, WNOHANG);
		if (result == 0) {
			kill(socket_pids[i], SIGTERM);
			waitpid(socket_pids[i], NULL, 0);
		} else {
			log_debug("... nevermind, process is dead already");
		}
	}
}

int send_spectrum(int socket, double *ptr, size_t size)
{
	int written;

	while (size > 0) {
		written = write(socket, ptr, size);

		if (written < 1)
			return -1;

		ptr += written;
		size -= written;
	}

	return 0;
}

int send_image(int socket, char *ptr, size_t size)
{
	int written;

	while (size > 0) {
		written = write(socket, ptr, size);

		if (written < 1)
			return -1;

		ptr += written;
		size -= written;
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

int comm_init(sConfigStruct * config)
{
	int sockfd;
	struct sockaddr_in serv_addr;

	buffers = (buffersStruct *) mmap(NULL, sizeof *buffers, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	// FIXME:
	buffers->topSize = 100000000;
	buffers->botSize = 100000000;
	buffers->camSize = 100000000;
	buffers->spcSize = 100000000;

	buffers->top = (char *)mmap(NULL, buffers->topSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	buffers->bot = (char *)mmap(NULL, buffers->botSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	buffers->cam = (char *)mmap(NULL, buffers->camSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	buffers->spc = (double *)mmap(NULL, buffers->spcSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

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

	/*
	 * make sure to flush all buffers before forking to avoid duplicate
	 * output in logfile
	 */
	fflush(0);

	/* do communication in own process and return to main thread */
	accept_pid = fork();

	if (accept_pid < 0) {
		log_error("Could not fork process: %s", strerror(errno));
		return -1;
	} else if (accept_pid == 0) {
		/* child
		 * replace signal handler inherited from parent process with my
		 * own, which will clean up the processes that this process will
		 * fork
		 */
		struct sigaction sa, osa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_handler = &kill_socket_processes;
		sigaction(SIGINT, &sa, &osa);
		sigaction(SIGTERM, &sa, &osa);

		init(sockfd);
		exit(0);
	} else {
		/* parent
		 * wait for a bit to give the fork a chance to rid themselves of
		 * the old signal handlers in case the program crashes right now
		 * (which it totally never does) or is otherwise stopped
		 */
		sleep(1);
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
			if (errno == EINTR) {
				/* The system call was interrupted which is the desired behavior */
				return 0;
			}
			log_error("ERROR on accepting connection: %s", strerror(errno));
			return -1;
		}

		log_debug("new connection established for socket %i, new socket %i", sockfd, newsockfd);

		/*
		 * make sure to flush all buffers before forking to avoid duplicate
		 * output in logfile
		 */
		fflush(0);

		/* spawn a process for every request */
		if ((pid = fork()) < 0) {
			log_error("Error while forking %s", strerror(errno));
		} else if (pid == 0) {	/* child */
			/* ignore signals in this process to avoid calling the
			 * cleanup (uninit) functions multiple times
			 */
			signal(SIGTERM, SIG_DFL);
			signal(SIGINT, SIG_DFL);

			handle_socket(newsockfd);
			exit(0);
		} else {	/* parent */
			/* remember all child pid to make sure to kill them later */
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

int comm_set_buffer(char *cmd, void *buffer, int size)
{
	char *bufc;
	double *bufd;
	/* wait until all reads are completed  */
	while (buffers->readLock) {
		log_debug("wait for readlock");
		sleepMs(10);
	}

	/* set a write Lock to prevent others to read a buffer while we are
	 * updating its content
	 */
	buffers->writeLock = 1;


	/* set size for each type of buffer */
	if (strncmp(cmd, "top", 3) == 0) {
		bufc = buffers->top;
		buffers->topSize = size;
	} else if (strncmp(cmd, "bot", 3) == 0) {
		bufc = buffers->bot;
		buffers->botSize = size;
	} else if (strncmp(cmd, "cam", 3) == 0) {
		bufc = buffers->cam;
		buffers->camSize = size;
	} else if (strncmp(cmd, "spc", 3) == 0) {
		bufd = buffers->spc;
		buffers->spcSize = size;
	} else {
		fprintf(stderr, "incorrect buffer name\n");
		buffers->writeLock = 0;
		return -1;
	}
	if (strncmp(cmd, "spc", 3) == 0) {
		memmove(bufd, (double *)buffer, size*sizeof(double));
	} else {
		memmove(bufc, (char *)buffer, size*sizeof(char));
	}

	/* release lock */
	buffers->writeLock = 0;

	return 0;
}

int send_buf(int newsockfd, char *cmd)
{
	int n;
	int size;
	double *bufd;
	char *bufc;

	if (strncmp(cmd, "top", 3) == 0) {
		bufc = buffers->top;
		size = buffers->topSize*sizeof(bufc);
	} else if (strncmp(cmd, "bot", 3) == 0) {
		bufc = buffers->bot;
		size = buffers->botSize*sizeof(bufc);
	} else if (strncmp(cmd, "cam", 3) == 0) {
		bufc = buffers->cam;
		size = buffers->camSize*sizeof(bufc);
	} else if (strncmp(cmd, "spc", 3) == 0) {
		bufd = buffers->spc;
		size = buffers->spcSize*sizeof(bufd);
	} else {
		log_error("incorrect buffer name");
		return -1;
	}

	/* work around: dont send anything if the buffer has not yet been set */
	if(size == 100000000 || size == 800000000) size = 0;

	//~ log_debug("Sending command (%s)", cmd);
	//~ write(newsockfd, cmd, sizeof(cmd));

	log_debug("Sending buffer size (%i)", size);
	write(newsockfd, &size, sizeof(size));

	/* short circuit if nothing is to be send*/
	if(size == 0){
		log_debug("Buffer not ready, nothing send but length=0");
		return 0;
	}

	log_debug("Sending payload");
	if (strncmp(cmd, "spc", 3) == 0) {
		n = send_spectrum(newsockfd, bufd, size);
	} else {
		n = send_image(newsockfd, bufc, size);
	}
	log_debug("send accomplished! Return value: %i", n);

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
		} else if (n != size) {
			log_error("received message was incomplete");
		}

		log_debug("received command '%s' of length %i", cmd, size);

		/* wait until all buffers are written */
		while (buffers->writeLock) {
			log_debug("wait for writelock");
			sleepMs(10);
		}

		/* set readlock to avoid someone changing the buffer while we
		 * are reading it
		 */
		buffers->readLock = 1;
		if (strncmp(cmd, "top", 3) == 0) {
			send_buf(newsockfd, cmd);
		} else if (strncmp(cmd, "bot", 3) == 0) {
			send_buf(newsockfd, cmd);
		} else if (strncmp(cmd, "cam", 3) == 0) {
			send_buf(newsockfd, cmd);
		} else if (strncmp(cmd, "spc", 3) == 0) {
			send_buf(newsockfd, cmd);
		} else {
			log_message("I did not understand this command: %s", cmd);
		}

		/* clear read lock */
		buffers->readLock = 0;
	}

	return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int comm_uninit(sConfigStruct * config)
{

	if (accept_pid) {
		kill(accept_pid, SIGTERM);
		waitpid(accept_pid, NULL, 0);
	}

	free(socket_pids);

	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"
