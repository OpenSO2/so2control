/*
 * This file defines some commonly used types
 *
 *
 *
 */
#ifndef _COMMON_
#define _COMMON_

#ifndef TRUE
#define TRUE     (1==1)
#define FALSE    (!TRUE)
#endif

#ifndef PHX
typedef unsigned long ui32;
typedef unsigned long mu32;
typedef mu32 tHandle;
typedef ui32 tFlag;

typedef struct {
	void *pvAddress;
	void *pvContext;
} stImageBuff;
#endif

int sleepMs(int x);

#endif
