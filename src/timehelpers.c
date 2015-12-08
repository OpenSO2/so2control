#ifdef WIN
#include<windows.h>
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include "timehelpers.h"

#if defined(WIN)
#include<windows.h>		/* Sleep */
#else
#include<unistd.h>		/* usleep */
#endif

int sleepMs(int x)
{
#if defined(WIN)
	Sleep(x);
#else
	usleep(x * 1000);
#endif
	return 0;
}

int getTimeStamp(){
	int mills = 0;
	timeStruct time;
	getTime(&time);

	mills = time.milli
		+ time.sec  * 1000
		+ time.min  * 1000 * 60
		+ time.hour * 1000 * 60 * 60
		+ time.day  * 1000 * 60 * 60 * 24
		+ time.mon  * 1000 * 60 * 60 * 24 * 30
		+ time.year * 1000 * 60 * 60 * 24 * 365
	;

	return mills;
}

time_t TimeFromTimeStruct(const timeStruct * pTime)
{
	struct tm tm;
	memset(&tm, 0, sizeof(tm));

	tm.tm_year = pTime->year - 1900;
	tm.tm_mon = pTime->mon - 1;
	tm.tm_mday = pTime->day;
	tm.tm_hour = pTime->hour;
	tm.tm_min = pTime->min;
	tm.tm_sec = pTime->sec;

	return mktime(&tm);
}

#ifdef WIN

/* WINDOWS VERSION */
int getTime(timeStruct * pTS)
{
	SYSTEMTIME time;
	GetSystemTime(&time);
	pTS->year = time.wYear;
	pTS->mon = time.wMonth;
	pTS->day = time.wDay;
	pTS->hour = time.wHour;
	pTS->min = time.wMinute;
	pTS->sec = time.wSecond;
	pTS->milli = time.wMilliseconds;
	return 0;
}

#else

/* POSIX VERSION */
int getTime(timeStruct * pTS)
{
	time_t seconds;
	long milliseconds;
	struct tm *tm;
	struct timespec spec;
	int stat;

	stat = clock_gettime(CLOCK_REALTIME, &spec);
	if (stat != 0) {
		//~ log_error("clock_gettime failed. (posix) \n");
		return 1;
	}

	milliseconds = round(spec.tv_nsec / 1.0e6);
	seconds = spec.tv_sec;
	tm = gmtime(&seconds);

	pTS->year = tm->tm_year + 1900;
	pTS->mon = tm->tm_mon + 1;
	pTS->day = tm->tm_mday;
	pTS->hour = tm->tm_hour;
	pTS->min = tm->tm_min;
	pTS->sec = tm->tm_sec;
	pTS->milli = milliseconds;
	return 0;
}
#endif
