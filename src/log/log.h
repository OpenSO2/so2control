#ifndef __LOG__
#define __LOG__
#include "configurations.h"
int log_init(sConfigStruct * config);
int log_message(char *message);
int log_debug(char *message, ...);
int log_error(char *message);
int log_uninit(void);
#endif
