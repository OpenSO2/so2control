/*
 * This file defines some commonly used types
 *
 *
 *
 */
#ifndef _COMMON_
#define _COMMON_

#ifndef TRUE
#define TRUE     (1==1)
#define FALSE    (!TRUE)
#endif

/******************************
 *   Structures
 ******************************/
typedef struct {
	int milli;		/* milliseconds after second */
	int sec;		/* seconds after the minute */
	int min;		/* minutes after the hour */
	int hour;		/* hours since midnight */
	int day;		/* day of the month */
	int mon;		/* month of the year */
	int year;		/* years since year 0 */
} timeStruct;

int sleepMs(int x);

#endif
