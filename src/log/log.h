#ifndef __LOG__
#define __LOG__
#include "configurations.h"

#ifdef __cplusplus
extern "C" {
#endif

int log_init(sConfigStruct * config);
int log_message(char *message, ...);
int log_debug(char *message, ...);
int log_error(char *message, ...);
int log_uninit(void);

#ifdef __cplusplus
}
#endif

#endif
