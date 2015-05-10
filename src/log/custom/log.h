#ifndef __LOG__
#define __LOG__

/* global variable for logfile name */
char nameLogFile[256];

int initLog();
int logMessage(char *message);
int logError(char *message);
int logExit();
#endif
