/*
 * This file provides support for the Silicon Active Framegrabber via the
 * PHX SDK, which is not part of this program and has to be acquired and installed
 * separately.
 */
#include<phx_api.h>
#include<phx_os.h>
#include"log.h"
#include"camera.h"

/* prototypes for private functions */
void internalCallback(tHandle hCamera, ui32 dwInterruptMask, void *params);
static int sendMessage(tHandle hCamera, ui8 *inputBuffer);
int setup_camera(sParameterStruct * sSO2Parameters);

int camera_abort(sParameterStruct * sSO2Parameters)
{
	tHandle hCamera = sSO2Parameters->hCamera;
	log_debug("camera_abort");
	return PHX_Acquire(hCamera, PHX_ABORT, NULL);
}

int camera_uninit(sParameterStruct * sSO2Parameters)
{
	tHandle hCamera = sSO2Parameters->hCamera;
	log_debug("camera_uninit");
	return PHX_CameraRelease(&hCamera);
}

int camera_get(sParameterStruct * sSO2Parameters, int waiter)
{
	log_debug("trigger phx cam %c", sSO2Parameters->identifier);

/*FIXME: Comment*/
#pragma GCC diagnostic ignored "-Wpedantic"
	PHX_Acquire(sSO2Parameters->hCamera, PHX_START, internalCallback);
#pragma GCC diagnostic warning "-Wpedantic"

	if (waiter) {
		/* if theres no callback, this function will work synchronously
		 * and wait for the return of
		 */
		while (!sSO2Parameters->fBufferReady){
			sleepMs(1);
		}
	}

	return 0;
}

int camera_setExposure(sParameterStruct * sSO2Parameters)
{
	/* FIXME why cast to int when double? */
	int exposureTime = sSO2Parameters->dExposureTime;	/* exposure time in parameter structure */
	tHandle hCamera = sSO2Parameters->hCamera;	/* hardware handle for camera */
	etStat eStat = PHX_OK;	/* Phoenix status variable */
	int shutterSpeed = 1;	/* in case something went wrong this value is accepted by both modi */
	ui8 * message;
	char tmp[9];

	/* before doing anything check if exposure time is within the limits */
	if (exposureTime < 2.4 || exposureTime > 1004400) {
		log_error("Exposure time declared in Configfile is out of range: 2.4us < Exposure Time > 1004.4ms");
		return PHX_ERROR_OUT_OF_RANGE;
	}

	if (exposureTime <= 83560) {
		/* ===========ELECTRONIC=SHUTTER========== */
		shutterSpeed = round(((exposureTime - 2.4) / 79.275) + 1);
		sprintf(tmp, "SHT %d", shutterSpeed);

		/* N normal, S Shutter, F frameblanking */
		eStat = sendMessage(hCamera, (ui8*)"NMD S");
		if (PHX_OK != eStat) {
			log_error("setting camera to electronic shutter mode failed");
			return eStat;
		}
		/* Shutter speed, 1 - 1055 */
		exposureTime = 2.4 + (shutterSpeed - 1) * 79.275;
		log_message("Camera uses electronic shutter mode. exposure time is: %d ms", exposureTime);
	} else {
		/*===========FRAME=BLANKING==========*/
/*FIXME: is this correct? */
		shutterSpeed = round(exposureTime / 83700);
		sprintf(tmp, "FBL %d", shutterSpeed);

		/* N normal, S Shutter, F frameblanking */
		eStat = sendMessage(hCamera, (ui8*)"NMD F");
		if (PHX_OK != eStat) {
			log_error("Setting camera to frameblanking mode failed");
			return eStat;
		}
		/* Shutter speed, 1 - 12 */
		exposureTime = shutterSpeed * 83700;
		log_message("Camera uses Frameblanking mode. exposure time is: %d ms", exposureTime);
	}

	message = (ui8*)tmp;
	eStat = sendMessage(hCamera, message);
	if (PHX_OK != eStat) {
		log_error("setting shutter to %d failed (exposuretime %d ms)", shutterSpeed, exposureTime);
		return eStat;
	}
	return eStat;
}

int camera_init(sParameterStruct * sSO2Parameters)
{
	etStat eStat = PHX_OK;
	int channel = sSO2Parameters->identifier == 'a' ? PHX_CHANNEL_A : PHX_CHANNEL_B;

	/* Load the framegrabber with the phoenix configuration file. This function sets the necessary camera handles */
	eStat = PHX_CameraConfigLoad(&sSO2Parameters->hCamera,
		CAMERA_PCF_FILE,
		(etCamConfigLoad) PHX_BOARD_AUTO | PHX_DIGITAL | channel | PHX_NO_RECONFIGURE | 1,
		&PHX_ErrHandlerDefault);
	if (PHX_OK != eStat) {
		log_error("loading camera config failed");
		return eStat;
	}

	/* Setup our own event context */
	eStat = PHX_ParameterSet(sSO2Parameters->hCamera, PHX_EVENT_CONTEXT, (void *)sSO2Parameters);
	if (PHX_OK != eStat) {
		log_error("Setup the control flags structure failed");
		return eStat;
	}

	log_message("configuring of the frame grabber was successfull");

	return setup_camera(sSO2Parameters);
}

/*
 * The following functions are phx specific and not exposed
 */

int setup_camera(sParameterStruct * sSO2Parameters)
{
	etStat eStat = PHX_OK;
	tHandle hCamera = sSO2Parameters->hCamera;

	log_message("setup camera");

	/* initialize default values */
	eStat = sendMessage(hCamera, (ui8*)"INI");
	if (PHX_OK != eStat) {
		log_error("sending INI to camera was unsuccessfull");
		return eStat;
	}
	/* freerunning or external control mode: */
	/* N freerun mode, E external */
	eStat = sendMessage(hCamera, (ui8*)"AMD N");
	if (PHX_OK != eStat) {
		log_error("sending AMD N to camera was unsuccessfull");
		return eStat;
	}
	/* scanning mode: N Normal, S superpixel */
	eStat = sendMessage(hCamera, (ui8*)"SMD N");
	if (PHX_OK != eStat) {
		log_error("sending SMD N to camera was unsuccessfull");
		return eStat;
	}
	/* horizontal pixel output: M = 1344 */
	eStat = sendMessage(hCamera, (ui8*)"SHA M");
	if (PHX_OK != eStat) {
		log_error("sending SHA M to camera was unsuccessfull");
		return eStat;
	}
	/* contrast gain: high */
	eStat = sendMessage(hCamera, (ui8*)"CEG H");
	if (PHX_OK != eStat) {
		log_error("sending CEG H to camera was unsuccessfull");
		return eStat;
	}

	log_message("configuration of camera was successfull");
	return eStat;
}

void internalCallback(tHandle hCamera, ui32 dwInterruptMask, void *params)
{
	stImageBuff buffythevampireslayer;
	etStat eStat;
	sParameterStruct *sSO2Parameters = (sParameterStruct*) params;
	log_debug("internal camera callback called");

	/* Fifo Overflow */
	if ( PHX_INTRPT_FIFO_OVERFLOW & dwInterruptMask ) {
		log_error("a fifo overflow occured.");
	}

	/* Handle the Buffer Ready event */
	if ( PHX_INTRPT_BUFFER_READY & dwInterruptMask ) {
		eStat = PHX_Acquire(hCamera, PHX_BUFFER_GET, &buffythevampireslayer);
		if (PHX_OK != eStat) {
			log_error("frame couldn't be aquired");
		}

		sSO2Parameters->stBuffer = buffythevampireslayer.pvAddress;
		sSO2Parameters->hCamera = hCamera;
		sSO2Parameters->fBufferReady = (1==1);
	}
}

static int sendMessage(tHandle hCamera, ui8 *msg)
{
	etStat eStat = PHX_OK;
	int i;
	int sleepCycleCounter = 1;
	int timeout = 500;
	ui32 recvLength;
	ui8 recv[256];
	ui32 msgLength = (ui32)strlen((const char *)msg);

	/* 3 tries before the sending of of the message is considered failed */
	for (i = 0; i < 3; i++) {
		/* Transmit the serial data to the camera */
		eStat = PHX_CommsTransmit(hCamera, msg, &msgLength, timeout);
		if (PHX_OK != eStat) {
			log_error("PHX_CommsTransmit( %s ) failed", msg);
			continue; /* short circuit */
		}

		/* if transmitting was successful program waits for incoming messages
		 * 0.5s is a good compromise between speed and reliability
		 */
		do {
			_PHX_SleepMs(timeout / 50);
			sleepCycleCounter++;
			/* Check how many characters are waiting to be read */
			eStat = PHX_ParameterGet(hCamera, PHX_COMMS_INCOMING, &recvLength);
			/* create a timeout signal if 0.5s are over and no data was received */
			if (sleepCycleCounter > (timeout / 10))
				eStat = PHX_WARNING_TIMEOUT;
		} while (0 == recvLength && PHX_OK == eStat);

		if (PHX_OK != eStat) {
			log_error("nothing was received from camera");
			continue; /* short circuit */
		}

		/* if data is received, download the data */
		eStat = PHX_CommsReceive(hCamera, recv, &recvLength, timeout);
		if (PHX_OK != eStat) {
			log_error("nothing was received from camera");
			continue; /* short circuit */
		}

		if (strncmp((const char *)msg, (const char *)recv, recvLength) != 0) {
			/* if cameras answer equals input string, exit successfull */
			log_debug("send message: %s was successful", msg);
			return 0;	/* here return of SUCCESS */
		} else {
			log_error("String send and string received were not equal.");
		}
	}
	log_error("sending message failed 3 times");
	return eStat;
}
