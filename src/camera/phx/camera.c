/*
 * This file provides support for the Silicon Active Framegrabber vie the
 * PHX SDK, thus this file is mostly just a wrapper:
 *
 * - camera_init -> PHX_CameraConfigLoad
 * - camera_get  -> PHX_Acquire
 * - camera_uninit -> PHX_CameraRelease
 */
#include<phx_api.h>
#include<phx_os.h>

#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include"configurations.h"

#define _PHX_LINE_SIZE		256

void PHXcallbackFunction(tHandle hCamera,	/* Camera handle. */
			 int dwInterruptMask,	/* Interrupt mask. */
			 sParameterStruct * sSO2Parameters	/* Pointer to user supplied context */
    )
{

	/* Fifo Overflow */
	if (PHX_INTRPT_FIFO_OVERFLOW & dwInterruptMask) {
		sSO2Parameters->fFifoOverFlow = TRUE;
	}

	/* Handle the Buffer Ready event */
	if (PHX_INTRPT_BUFFER_READY & dwInterruptMask) {
		callbackFunction(sSO2Parameters);
	}
}

int camera_init(sParameterStruct * sSO2Parameters)
{
	int status = 0;
	int channel =
	    sSO2Parameters->identifier == 'a' ? PHX_CHANNEL_A : PHX_CHANNEL_B;
	/* Load the framegrabber with the phoenix configuration file. The function sets the necessary camera handles */
	status =
	    PHX_CameraConfigLoad(&sSO2Parameters->hCamera,
				 "src/camera/phx/c8484.pcf",
				 (etCamConfigLoad) PHX_BOARD_AUTO | PHX_DIGITAL
				 | channel | PHX_NO_RECONFIGURE | 1,
				 &PHX_ErrHandlerDefault);

	if (0 != status) {
		log_error("loading camera config failed");
		return status;
	}

	/* Pre-set the Camera with a medium exposure time
	 * NMD: N, S, F
	 * N = Normal, S = Electronic Shutter, F = Frameblanking
	 * Speed:
	 * Frameblanking FBL: 1 - 12
	 * Electronic Shutter SHT = 1 - 1055
	 * FBL > SHT
	 * for conversion to milliseconds see camera manual
	 *

	 status = sendMessage(hCamera, "NMD S");
	 if (0 != status )
	 {
	 log_error("setting camera to electronic shutter mode failed");
	 return status;
	 }

	 status = sendMessage(hCamera, "SHT 1055");
	 if (0 != status )
	 {
	 log_error("setting SHT value 1055 failed");
	 return status;
	 }
	 /**/
	return status;
}

int camera_abort(sParameterStruct * sSO2Parameters)
{
	tHandle hCamera = sSO2Parameters->hCamera;
	return PHX_Acquire(hCamera, PHX_ABORT, NULL);
}

int camera_uninit(sParameterStruct * sSO2Parameters)
{
	tHandle hCamera = sSO2Parameters->hCamera;
	return PHX_CameraRelease(&hCamera);
}

int camera_get(sParameterStruct * sSO2Parameters, short **stBuffer)
{
	int status;
	stImageBuff buffythevampireslayer;

	status =
	    PHX_Acquire(sSO2Parameters->hCamera, PHX_BUFFER_GET,
			&buffythevampireslayer);
	*stBuffer = buffythevampireslayer.pvAddress;

	return status;
}

int camera_trigger(sParameterStruct * sSO2Parameters,
		   void (*callbackFunction) (sParameterStruct * sSO2Parameters))
{
	tHandle hCamera = sSO2Parameters->hCamera;
	return PHX_Acquire(hCamera, PHX_START, (void *)PHXcallbackFunction);
}

int camera_config(sParameterStruct * sSO2Parameters)
{
	int status;

	/* load the default configurations for the framegrabber */
	status = defaultConfig(sSO2Parameters);
	if (status != 0) {
		log_error("configuring camera %c failed");
		return status;
	}

	/* load the configurations for the exposure trigger */
	status = triggerConfig(sSO2Parameters);
	if (status != 0) {
		log_error("function triggerConfig(...) for camera 1 failed");
		return status;
	}

	/* set the camera with the right options */
	status = defaultCameraConfig(sSO2Parameters);
	if (status != 0) {
		log_error
		    ("function defaultCameraConfig(...) for camera 1 failed");
		return status;
	}
}

int fixExposureTime(sParameterStruct * sSO2Parameters)
{
	int exposureTime = (int)sSO2Parameters->dExposureTime;	/* exposure time in parameter structure */
	tHandle hCamera = sSO2Parameters->hCamera;	/* hardware handle for camera */
	int status = 0;		/* status variable */
	int shutterSpeed = 1;	/* in case something went wrong this value is accepted by both modi */
	char message[9];
	char messbuff[512];
	char errbuff[512];
	etStat eStat;

	/* before doing anything check if exposure time is within the limits */
	if (exposureTime < 2.4 || exposureTime > 1004400) {
		log_error
		    ("Exposure time declared in Configfile is out of range: 2.4us < Exposure Time > 1004.4ms");
		status = 1;
		return status;
	}

	if (exposureTime <= 83560) {
		/*===========ELECTRONIC=SHUTTER==========*/

		shutterSpeed = roundToInt(((exposureTime - 2.4) / 79.275) + 1);
		sprintf(message, "SHT %d", shutterSpeed);

		/* N normal, S Shutter, F frameblanking */
		eStat = sendMessage(hCamera, "NMD S");
		if (PHX_OK != eStat) {
			log_error
			    ("setting camera to electronic shutter mode failed");
			return eStat;
		}
		/* Shutter speed, 1 - 1055 */
		eStat = sendMessage(hCamera, message);
		if (PHX_OK != eStat) {
			sprintf(errbuff,
				"setting SHT value to %d failed (exposuretime %d ms)",
				shutterSpeed, exposureTime);
			log_error(errbuff);
			return eStat;
		} else {
			exposureTime = (int)(2.4 + (shutterSpeed - 1) * 79.275);
			sprintf(messbuff,
				"Camera uses electronic shutter mode. exposure time is: %d ms",
				exposureTime);
			log_message(messbuff);
		}
	} else {
		/* ===========FRAME=BLANKING========== */

		shutterSpeed = roundToInt(exposureTime / 83700);
		sprintf(message, "FBL %d", shutterSpeed);

		/* N normal, S Shutter, F frameblanking */
		eStat = sendMessage(hCamera, "NMD F");
		if (PHX_OK != eStat) {
			log_error("Setting camera to frameblanking mode failed");
			return eStat;
		}
		/* Shutter speed, 1 - 12 */
		eStat = sendMessage(hCamera, message);
		if (PHX_OK != eStat) {
			sprintf(errbuff,
				"setting FBL value to %d failed (exposuretime %d ms)",
				shutterSpeed, exposureTime);
			log_error(errbuff);
			return eStat;
		} else {
			exposureTime = shutterSpeed * 83700;
			sprintf(messbuff,
				"Camera uses Frameblanking mode. exposure time is: %d ms",
				exposureTime);
			log_message(messbuff);
		}
	}
	return eStat;
}

int camera_setExposure(sParameterStruct * sSO2Parameters)
{
	int exposureTime = (int)sSO2Parameters->dExposureTime;	/* exposure time in parameter structure */
	tHandle hCamera = sSO2Parameters->hCamera;	/* hardware handle for camera */
	etStat eStat = PHX_OK;	/* Phoenix status variable */
	int shutterSpeed = 1;	/* in case something went wrong this value is accepted by both modi */
	char message[9];
	char messbuff[512];
	char errbuff[512];

	/* before doing anything check if exposure time is within the limits */
	if (exposureTime < 2.4 || exposureTime > 1004400) {
		log_error
		    ("Exposure time declared in Configfile is out of range: 2.4us < Exposure Time > 1004.4ms");
		eStat = PHX_ERROR_OUT_OF_RANGE;
		return eStat;
	}

	if (exposureTime <= 83560) {
		/* ===========ELECTRONIC=SHUTTER========== */
		shutterSpeed = roundToInt(((exposureTime - 2.4) / 79.275) + 1);
		sprintf(message, "SHT %d", shutterSpeed);

		/* N normal, S Shutter, F frameblanking */
		eStat = sendMessage(hCamera, "NMD S");
		if (PHX_OK != eStat) {
			log_error
			    ("setting camera to electronic shutter mode failed");
			return eStat;
		}
		/* Shutter speed, 1 - 1055 */
		eStat = sendMessage(hCamera, message);
		if (PHX_OK != eStat) {
			sprintf(errbuff,
				"setting SHT value to %d failed (exposuretime %d ms)",
				shutterSpeed, exposureTime);
			log_error(errbuff);
			return eStat;
		} else {
			exposureTime = (int)(2.4 + (shutterSpeed - 1) * 79.275);
			sprintf(messbuff,
				"Camera uses electronic shutter mode. exposure time is: %d ms",
				exposureTime);
			log_message(messbuff);
		}
	} else {
		/*===========FRAME=BLANKING==========*/

		shutterSpeed = roundToInt(exposureTime / 83700);
		sprintf(message, "FBL %d", shutterSpeed);

		/* N normal, S Shutter, F frameblanking */
		eStat = sendMessage(hCamera, "NMD F");
		if (PHX_OK != eStat) {
			log_error("Setting camera to frameblanking mode failed");
			return eStat;
		}
		/* Shutter speed, 1 - 12 */
		eStat = sendMessage(hCamera, message);
		if (PHX_OK != eStat) {
			sprintf(errbuff,
				"setting FBL value to %d failed (exposuretime %d ms)",
				shutterSpeed, exposureTime);
			log_error(errbuff);
			return eStat;
		} else {
			exposureTime = shutterSpeed * 83700;
			sprintf(messbuff,
				"Camera uses Frameblanking mode. exposure time is: %d ms",
				exposureTime);
			log_message(messbuff);
		}
	}
	return eStat;
}

int camera_setExposureSwitch(sParameterStruct * sSO2Parameters, int timeSwitch)
{
	etStat eStat = PHX_OK;	/* Phoenix status variable */
	tHandle hCamera = sSO2Parameters->hCamera;	/* hardware handle for camera */
	stImageBuff stBuffer;	/* Buffer where the Framegrabber stores the image */
	char messbuff[512];
	char errbuff[512];

	switch (timeSwitch) {
	case 0:
		printf("starting electronic shutter mode\nExposuretime is set\n");
		sSO2Parameters->dExposureTime =
		    0.0000124 + (1055 - 1) * 0.000079275;
		log_message("Camera is set to electronic shutter mode.");
		sprintf(messbuff, "Exposure time = %f ms",
			sSO2Parameters->dExposureTime);
		log_message(messbuff);
		break;

	case 1:
		log_message("Camera is set to frameblanking mode.");
		setFrameBlanking(sSO2Parameters);
		break;

	case 2:
		log_message("Camera is set to electronic shutter mode.");
		setElektronicShutter(sSO2Parameters);
		break;

	case 3:
		log_error
		    ("Contrast in image is to high to set an exposure time this is not fatal if this happens more often change values for -HistogramMinInterval- and -HistogramPercentage- in config file");
		log_message("Camera is set to electronic shutter mode.");
		sprintf(messbuff, "Exposure time = %f ms",
			sSO2Parameters->dExposureTime);
		log_message(messbuff);
		break;

	default:
		sprintf(errbuff,
			"unexpected value for -int timeSwitch- in setExposureTime(...) timeSwitch = %d",
			timeSwitch);
		log_error(errbuff);
		return 1;
	}

}

/*
 * The following functions are phx specific and not exposed
 */

int getOneBuffer(sParameterStruct * sSO2Parameters, stImageBuff * stBuffer)
{
	/*  this function is very similar to startAquisition( ... ) */
	etStat eStat = PHX_OK;	/* Status variable */
	tHandle hCamera = sSO2Parameters->hCamera;	/* hardware handle for camera */
	int startErrCount = 0;	/* counting how often the start of capture process failed */

	/* Initiate a software trigger of the exposure control signal
	 * see triggerConfig()
	 */
	PHX_Acquire(hCamera, PHX_EXPOSE, NULL);

	do {
		/* start capture, hand over callback function */
		eStat =
		    PHX_Acquire(hCamera, PHX_START,
				(void *)PHXcallbackFunction);
		if (PHX_OK == eStat) {
			/* if starting the capture was successful reset error counter to zero */
			startErrCount = 0;
			/* Wait for a user defined period between each camera trigger call */
			_PHX_SleepMs(sSO2Parameters->dInterFrameDelay);

			/* Wait here until either:
			 * (a) The user aborts the wait by pressing a key in the console window
			 *     DEACTIVATE THIS LATER
			 * (b) The BufferReady flag is set indicating that the image is complete
			 * (c) The FIFO overflow flag is set indicating that the image is corrupt.
			 * Keep calling the sleep function to avoid burning CPU cycles
			 */
			while (!sSO2Parameters->fBufferReady
			       && !sSO2Parameters->fFifoOverFlow && !kbhit()) {
				_PHX_SleepMs(10);
			}

			/* if BufferReady flag is set, reset it for next image */
			sSO2Parameters->fBufferReady = FALSE;

			/* download the buffer and place it in 'stBuffer' */
			eStat = PHX_Acquire(hCamera, PHX_BUFFER_GET, stBuffer);
			if (PHX_OK != eStat) {
				log_error
				    ("acquisition of one buffer for calculating the exposuretime failed (not fatal)");
				/* stopping the acquisition */
				PHX_Acquire(hCamera, PHX_ABORT, NULL);
				return eStat;
			}

			/* stopping the acquisition */
			PHX_Acquire(hCamera, PHX_ABORT, NULL);
		}
		else {
			log_error
			    ("starting acquisition for calculating the exposuretime failed (not fatal)");
			/* if starting the capture failed more than 3 times program stops */
			startErrCount++;
			if (startErrCount >= 3) {
				log_error
				    ("Acquiring one buffer for calculating the exposuretime failed 3 times in a row. (fatal)");
				PHX_Acquire(hCamera, PHX_ABORT, NULL);
				return eStat;
			}
			PHX_Acquire(hCamera, PHX_ABORT, NULL);
		}
		/* loops only if something went wrong */
	} while (startErrCount != 0);
	return eStat;
}

/*
 * FIXME: merge setFrameBlanking with setElektronicShutter and document
 */

int setFrameBlanking(sParameterStruct * sSO2Parameters)
{
	etStat eStat = PHX_OK;	/* Phoenix status variable */
	tHandle hCamera = sSO2Parameters->hCamera;	/* hardware handle for camera */
	stImageBuff stBuffer;	/* Buffer where the Framegrabber stores the image */
	char message[9];	/* Message buffer for communication with camera */
	int timeSwitch = 1;	/* Integer switch to switch between exposure modi */
	double FBvalue = 6;	/* pre-set value 12/2=6 */
	double divisor = FBvalue;	/* actual value send to camera */
	int switchMemory1 = 2;	/* we need 2 memories because we */
	int switchMemory2 = 3;	/* need to store the information over 2 loop-cycles */
	char messbuff[512];
	char errbuff[512];

	/* Switching to Frameblanking mode */
	eStat = sendMessage(hCamera, "NMD F");
	if (PHX_OK != eStat) {
		log_error("setting camera to frameblanking mode failed");
		return eStat;
	}

	log_message("Starting to find right FB value");
	while (timeSwitch != 0 && timeSwitch != 3) {
		if (FBvalue > 12 || FBvalue < 1) {
			log_error("FB value is out of range");
			break;
		}
		divisor = divisor / 2;
		/* it seems we are doing this twice not sure why */
		divisor = (double)roundToInt(divisor);

		sprintf(message, "FBL %d", roundToInt(FBvalue));
		eStat = sendMessage(hCamera, message);
		if (PHX_OK != eStat) {
			log_error("setting FBL value failed failed");
			return eStat;
		}

		/* Acquire first buffer to decide between FBL or SHT */
		eStat = getOneBuffer(sSO2Parameters, &stBuffer);
		if (PHX_OK != eStat) {
			log_error("failed to obtain one image buffer");
			return eStat;
		}

		/* calculate histogram to test for over or under exposition */
		evalHist(&stBuffer, sSO2Parameters, &timeSwitch);

		/* a little bit hacky but it works */
		if (switchMemory2 == timeSwitch) {
			sprintf(messbuff,
				"setFrameBlanking(...) is stuck between FB values %d and %d. value is set to %d. This is not fatal",
				roundToInt(switchMemory1),
				roundToInt(switchMemory2),
				roundToInt(switchMemory2));
			log_message(messbuff);
			timeSwitch = 0;
		}

		switch (timeSwitch) {
		case 0:
			break;

		case 1:
			log_message("Image is underexposed, FB-value is set up");
			FBvalue = FBvalue + divisor;

			break;

		case 2:
			log_message
			    ("Image is overexposed, FB-value is set down");
			FBvalue = FBvalue - divisor;
			break;

		case 3:
			log_error
			    ("Contrast in image is to high to set an exposure time this is not fatal if this happens more often change values for -HistogramMinInterval- and -HistogramPercentage- in config file");
			break;

		default:
			sprintf(errbuff,
				"unexpected value for -int timeSwitch- in setExposureTime(...) timeSwitch = %d",
				timeSwitch);
			log_error(errbuff);
			return 1;
		}
		switchMemory2 = switchMemory1;
		switchMemory1 = timeSwitch;

	}
	/* save the exposure time in [ms] to control Struct */
	sSO2Parameters->dExposureTime = FBvalue * 0.0837;
	sprintf(messbuff, "Exposure time is set to %f", FBvalue * 0.0837);
	log_message(messbuff);
	return eStat;
}

int setElektronicShutter(sParameterStruct * sSO2Parameters)
{
	etStat eStat = PHX_OK;	/* Phoenix status variable */
	tHandle hCamera = sSO2Parameters->hCamera;	/* hardware handle for camera */
	stImageBuff stBuffer;	/* Buffer where the Framegrabber stores the image */
	char message[9];	/* Message buffer for communication with camera */
	int timeSwitch = 1;	/* Integer switch to switch between exposure modi */
	double SHTvalue = 528;	/* SHT max = 1055 1055/2=527.5 -> 528 */
	double divisor = SHTvalue;	/* actual value send to camera */
	int switchMemory1 = 2;	/* we need 2 memories because we */
	int switchMemory2 = 3;	/* need to store the information over 2 loop-cycles */
	char messbuff[512];
	char errbuff[512];

	/* Switching to Electronic Shutter mode */
	eStat = sendMessage(hCamera, "NMD S");
	if (PHX_OK != eStat) {
		log_error("setting camera to electronic shutter mode failed");
		return eStat;
	}

	log_message("Starting to find right SHT value");
	while (timeSwitch != 0 && timeSwitch != 3) {
		if (SHTvalue > 1055 || SHTvalue < 1) {
			log_error("SHT value is out of range");
			break;
		}

		divisor = divisor / 2.;

		sprintf(message, "SHT %d", roundToInt(SHTvalue));
		eStat = sendMessage(hCamera, message);
		if (PHX_OK != eStat) {
			log_error("setting SHT value failed failed");
			return eStat;
		}

		/* Acquire first buffer to decide between FBL or SHT */
		eStat = getOneBuffer(sSO2Parameters, &stBuffer);
		if (PHX_OK != eStat) {
			log_error("failed to obtain one image buffer");
			return eStat;
		}

		/* calculate histogram to test for over or unter exposition */
		evalHist(&stBuffer, sSO2Parameters, &timeSwitch);

		/* a little bit hacky but it works */
		if (switchMemory2 == timeSwitch) {
			sprintf(errbuff,
				"setElectronicShutter(...) is stuck between SHT values %d and %d. value is set to %d. This is not fatal",
				roundToInt(switchMemory1),
				roundToInt(switchMemory2),
				roundToInt(switchMemory2));
			log_error(errbuff);
			timeSwitch = 0;
		}

		switch (timeSwitch) {
		case 0:
			break;

		case 1:
			log_message
			    ("Image is underexposed, SHT-value is set up");
			SHTvalue = SHTvalue + divisor;
			break;

		case 2:
			log_message
			    ("Image is overexposed, SHT-value is set down");
			SHTvalue = SHTvalue - divisor;
			break;

		case 3:
			log_error
			    ("Contrast in image is to high to set an exposure time this is not fatal if this happens more often change values for -HistogramMinInterval- and -HistogramPercentage- in config file");
			break;

		default:
			sprintf(errbuff,
				"unexpected value for -int timeSwitch- in setExposureTime(...) timeSwitch = %d",
				timeSwitch);
			log_error(errbuff);
			return 1;
		}
		switchMemory2 = switchMemory1;
		switchMemory1 = timeSwitch;
	}

	/* FIXME: GENAUIGEIT VON DEXPOSURETIME REICHT NICHT AUS!!!!!!!! */

	/* save the exposure time to control Struct */
	/* FIXME: add documention */
	sSO2Parameters->dExposureTime =
	    0.0000124 + (SHTvalue - 1) * 0.000079275;
	sprintf(messbuff, "Exposure time is set to %f",
		0.0000124 + (SHTvalue - 1) * 0.000079275);
	log_message(messbuff);
	return eStat;
}

int triggerConfig(sParameterStruct * sSO2Parameters)
{
	/* in my opinion this function is the most complicated shit in the whole programm ;) */

	etStat eStat = PHX_OK;	/* Status variable */
	etParamValue eParamValue;
	ui32 dwTriggerPulseWidthUs = sSO2Parameters->dTriggerPulseWidth;
	tHandle hCamera = sSO2Parameters->hCamera;

	/* Enable the CCIO port as an output.
	 * This call is benign on Camera Link boards as CCIO is, by definition,
	 * an output only port. CCIO := Camera Control Input Output
	 */
	eParamValue = PHX_ENABLE;
	eStat =
	    PHX_ParameterSet(hCamera, PHX_IO_CCIO_OUT, (void *)&eParamValue);
	if (PHX_OK != eStat) {
		log_error("opening the CCIO port failed");
		return eStat;
	}

	/* Initialise the CCIO bit 1 pin as a negative going output driven from the exposure
	 * timer 1 with a pre-defined pulse width
	 */
	eParamValue = (etParamValue) (PHX_IO_METHOD_BIT_TIMER_NEG | 1);
	eStat = PHX_ParameterSet(hCamera, PHX_IO_CCIO, (void *)&eParamValue);
	if (PHX_OK != eStat) {
		log_error
		    ("Initialsing the CCIO bit 1 pin as a negative output for exposure timing failed");
		return eStat;
	}

	/* the trigger pulse width is define in the config file. min: 1 us */
	eParamValue = (etParamValue) dwTriggerPulseWidthUs;
	eStat =
	    PHX_ParameterSet(hCamera, PHX_IO_TIMER_1_PERIOD,
			     (void *)&eParamValue);
	if (PHX_OK != eStat) {
		log_error("setting the trigger pulse width failed");
		return eStat;
	}

	/* set the framegrabber that exposure is started by software trigger */
	eParamValue = PHX_EXPTRIG_SWTRIG;
	eStat =
	    PHX_ParameterSet(hCamera, PHX_EXPTRIG_SRC, (void *)&eParamValue);
	if (PHX_OK != eStat) {
		log_error
		    ("setting the exposure start to software trigger failed");
		return eStat;
	}

	log_message("trigger configuration was successfull");
	return eStat;
}

int defaultConfig(sParameterStruct * sSO2Parameters)
{
	etStat eStat = PHX_OK;	/* Status variable */
	etParamValue eParamValue;
	tHandle hCamera = sSO2Parameters->hCamera;

	/* Camera Communication Settings ( standard serial...)
	 * These settings are 9600 Baud, 8 data, no parity,
	 * 1 stop with no flow control */

	eParamValue = PHX_COMMS_DATA_8;
	eStat = PHX_ParameterSet(hCamera, PHX_COMMS_DATA, &eParamValue);
	if (PHX_OK != eStat) {
		log_error("setting of PHX_COMMS_DATA to camera failed");
		return eStat;
	}

	eParamValue = PHX_COMMS_STOP_1;
	eStat = PHX_ParameterSet(hCamera, PHX_COMMS_STOP, &eParamValue);
	if (PHX_OK != eStat) {
		log_error("setting of PHX_COMMS_STOP to camera failed");
		return eStat;
	}

	eParamValue = PHX_COMMS_PARITY_NONE;
	eStat = PHX_ParameterSet(hCamera, PHX_COMMS_PARITY, &eParamValue);
	if (PHX_OK != eStat) {
		log_error("setting of PHX_COMMS_PARITY to camera failed");
		return eStat;
	}

	eParamValue = (etParamValue) 9600;
	eStat = PHX_ParameterSet(hCamera, PHX_COMMS_SPEED, &eParamValue);
	if (PHX_OK != eStat) {
		log_error("setting of PHX_COMMS_SPEED to camera failed");
		return eStat;
	}

	eParamValue = PHX_COMMS_FLOW_NONE;
	eStat =
	    PHX_ParameterSet(hCamera,
			     (etParam) (PHX_COMMS_FLOW | PHX_CACHE_FLUSH),
			     &eParamValue);

	if (PHX_OK != eStat) {
		log_error("configuration of serial connection to camera failed");
		return eStat;
	}

	/* Image format settings
	 * 1344x1024, 12-Bit Source, 12-Bit Output,
	 * make use of the Data valid signal providetd
	 * by the CameraLink Camera */

	/* some cameras output a 'Data Enable' control signal to indicate when the data is valid.
	 * this option makes the framegrabber software to use such control signal */
	eParamValue = PHX_ENABLE;
	eStat = PHX_ParameterSet(hCamera, PHX_CAM_DATA_VALID, &eParamValue);
	if (PHX_OK != eStat) {
		log_error
		    ("make the framegrabber use the -data valid- signal failed");
		return eStat;
	}

	eParamValue = (etParamValue) 12;
	eStat = PHX_ParameterSet(hCamera, PHX_CAM_SRC_DEPTH, &eParamValue);
	if (PHX_OK != eStat) {
		log_error
		    ("Setting the image depth recieved from camera to 12-bit failed");
		return eStat;
	} else
		log_message("Image depth recieved from camera is set to 12-bit");

	/* these two options are commented out because they are set in the PHX config file. Somehow the right resolution only
	 * works if these options are set in the PHX config file. A goal would be to completly remove this config file but
	 * because of this two options whe still have to keep them */

	/*eParamValue= (etParamValue)1344;
	   eStat = PHX_ParameterSet(hCamera, PHX_CAM_ACTIVE_XLENGTH, (etParamValue*) &eParamValue );
	   if ( PHX_OK != eStat ) goto Error;

	   eParamValue = (etParamValue)1024;
	   eStat = PHX_ParameterSet(hCamera, PHX_CAM_ACTIVE_YLENGTH, (etParamValue*) &eParamValue );
	   if ( PHX_OK != eStat ) goto Error;
	 */
	eParamValue = PHX_DST_FORMAT_Y12;
	eStat = PHX_ParameterSet(hCamera, PHX_DST_FORMAT, &eParamValue);
	if (PHX_OK != eStat) {
		log_error("Setting the image file depth to 12-bit failed");
		return eStat;
	} else
		log_message("Image file depth is set to 12-bit");

	/* Enable FIFO Overflow events */
	eParamValue = PHX_INTRPT_FIFO_OVERFLOW;
	eStat = PHX_ParameterSet(hCamera, PHX_INTRPT_SET, &eParamValue);
	if (PHX_OK != eStat) {
		log_error("Set interpretation of FIFO overflows failed");
		return eStat;
	} else
		log_message("Right interpretation of FIFO overflows is set");

	/* Setup our own event context */
	eStat =
	    PHX_ParameterSet(hCamera, PHX_EVENT_CONTEXT,
			     (void *)sSO2Parameters);
	if (PHX_OK != eStat) {
		log_error("Setup the control flags structure failed");
		return eStat;
	}

	log_message("configuration of the Framegrabber was successfull");
	return 0;
}

int defaultCameraConfig(sParameterStruct * sSO2Parameters)
{
	etStat eStat = PHX_OK;
	tHandle hCamera = sSO2Parameters->hCamera;

	/* initialise default vaulues */
	eStat = sendMessage(hCamera, "INI");
	if (PHX_OK != eStat) {
		log_error("sending INI to camera was unsuccessfull");
		return eStat;
	}
	/* freerunning or external control mode: */
	/* N freerun mode, E external */
	eStat = sendMessage(hCamera, "AMD N");
	if (PHX_OK != eStat) {
		log_error("sending AMD N to camera was unsuccessfull");
		return eStat;
	}
	/* scanning mode: N Normal, S superpixel */
	eStat = sendMessage(hCamera, "SMD N");
	if (PHX_OK != eStat) {
		log_error("sending SMD N to camera was unsuccessfull");
		return eStat;
	}
	/* horizontal pixel output: M = 1344 */
	eStat = sendMessage(hCamera, "SHA M");
	if (PHX_OK != eStat) {
		log_error("sending SHA M to camera was unsuccessfull");
		return eStat;
	}
	/* contrast gain: high */
	eStat = sendMessage(hCamera, "CEG H");
	if (PHX_OK != eStat) {
		log_error("sending CEG H to camera was unsuccessfull");
		return eStat;
	}

	log_message("configuration of camera was successfull");
	return eStat;
}

int sendMessage(tHandle hCamera, char *inputBuffer)
{
	/* error handling is inverted in this function. cant remember why */
	etStat eStat = PHX_OK;
	etParamValue eParamValue;
	int i, sleepCycleCounter = 1;
	int timeout = 500;
	ui32 InputLineBufferLength = 0;
	ui32 OutputLineBufferLength = 0;
	char inputLineBuffer[_PHX_LINE_SIZE];
	char outputLineBuffer[_PHX_LINE_SIZE];
	char *pInputLineBuffer;
	pInputLineBuffer = inputLineBuffer;
	sprintf(pInputLineBuffer, "%s\r", inputBuffer);

	/* 3 tries before the sending of of the message is considered failed */
	for (i = 0; i < 3; i++) {
		/* Transmit the serial data to the camera */
		InputLineBufferLength = strlen(inputLineBuffer);
		eParamValue = (etParamValue) InputLineBufferLength;
		eStat =
		    PHX_CommsTransmit(hCamera, (ui8 *) inputLineBuffer,
				      (ui32 *) & eParamValue, timeout);
		if (PHX_OK == eStat) {
			/* if transmitting was successful program waits for incoming messages
			 * 0.5s is a good compromise between speed and reliability
			 */
			do {
				_PHX_SleepMs(timeout / 50);
				sleepCycleCounter++;
				/* Check how many characters are waiting to be read */
				eStat =
				    PHX_ParameterGet(hCamera,
						     PHX_COMMS_INCOMING,
						     &OutputLineBufferLength);
				/* create a timeout signal if 0.5s are over and no data was recieved */
				if (sleepCycleCounter > (timeout / 10))
					eStat = PHX_WARNING_TIMEOUT;

			} while (0 == OutputLineBufferLength
				 && PHX_OK == eStat);

			if (PHX_OK == eStat) {
				/* if data is recieved, download the data */
				eParamValue =
				    (etParamValue) OutputLineBufferLength;
				eStat =
				    PHX_CommsReceive(hCamera,
						     (ui8 *) outputLineBuffer,
						     (ui32 *) & eParamValue,
						     timeout);
				sprintf(outputLineBuffer, "%s\r", outputLineBuffer);
				if (PHX_OK == eStat) {
					if (strcmp
					    (inputLineBuffer,
					     outputLineBuffer) != 0) {
						/* if cameras answer equals input string, exit successfull */
						printf("DEBUG: send message: %s was successful\n",inputLineBuffer);
						return 0; /* here return of SUCCESS */
					} else {
						log_error
						    ("String send and string receive were not equal.");
					}
				}
				else
					log_error
					    ("nothing was received from camera");
			}
			else
				log_error("nothing was received from camera");
		}
		else
			log_error("PHX_CommsTransmit(...) failed");

	}
	log_error("sending message failed 3 times");
	return eStat;
}
