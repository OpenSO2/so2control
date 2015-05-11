#ifndef _CAMERA_
#define _CAMERA_

#include<common.h>

/**
 *
 */
int camera_init(tHandle hCamera);

/**
 *
 */
int camera_uninit(tHandle hCamera);

/**
 *
 */
int camera_get( tHandle handle, short **stBuffer );

/**
 *
 */
int camera_abort(tHandle hCamera);

#endif
