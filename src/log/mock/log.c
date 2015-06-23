#include<stdio.h>
#include<stdarg.h>
#include<time.h>
#include "log.h"

int log_init(void)
{
	return 0;
}


int log_message(char * message)
{
	printf("MOCK LOG: %s \n", message);
	return 0;
}

int log_error(char * message)
{
	printf("MOCK ERROR: %s \n", message);
	return 0;
}

int log_debug(char * message, ... )
{
	char buffer[512];
	va_list args;
	char * format = "MOCK DEBUG: %s\n";

	va_start( args, message );
	vsprintf( buffer, message, args );
	va_end( args );

	printf(format, buffer);

	return 0;
}

int log_uninit(void)
{
	return 0;
}
