#ifndef __LOG__
#define __LOG__
#include "configurations.h"

#ifdef __cplusplus
extern "C" {
#endif

int log_init(sConfigStruct *);
int log_message(char *, ...);
int log_debug(char *, ...);
int log_error(char *, ...);
int log_uninit(void);

#ifdef __cplusplus
}
#endif
#endif
