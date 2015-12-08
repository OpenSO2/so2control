#ifndef _timeh_
#define _timeh_
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

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
int sleepMs(int x);
int getTime(timeStruct * pTS);
time_t TimeFromTimeStruct(const timeStruct * pTime);

#endif
