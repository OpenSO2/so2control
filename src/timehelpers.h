#ifndef _timehelpersh_
#define _timehelpersh_
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef WIN
	#include<windows.h>
#else
	#define _POSIX_C_SOURCE 200809L
	#include<unistd.h>		/* usleep */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Structures */
typedef struct {
	int milli;		/* milliseconds after second */
	int sec;		/* seconds after the minute */
	int min;		/* minutes after the hour */
	int hour;		/* hours since midnight */
	int day;		/* day of the month */
	int mon;		/* month of the year */
	int year;		/* years since year 0 */
} timeStruct;

/* Functions */
int sleepMs(int);
int getTime(timeStruct *);
time_t TimeFromTimeStruct(const timeStruct *);
long getTimeStamp(void);

#ifdef __cplusplus
}
#endif

#endif
