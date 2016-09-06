#include <stdio.h>   /* Standard input/output definitions */
#include "../filterwheel.h"
#include "timehelpers.h"
#include "log.h"

#if defined(POSIX)
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

static int fd = -1;  /* File descriptor for the port */
#else
#include <windows.h>
static HANDLE hSerial;
#endif

int filterwheel_init(sConfigStruct * config)
{
#ifdef POSIX
	fd = open(config->filterwheel_device, O_RDWR | O_NOCTTY | O_NDELAY);

	return fd == -1 ? 1 : 0;
#else
	/* Declare variables and structures */
	DCB dcbSerialParams = { 0 };
	COMMTIMEOUTS timeouts = { 0 };

	/* Open the highest available serial port number */
	fprintf(stderr, "Opening serial port...");
	hSerial = CreateFile(config->filterwheel_device, GENERIC_READ | GENERIC_WRITE, 0, NULL,
			     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hSerial == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Error\n");
		return 1;
	}
	/*
	 * Set device parameters (38400 baud, 1 start bit,
	 * 1 stop bit, no parity)
	 */
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (GetCommState(hSerial, &dcbSerialParams) == 0) {
		fprintf(stderr, "Error getting device state\n");
		return 1;
	}

	dcbSerialParams.BaudRate = CBR_38400;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (SetCommState(hSerial, &dcbSerialParams) == 0) {
		fprintf(stderr, "Error setting device parameters\n");
		return 1;
	}
	/* Set COM port timeout settings */
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	if (SetCommTimeouts(hSerial, &timeouts) == 0) {
		fprintf(stderr, "Error setting timeouts\n");
		return 1;
	}
	return 0;
#endif
}

/* position: o c */
int filterwheel_send(int position)
{
	int i;
	int bytes_to_send = 1;
	char buffer[80] = "";
#ifdef POSIX
	log_debug("write to filterwheel: %i", position);
	write(fd, &position, bytes_to_send);
	for (i=0;;i++){
		sleepMs(100);
		read(fd, buffer, 80);

		if (strstr(buffer, "done") != NULL) {
			return 0;
			log_debug("waited %i cycles per 100ms for filterwheel", i);
		}
	}
	sleepMs(2000);
	return 0;
#else
	/* Send specified text (remaining command line arguments) */
	DWORD bytes_written, total_bytes_written = 0;
	return WriteFile(hSerial, bytes_to_send, 5, &bytes_written, NULL);
#endif
}
#pragma GCC diagnostic ignored "-Wunused-parameter"
int filterwheel_uninit(sConfigStruct * config)
{
#if defined(POSIX)
	if(fd != -1)
		return close(fd);
#else
	return CloseHandle(hSerial);
#endif
	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"
