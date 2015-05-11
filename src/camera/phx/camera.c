
int camera_init(handle){
	int status = 0;

	/* Load the framegrabber with the phoenix configuration file. The function sets the necessary camera handles */
	status = PHX_CameraConfigLoad( &handle, "configurations//c8484.pcf", (etCamConfigLoad)PHX_BOARD_AUTO | PHX_DIGITAL | PHX_CHANNEL_A | PHX_NO_RECONFIGURE | 1, &PHX_ErrHandlerDefault);
	if (0 != status )
	{
		logError("loading camera config failed");
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
	 */

	status = sendMessage(hCamera, "NMD S");
	if (0 != status )
	{
		logError("setting camera to electronic shutter mode failed");
		return status;
	}

	status = sendMessage(hCamera, "SHT 1055");
	if (0 != status )
	{
		logError("setting SHT value 1055 failed");
		return status;
	}
}

int camera_abort( handle ){
	PHX_Acquire( handle, PHX_ABORT, NULL );
}

int camera_stop( handle ){
	PHX_CameraRelease( handle );
}

int camera_get(hCamera, &stBuffer){
	return PHX_Acquire( hCamera, PHX_BUFFER_GET, &stBuffer );
}


int fixExposureTime(sParameterStruct *sSO2Parameters)
{
	int			exposureTime	= (int)sSO2Parameters->dExposureTime; /* exposure time in parameter structure */
	tHandle		hCamera			= sSO2Parameters->hCamera; /* hardware handle for camera */
	int			status			= 0; /* status variable */
	int			shutterSpeed	= 1; /* in case something went wrong this value is accepted by both modi */
	char		message[9];
	char		messbuff[512];
	char		errbuff[512];

	/* before doing anything check if exposure time is within the limits */
	if ( exposureTime < 2.4 || exposureTime > 1004400)
	{
		logError("Exposure time declared in Configfile is out of range: 2.4us < Exposure Time > 1004.4ms");
		status = 1;
		return status;
	}

	if (exposureTime <= 83560)
	{	//===========ELECTRONIC=SHUTTER==========

		shutterSpeed = roundToInt(((exposureTime - 2.4)/79.275)+1);
		sprintf(message,"SHT %d",shutterSpeed);

		// N normal, S Shutter, F frameblanking
		eStat = sendMessage(hCamera, "NMD S");
		if ( OK != eStat )
		{
			logError("setting camera to electronic shutter mode failed");
			return eStat;
		}

		// Shutter speed, 1 - 1055
		eStat = sendMessage(hCamera, message);
		if ( OK != eStat )
		{
			sprintf(errbuff,"setting SHT value to %d failed (exposuretime %d ms)",shutterSpeed,exposureTime);
			logError(errbuff);
			return eStat;
		}
		else
		{
			exposureTime = (int)(2.4 + (shutterSpeed-1) * 79.275);
			sprintf(messbuff,"Camera uses electronic shutter mode. exposure time is: %d ms",exposureTime);
			logMessage(messbuff);
		}
	}
	else
	{	//===========FRAME=BLANKING==========

		shutterSpeed = roundToInt(exposureTime/83700);
		sprintf(message,"FBL %d",shutterSpeed);

		// N normal, S Shutter, F frameblanking
		eStat = sendMessage(hCamera, "NMD F");
		if ( OK != eStat )
		{
			logError("Setting camera to frameblanking mode failed");
			return eStat;
		}

		// Shutter speed, 1 - 12
		eStat = sendMessage(hCamera, message);
		if ( OK != eStat )
		{
			sprintf(errbuff,"setting FBL value to %d failed (exposuretime %d ms)",shutterSpeed,exposureTime);
			logError(errbuff);
			return eStat;
		}
		else
		{
			exposureTime = shutterSpeed * 83700;
			sprintf(messbuff,"Camera uses Frameblanking mode. exposure time is: %d ms",exposureTime);
			logMessage(messbuff);
		}
	}
	return eStat;
}



int camera_setExposureSwitch(sParameterStruct *sSO2Parameters, int timeSwitch){
	switch(timeSwitch)
	{
		case 0 : //printf("starting electronic shutter mode\nExposuretime is set\n");
				sSO2Parameters->dExposureTime = 0.0000124+(1055-1)*0.000079275;
				logMessage("Camera is set to electronic shutter mode.");
				sprintf(messbuff,"Exposure time = %f ms",sSO2Parameters->dExposureTime);
				logMessage(messbuff);
				break;

		case 1: logMessage("Camera is set to frameblanking mode.");
				setFrameBlanking(sSO2Parameters);
				break;

		case 2: logMessage("Camera is set to electronic shutter mode.");
				setElektronicShutter(sSO2Parameters);
				break;

		case 3: logError("Contrast in image is to high to set an exposure time this is not fatal if this happens more often change values for -HistogramMinInterval- and -HistogramPercentage- in config file");
				logMessage("Camera is set to electronic shutter mode.");
				sprintf(messbuff,"Exposure time = %f ms",sSO2Parameters->dExposureTime);
				logMessage(messbuff);
				break;

		default:
				sprintf(errbuff,"unexpected value for -int timeSwitch- in setExposureTime(...) timeSwitch = %d",timeSwitch);
				logError(errbuff);
				return 1;
	}

}


/*
 * The following functions are phx specific and not exposed
 */



int getOneBuffer(sParameterStruct *sSO2Parameters, stImageBuff *stBuffer)
{
	/*  this function is very similar to startAquisition( ... ) */
	int			status			= 0; /* Status variable */
	tHandle		hCamera			= sSO2Parameters->hCamera; /* hardware handle for camera */
	int 		startErrCount	= 0; /* counting how often the start of capture process failed */

	/* Initiate a software trigger of the exposure control signal
	 * see triggerConfig()
	 */
	PHX_Acquire( hCamera, PHX_EXPOSE, NULL );

	do
	{
		/* start capture, hand over callback function*/
		status = camera_trigger( hCamera, (void*) callbackFunction );
		if ( 0 == status )
		{
			/* if starting the capture was successful reset error counter to zero */
			startErrCount = 0;
			/* Wait for a user defined period between each camera trigger call*/
			sleepMs( sSO2Parameters->dInterFrameDelay );

			/* Wait here until either:
			 * (a) The user aborts the wait by pressing a key in the console window
			 *     DEACTIVATE THIS LATER
			 * (b) The BufferReady flag is set indicating that the image is complete
			 * (c) The FIFO overflow flag is set indicating that the image is corrupt.
			 * Keep calling the sleep function to avoid burning CPU cycles
			 */
			while ( !sSO2Parameters->fBufferReady && !sSO2Parameters->fFifoOverFlow && !kbhit() )
			{
				sleepMs(10);
			}

			/* if BufferReady flag is set, reset it for next image */
			sSO2Parameters->fBufferReady = FALSE;

			/* download the buffer and place it in 'stBuffer' */
			status = camera_get( hCamera );
			if ( 0 != status )
			{
				logError("acquisition of one buffer for calculating the exposuretime failed (not fatal)");
				/* stopping the acquisition */
				camera_abort(hCamera);
				return eStat;
			}

			/* stopping the acquisition */
			camera_abort(hCamera);
		} // if ( OK == eStat )
		else
		{
			logError("starting acquisition for calculating the exposuretime failed (not fatal)");
			/* if starting the capture failed more than 3 times program stops */
			startErrCount++;
			if(startErrCount >= 3)
			{
				logError("Acquiring one buffer for calculating the exposuretime failed 3 times in a row. (fatal)");
				camera_abort(hCamera);
				return eStat;
			}
			camera_abort(hCamera);
		} // else
	/* loops only if something went wrong */
	} while (startErrCount != 0);
	return eStat;
}


int setFrameBlanking(sParameterStruct *sSO2Parameters)
{
	etStat			eStat		= OK; /* Phoenix status variable */
	tHandle			hCamera		= sSO2Parameters->hCamera; /* hardware handle for camera */
	stImageBuff		stBuffer; /* Buffer where the Framegrabber stores the image */
	char			message[9]; /* Message buffer for communication with camera */
	int				timeSwitch	= 1; /* Integer switch to switch between exposure modi */
	double			FBvalue		= 6; /* pre-set value 12/2=6 */
	double			divisor		= FBvalue; /* actual value send to camera */
	int 			switchMemory1 = 2; /* we need 2 memories because we */
	int 			switchMemory2 = 3; /* need to store the information over 2 loop-cycles */
	char			messbuff[512];
	char			errbuff[512];

	/* Switching to Frameblanking mode */
	eStat = sendMessage(hCamera,"NMD F");
	if ( OK != eStat )
	{
		logError("setting camera to frameblanking mode failed");
		return eStat;
	}

	logMessage("Starting to find right FB value");
	while(timeSwitch != 0 && timeSwitch != 3)
	{
		if(FBvalue > 12 || FBvalue < 1)
		{
			logError("FB value is out of range");
			break;
		}
		divisor = divisor / 2;
		/* it seems we are doing this twice not sure why */
		divisor = (double)roundToInt(divisor);

		sprintf(message,"FBL %d",roundToInt(FBvalue));
		eStat = sendMessage(hCamera,message);
		if (OK != eStat )
		{
			logError("setting FBL value failed failed");
			return eStat;
		}

		/* Acquire first buffer to decide between FBL or SHT */
		eStat = getOneBuffer(sSO2Parameters, &stBuffer);
		if ( OK != eStat )
		{
			logError("failed to obtain one image buffer");
			return eStat;
		}

		/* calculate histogram to test for over or under exposition */
		evalHist(&stBuffer, sSO2Parameters, &timeSwitch);


		/* a little bit hacky but it works */
		if (switchMemory2 == timeSwitch)
		{
			sprintf(messbuff,"setFrameBlanking(...) is stuck between FB values %d and %d. value is set to %d. This is not fatal", roundToInt(switchMemory1), roundToInt(switchMemory2), roundToInt(switchMemory2));
			logMessage(messbuff);
			timeSwitch = 0;
		}

		switch(timeSwitch)
		{
		case 0 : break;

		case 1: logMessage("Image is underexposed, FB-value is set up");
				FBvalue = FBvalue + divisor;

				break;

		case 2: logMessage("Image is overexposed, FB-value is set down");
				FBvalue = FBvalue - divisor;
				break;

		case 3: logError("Contrast in image is to high to set an exposure time this is not fatal if this happens more often change values for -HistogramMinInterval- and -HistogramPercentage- in config file");
				break;

		default:sprintf(errbuff,"unexpected value for -int timeSwitch- in setExposureTime(...) timeSwitch = %d",timeSwitch);
				logError(errbuff);
				return 1;
		}
		switchMemory2 = switchMemory1;
		switchMemory1 = timeSwitch;

	}
	/* save the exposure time in [ms] to control Struct */
	sSO2Parameters->dExposureTime = FBvalue * 0.0837;
	sprintf(messbuff,"Exposure time is set to %f", FBvalue * 0.0837);
	logMessage(messbuff);
	return eStat;
}

int setElektronicShutter(sParameterStruct *sSO2Parameters)
{
	etStat			eStat		= OK; /* Phoenix status variable */
	tHandle			hCamera		= sSO2Parameters->hCamera; /* hardware handle for camera */
	stImageBuff		stBuffer; /* Buffer where the Framegrabber stores the image */
	char			message[9]; /* Message buffer for communication with camera */
	int				timeSwitch 	= 1; /* Integer switch to switch between exposure modi */
	double			SHTvalue	= 528; /* SHT max = 1055 1055/2=527.5 -> 528 */
	double			divisor		= SHTvalue; /* actual value send to camera */
	int 			switchMemory1 = 2; /* we need 2 memories because we */
	int 			switchMemory2 = 3; /* need to store the information over 2 loop-cycles */
	char			messbuff[512];
	char			errbuff[512];

	/* Switching to Electronic Shutter mode */
	eStat = sendMessage(hCamera,"NMD S");
	if ( OK != eStat )
	{
		logError("setting camera to electronic shutter mode failed");
		return eStat;
	}

	logMessage("Starting to find right SHT value");
	while(timeSwitch != 0 && timeSwitch != 3)
	{
		if(SHTvalue > 1055 || SHTvalue < 1)
		{
			logError("SHT value is out of range");
			break;
		}

		divisor = divisor / 2.;

		sprintf(message,"SHT %d",roundToInt(SHTvalue));
		eStat = sendMessage(hCamera,message);
		if (OK != eStat )
		{
			logError("setting SHT value failed failed");
			return eStat;
		}

		/* Acquire first buffer to decide between FBL or SHT */
		eStat = getOneBuffer(sSO2Parameters, &stBuffer);
		if ( OK != eStat )
		{
			logError("failed to obtain one image buffer");
			return eStat;
		}

		/* calculate histogram to test for over or unter exposition */
		evalHist(&stBuffer, sSO2Parameters, &timeSwitch);

		/* a little bit hacky but it works */
		if (switchMemory2 == timeSwitch)
		{
			sprintf(errbuff,"setElectronicShutter(...) is stuck between SHT values %d and %d. value is set to %d. This is not fatal", roundToInt(switchMemory1), roundToInt(switchMemory2), roundToInt(switchMemory2));
			logError(errbuff);
			timeSwitch = 0;
		}

		switch(timeSwitch)
		{
		case 0 : break;

		case 1: logMessage("Image is underexposed, SHT-value is set up");
				SHTvalue = SHTvalue + divisor;
				break;

		case 2: logMessage("Image is overexposed, SHT-value is set down");
				SHTvalue = SHTvalue - divisor;
				break;

		case 3: logError("Contrast in image is to high to set an exposure time this is not fatal if this happens more often change values for -HistogramMinInterval- and -HistogramPercentage- in config file");
				break;

		default:sprintf(errbuff,"unexpected value for -int timeSwitch- in setExposureTime(...) timeSwitch = %d",timeSwitch);
				logError(errbuff);
				return 1;
		}
		switchMemory2 = switchMemory1;
		switchMemory1 = timeSwitch;
	}


	/* FIXME: GENAUIGEIT VON DEXPOSURETIME REICHT NICHT AUS!!!!!!!! */

	/* save the exposure time to control Struct */
	// FIXME: add documention
	sSO2Parameters->dExposureTime = 0.0000124+(SHTvalue-1)*0.000079275;
	sprintf(messbuff,"Exposure time is set to %f", 0.0000124+(SHTvalue-1)*0.000079275);
	logMessage(messbuff);
	return eStat;
}
