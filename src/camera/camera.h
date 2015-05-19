/*
 * This implements the actual camera/framegrabber interface and will
 * mostly be a framegrabber SDK wrapper to allow for the undelying
 * SDK to be changed. This can also be used to mock the hardware
 * functions for unit/integration testing.
 *
 * All functions accept hCamera handle as the first argument.
 *
 * To support other framegrabbers, this file will have to be reimplemented.
 *
 * The following functions are provided by this file:
 * camera_init - inits the camera/framegrabber
 * camera_get - aquires one image/frame from the camera/framegrabber
 * camera_stop - stops (uninits) the camera/framegrabber and does neccesarry clean up
 *
 * 
 * 
 * 
 * camera_init
 * camera_config
 * camera_setExposure
 * camera_trigger
 * camera_get 
 * camera_abort
 * camera_stop
 */
#ifndef _CAMERA_
#define _CAMERA_

#include<common.h>

/**
 *
 */
int camera_init(sParameterStruct *sSO2Parameters);

/**
 *
 */
int camera_stop(sParameterStruct *sSO2Parameters);

/**
 *
 */
int camera_trigger( sParameterStruct *sSO2Parameters, void (*callbackFunction)(void *sSO2Parameters) );

/**
 *
 */
int camera_get( sParameterStruct *sSO2Parameters, short **stBuffer );

/**
 *
 */
int camera_abort(sParameterStruct *sSO2Parameters);

/**
 *
 */
int camera_setExposureSwitch(sParameterStruct *sSO2Parameters, int timeSwitch);

/**
 *
 */
int camera_config(sParameterStruct *sSO2Parameters);

//~ int triggerConfig			(sParameterStruct *sSO2Parameters);
//~ int defaultConfig			(sParameterStruct *sSO2Parameters);
//~ int defaultCameraConfig		(sParameterStruct *sSO2Parameters);


#endif
