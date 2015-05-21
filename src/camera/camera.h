/*
 * This implements the actual camera/framegrabber interface and will
 * mostly be a framegrabber SDK wrapper to allow for the undelying
 * SDK to be changed. This can also be used to mock the hardware
 * functions for unit/integration testing.
 * To support other framegrabbers, this file will have to be reimplemented.
 *
 *
 * A typical life cycle would be
 *
 * camera_init
 * camera_config
 * camera_setExposure
 *
 * camera_trigger
 * camera_get
 *
 * camera_abort
 * camera_uninit
 */
#ifndef _CAMERA_
#define _CAMERA_

#include<common.h>
#include<configurations.h>

/**
 * inits the camera/framegrabber
 */
int camera_init(sParameterStruct * sSO2Parameters);

/**
 * stops (uninits) the camera/framegrabber and does neccesarry clean up
 */
int camera_uninit(sParameterStruct * sSO2Parameters);

/**
 *
 */
int camera_trigger(sParameterStruct * sSO2Parameters,
		   void (*callbackFunction) (void *sSO2Parameters));

/**
 * aquires one image/frame from the camera/framegrabber
 */
int camera_get(sParameterStruct * sSO2Parameters, short **stBuffer);

/**
 * abort the current aquisition
 */
int camera_abort(sParameterStruct * sSO2Parameters);

/**
 *
 */
int camera_setExposureSwitch(sParameterStruct * sSO2Parameters, int timeSwitch);

/**
 *
 */
int camera_config(sParameterStruct * sSO2Parameters);

#endif
