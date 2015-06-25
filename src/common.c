#include<math.h>
#include<string.h>		/* memset */
#include<unistd.h>		/* usleep */

int roundToInt(double value)
{
	/* This function is necessary because round()
	 * seems not implemented in the VC6.0 version of "math.h"
	 */
	int result;
	double temp;

	temp = value - floor(value);
	if (temp >= 0.5)
		result = (int)(floor(value) + 1);
	else
		result = (int)(floor(value));

	return result;
}

int sleepMs(int x)
{
#if defined(WIN)
	Sleep(x);
#else
	usleep(x * 1000);
#endif
	return 0;
}
