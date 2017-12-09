#ifndef _COMM_H_
#define _COMM_H_
#include "configurations.h"

#ifdef __cplusplus
extern "C" {
#endif

int comm_init(sConfigStruct *);
int comm_set_buffer(char *, void *, int);
int comm_uninit(sConfigStruct *);

#ifdef __cplusplus
}
#endif

#endif /* _COMM_H_ */
