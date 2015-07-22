#if defined(WIN)
#include<windows.h>		/* Sleep */
#else
#include<unistd.h>		/* usleep */
#endif
#include "common.h"

int sleepMs(int x)
{
#if defined(WIN)
	Sleep(x);
#else
	usleep(x * 1000);
#endif
	return 0;
}
