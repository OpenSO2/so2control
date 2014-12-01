#include<stdio.h>
#include<time.h>
#include "log.h"

int initLog()
{
	FILE		*logfile;
	time_t time_ptr;
	struct tm logStartTime;
	time(&time_ptr);
	logStartTime = *gmtime(&time_ptr);

	sprintf(nameLogFile,"logs/log_%04d_%02d_%02d_%02d_%02d.txt", logStartTime.tm_year + 1900,
		logStartTime.tm_mon, logStartTime.tm_mday, logStartTime.tm_hour, logStartTime.tm_min);

	logfile = fopen(nameLogFile, "a");
	if(NULL == logfile){
		return 1;
	}
	fprintf(logfile,"Logfile for SO2-Camera Control Software\n");
	fprintf(logfile,"Today is the %02d.%02d.%04d %02d:%02d\n",
		logStartTime.tm_mday, logStartTime.tm_mon, logStartTime.tm_year + 1900,
		logStartTime.tm_hour, logStartTime.tm_min);
	fprintf(logfile,"=======================================\n");
	fprintf(logfile,"%02d:%02d:%02d | INFO  | Program started \n", logStartTime.tm_hour,
		logStartTime.tm_min, logStartTime.tm_sec);
	fclose(logfile);
	return 0;
}

int logMessage(char *message)
{
	FILE *logfile;
	char buffer[512];
	time_t time_ptr;
	struct tm logTime;
	time(&time_ptr);
	logTime = *gmtime(&time_ptr);

	/* nameLogFile is a global variable */
	logfile = fopen(nameLogFile,"a");
	sprintf(buffer,"%02d:%02d:%02d | INFO  | %s \n", logTime.tm_hour, logTime.tm_min, logTime.tm_sec, message);
	fputs(buffer, logfile);
	printf("%s", buffer);
	fclose(logfile);
	return 0;
}

int logError(char *message)
{
	FILE *logfile;
	char buffer[512];
	time_t time_ptr;
	struct tm logTime;
	time(&time_ptr);
	logTime = *gmtime(&time_ptr);

	/* nameLogFile is a global variable */
	logfile = fopen(nameLogFile,"a");

	sprintf(buffer,"%02d:%02d:%02d | ERROR | %s \n", logTime.tm_hour, logTime.tm_min, logTime.tm_sec, message);
	fprintf(stderr, "%s", buffer);
	fputs(buffer,logfile);
	fclose(logfile);
	return 0;
}

int logExit()
{
	FILE		*logfile;
	time_t time_ptr;
	struct tm logTime;
	time(&time_ptr);
	logTime = *gmtime(&time_ptr);

	logfile = fopen(nameLogFile,"a");
	fprintf(logfile,"=======================================\n");
	fprintf(logfile,"Today is the %02d.%02d.%04d %02d:%02d\n",
		logTime.tm_mday, logTime.tm_mon, logTime.tm_year + 1900,
		logTime.tm_hour, logTime.tm_min);
	fprintf(logfile,"The program exited this log file ends here\n \n");
	fclose(logfile);
	return 0;
}
