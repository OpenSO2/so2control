#ifndef _COMM_H_
#define _COMM_H_
#include "configurations.h"

#ifdef __cplusplus
extern "C" {
#endif

int comm_init(sConfigStruct * config);
int comm_set_buffer(char *cmd, char *buffer, int size);
int comm_uninit(sConfigStruct * config);

#ifdef __cplusplus
}
#endif

#endif /* _COMM_H_ */
