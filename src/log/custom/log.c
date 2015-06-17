#include<stdio.h>
#include<stdarg.h>
#include<time.h>
#include "log.h"

static FILE * logfile;
static char   nameLogFile[256];
static char   buffer[512];
static time_t time_ptr;
static struct tm logTime;

int log_init()
{
	time_t time_ptr;
	struct tm logTime;
	time(&time_ptr);
	logTime = *gmtime(&time_ptr);

	sprintf(nameLogFile, "logs/log_%04d_%02d_%02d_%02d_%02d.txt",
		logTime.tm_year + 1900, logTime.tm_mon,
		logTime.tm_mday, logTime.tm_hour,
		logTime.tm_min);

	logfile = fopen(nameLogFile, "a");
	if (NULL == logfile) {
		return 1;
	}
	fprintf(logfile, "Logfile for SO2-Camera Control Software\n");
	fprintf(logfile, "Today is the %02d.%02d.%04d %02d:%02d\n",
		logTime.tm_mday, logTime.tm_mon,
		logTime.tm_year + 1900, logTime.tm_hour,
		logTime.tm_min);
	fprintf(logfile, "=======================================\n");
	fprintf(logfile, "%02d:%02d:%02d | INFO  | Program started \n",
		logTime.tm_hour, logTime.tm_min, logTime.tm_sec);

	printf("*******************************************\n");
	printf("*                                         *\n");
	printf("*       SO2-Camera Control Software       *\n");
	printf("*                                         *\n");
	printf("*******************************************\n");
	printf("*                                         *\n");
	printf("* A control program for SO2 measuring     *\n");
	printf("* camera based on two Hamamatsu           *\n");
	printf("* C8484-16 CCD Cameras.                   *\n");
	printf("*                                         *\n");
	printf("* written by Johann Jacobson              *\n");
	printf("* and Morten Harms                        *\n");
	printf("*                                         *\n");
	printf("* johann.jacobson(at)zmaw.de              *\n");
	printf("* morten.harms(at)zmaw.de                 *\n");
	printf("*                                         *\n");
	printf("*******************************************\n");

	return 0;
}


int log_message(char * message)
{
	time(&time_ptr);
	logTime = *gmtime(&time_ptr);
	sprintf(buffer, "%02d:%02d:%02d | INFO  | %s \n", logTime.tm_hour,
		logTime.tm_min, logTime.tm_sec, message);
	fputs(buffer, logfile);
	printf("%s", buffer);
	return 0;
}

int log_error(char * message)
{
	time(&time_ptr);
	logTime = *gmtime(&time_ptr);
	sprintf(buffer, "%02d:%02d:%02d | ERROR | %s \n", logTime.tm_hour,
		logTime.tm_min, logTime.tm_sec, message);
	fprintf(stderr, "%s", buffer);
	fputs(buffer, logfile);
	return 0;
}

int log_debug(char * message, ... )
{
	va_list args;
	char * format = "%02d:%02d:%02d | DEBUG | %s\n";
	time(&time_ptr);
	logTime = *gmtime(&time_ptr);

	va_start( args, message );
	vsprintf( buffer, message, args );
	va_end( args );

	printf(format, logTime.tm_hour,
		logTime.tm_min, logTime.tm_sec, buffer);
	fprintf(logfile, format, logTime.tm_hour,
		logTime.tm_min, logTime.tm_sec, buffer);

	return 0;
}

int log_uninit()
{
	time(&time_ptr);
	logTime = *gmtime(&time_ptr);

	fprintf(logfile, "=======================================\n");
	fprintf(logfile, "Today is the %02d.%02d.%04d %02d:%02d\n",
		logTime.tm_mday, logTime.tm_mon, logTime.tm_year + 1900,
		logTime.tm_hour, logTime.tm_min);
	fprintf(logfile, "The program exited this log file ends here\n \n");

	fclose(logfile);
	return 0;
}
