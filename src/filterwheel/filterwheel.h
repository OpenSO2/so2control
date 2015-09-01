#ifndef _filterwheel_
#define _filterwheel_

#include "configurations.h"

#define FILTERWHEEL_CLOSED_A 1
#define FILTERWHEEL_CLOSED_B 1
#define FILTERWHEEL_OPENED_A 2
#define FILTERWHEEL_OPENED_B 2

int filterwheel_init(sConfigStruct * config);
int filterwheel_send(int position);
int filterwheel_uninit(sConfigStruct * config);
#endif
