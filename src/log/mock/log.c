#include<stdio.h>
#include<stdarg.h>
#include<time.h>
#include "log.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
int log_init(sConfigStruct * config)
{
	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

int log_message(char *message)
{
	printf("MOCK LOG: %s \n", message);
	return 0;
}

int log_error(char *message)
{
	printf("MOCK ERROR: %s \n", message);
	return 0;
}

int log_debug(char *message, ...)
{
	char buffer[512];
	va_list args;

	va_start(args, message);
	vsprintf(buffer, message, args);
	va_end(args);

	printf("MOCK DEBUG: %s\n", buffer);

	return 0;
}

int log_uninit(void)
{
	return 0;
}
