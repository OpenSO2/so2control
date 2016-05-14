#include "spectrometer-shutter.h"
#include "log.h"
#include "timehelpers.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#ifdef _WIN32
#define O_NOCTTY 0
#else
#include <termios.h>
#endif

#define SPECTROMETER_SHUTTER_OPEN 7100
#define SPECTROMETER_SHUTTER_CLOSED 5400

static int fd = -1;
static int channel = -1;

// Sets the target of a Maestro channel.
// See the "Serial Servo Commands" section of the user's guide.
// The units of 'target' are quarter-microseconds.
int maestroSetTarget(unsigned short target);
int maestroSetTarget(unsigned short target)
{
	unsigned char command[] = {0x84, channel, target & 0x7F, target >> 7 & 0x7F};
	if (write(fd, command, sizeof(command)) == -1) {
		log_error("error writing");
		return -1;
	}
	return 0;
}

int spectrometer_shutter_init(sConfigStruct * config)
{
	#ifdef POSIX
		struct termios options;
	#endif
	const char * device = config->spectrometer_shutter_device;
	channel = config->spectrometer_shutter_channel;

	if(channel > 5 || channel < 0){
		log_error("spectrometer_shutter channel insane");
		close(fd);
		return 2;
	}
	log_debug("opening device %s on channel %i", device, channel);

	fd = open(device, O_RDWR | O_NOCTTY);
	if (fd == -1) {
		log_error("could not open spectrometer_shutter device");
		return 1;
	}

	#ifdef POSIX
		tcgetattr(fd, &options);
		options.c_iflag &= ~(INLCR | IGNCR | ICRNL | IXON | IXOFF);
		options.c_oflag &= ~(ONLCR | OCRNL);
		options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
		tcsetattr(fd, TCSANOW, &options);
	#else
		_setmode(fd, _O_BINARY);
	#endif

	return 0;
}

int spectrometer_shutter_open(void)
{
	int state = maestroSetTarget(SPECTROMETER_SHUTTER_OPEN);
	/* FIXME */
	sleepMs(100);
	return state;
}

int spectrometer_shutter_close(void)
{
	int state = maestroSetTarget(SPECTROMETER_SHUTTER_CLOSED);
	/* FIXME */
	sleepMs(100);
	return state;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int spectrometer_shutter_uninit(sConfigStruct * config)
{
	close(fd);
	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"
