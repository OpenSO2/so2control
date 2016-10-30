#ifndef _COMM_H_
#define _COMM_H_
#include "configurations.h"
int comm_init(sConfigStruct * config);
int comm_set_buffer(char * cmd, char * buffer, int size);
int comm_uninit(sConfigStruct * config);
#endif /* _COMM_H_ */
