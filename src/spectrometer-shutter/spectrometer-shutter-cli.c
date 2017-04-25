#include<stdio.h>
#include "spectrometer-shutter.h"

int main(int argc, char *argv[])
{
	sConfigStruct config;
	int status;
	char *a = argv[1];

	if (argc != 2 || (strcmp(a, "close") != 0 && strcmp(a, "open") != 0)) {
		printf("usage: spectrometer_shutter-cli <open|close>\n");
		return 1;
	}
#if defined(POSIX)
	config.spectrometer_shutter_device = "/dev/serial/by-id/usb-Pololu_Corporation_Pololu_Micro_Maestro_6-Servo_Controller_00135615-if00";
#else
	config.spectrometer_shutter_device = "\\\\.\\COM22";
#endif

	config.spectrometer_shutter_channel = 5;

	status = spectrometer_shutter_init(&config);
	if (status) {
		printf("failed to init spectrometer shutter\n");
		return status;
	}

	printf("%s shutter \n", a);

	status = strcmp(a, "close") == 0 ? spectrometer_shutter_close() : spectrometer_shutter_open();
	if (status) {
		printf("failed to %s spectrometer shutter\n", a);
		return status;
	}

	status = spectrometer_shutter_uninit(&config);
	if (status) {
		printf("failed to uninit spectrometer shutter\n");
		return status;
	}

	return 0;
}
