#include <stdio.h>		/* Standard input/output definitions */
#include <unistd.h>
#include "filterwheel.h"
#include "common.h"
#pragma GCC diagnostic ignored "-Wunused-parameter"
int filterwheel_init(sConfigStruct * config)
{
	return 0;
}

int filterwheel_send(int position)
{
	sleepMs(2000);
	return 0;
}

int filterwheel_uninit(sConfigStruct * config)
{
	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"
