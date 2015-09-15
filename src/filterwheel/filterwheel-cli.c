#include<stdio.h>
#include "filterwheel.h"

int main(int argc, char *argv[])
{
	sConfigStruct config;
	int a;

	if (argc != 2) {
		printf("usage: filterwheel-cli <command>\n");
		return 1;
	}

	a = (int)*argv[1] - '0'; /* convert char to int */

	#if defined(POSIX)
	snprintf(config.filterwheel_device, 250, "%s", "/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_AI02PNA1-if00-port0");
	#else
	_snprintf_s(config.filterwheel_device, 250, "%s", "\\\\.\\COM22");
	#endif

	filterwheel_init(&config);
	printf("sending %i...\n", a);
	filterwheel_send(a);
	filterwheel_uninit(&config);

	return 0;
}
