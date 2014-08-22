#include<stdio.h>
#include<windows.h>
#include "log.h"

int initLog()
{
	FILE		*logfile;
	SYSTEMTIME	logStartTime;
	GetSystemTime(&logStartTime);

	sprintf(nameLogFile,"logs/log_%04d_%02d_%02d_%02d_%02d.txt",logStartTime.wYear,
		logStartTime.wMonth, logStartTime.wDay, logStartTime.wHour, logStartTime.wMinute);

	logfile = getFile(nameLogFile);
	if(NULL == logfile){
		return 1;
	}
	fprintf(logfile,"Logfile for SO2-Camera Control Software\n");
	fprintf(logfile,"Today is the %02d.%02d.%04d %02d:%02d\n",
		logStartTime.wDay, logStartTime.wMonth, logStartTime.wYear,
		logStartTime.wHour, logStartTime.wMinute);
	fprintf(logfile,"=======================================\n");
	fprintf(logfile,"%02d:%02d:%02d | INFO  | Program started \n", logStartTime.wHour,
		logStartTime.wMinute, logStartTime.wSecond);
	fclose(logfile);
	return 0;
}

int logMessage(char *message)
{
	SYSTEMTIME time;
	FILE *logfile;
	char buffer[512];

	/* nameLogFile is a global variable */
	logfile = fopen(nameLogFile,"a");
	GetSystemTime(&time);
	sprintf(buffer,"%02d:%02d:%02d | INFO  | %s \n", time.wHour, time.wMinute, time.wSecond, message);
	fputs(buffer,logfile);
	printf(buffer);
	fclose(logfile);
	return 0;
}

int logError(char *message)
{
	SYSTEMTIME time;
	FILE *logfile;
	char buffer[512];

	/* nameLogFile is a global variable */
	logfile = fopen(nameLogFile,"a");
	GetSystemTime(&time);
	sprintf(buffer,"%02d:%02d:%02d | ERROR | %s \n", time.wHour, time.wMinute, time.wSecond, message);
	fprintf(stderr, buffer);
	fputs(buffer,logfile);
	fclose(logfile);
	return 0;
}

int logExit()
{
	FILE		*logfile;
	SYSTEMTIME	logStartTime;
	GetSystemTime(&logStartTime);

	logfile = fopen(nameLogFile,"a");
	fprintf(logfile,"=======================================\n");
	fprintf(logfile,"Today is the %02d.%02d.%04d %02d:%02d\n",
		logStartTime.wDay, logStartTime.wMonth, logStartTime.wYear,
		logStartTime.wHour, logStartTime.wMinute);
	fprintf(logfile,"The program exited this log file ends here\n \n");
	fclose(logfile);
	return 0;
}
