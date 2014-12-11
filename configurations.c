#include"configurations.h"
#include<string.h>
#include"common.h"
#define MAXBUF 1024
#include"log.h"


int structInit(sParameterStruct *sSO2Parameters,char identifier)
{
	sSO2Parameters->dImageCounter = 0;
	sSO2Parameters->eStat = PHX_OK;
	sSO2Parameters->dTriggerPulseWidth = 15;
	sSO2Parameters->dBufferlength = 1376256;
	sSO2Parameters->dHistMinInterval = 350;
	sSO2Parameters->dHistPercentage = 5;
	sSO2Parameters->dInterFrameDelay = 10;
	sSO2Parameters->fid = NULL;
	sSO2Parameters->identifier = identifier;
	return 0;
}

int configurations(sParameterStruct *sSO2Parameters, char identifier)
{
	etStat	eStat = PHX_OK;
	int		status = 0; /* status variable for return values */

	/*starting the logfile */
	if(identifier == 'a')
	{
		status = initLog();
		if(status != 0)
		{
			/* if creating a logfile fails we have to terminate the program. The error message then has to go directly to the screen */
			printf("creating a logfile failed. Program is aborting...\n");
			return status;
		}
	}
	/* initialise the parameterstructure with default values */
	structInit(sSO2Parameters, identifier);

	/* load the default configurations for the framegrabber */
	eStat = defaultConfig(sSO2Parameters);
	if(eStat != PHX_OK)
	{
		logError("function defaultConfig(...) for camera 1 failed");
		return eStat;
	}

	/* load the configurations for the exposure trigger */
	eStat = triggerConfig(sSO2Parameters);
	if(eStat != PHX_OK)
	{
		logError("function triggerConfig(...) for camera 1 failed");
		return eStat;
	}

	/* set the camera with the right options */
	eStat = defaultCameraConfig(sSO2Parameters);
	if(eStat != PHX_OK)
	{
		logError("function defaultCameraConfig(...) for camera 1 failed");
		return eStat;
	}

	/* name of Configfile is hard coded maybe change this sometime */
	status = readConfig("configurations//SO2Config.conf", sSO2Parameters);
	if(status != 0) logError("readConfig(...) failed");
	return status;
}

int readConfig(char *filename, sParameterStruct *sSO2Parameters)
{
	FILE *pFILE; /* filehandle for config file */
	char *lineBuf;/* buffer that holds the current line of the config file */
	char *delimeterBuf; /* buffer that holds the line after a specified delimeter */
	int n=1,i=0,delimIndex=0,linebreak=0, valueSize=0;
	int linenumber=1;
	double sizeOfImage = 2.7;
	int dTmp;
	char cTmp[MAXBUF]; /* a temporal buffer used when strings are read from the config file */
	char errbuff[MAXBUF]; /* a buffer to construct a proper error message */

	lineBuf = (char*) malloc(MAXBUF);

	pFILE = fopen(filename,"r");

	if (pFILE!=NULL)
	{
		while(fgets(lineBuf, MAXBUF, pFILE) != NULL)
		{
			/* skip lines which are marked as a commend */
			if(lineBuf[0] != '#')
			{
				/* search for corresponding strings */
				if (strstr(lineBuf,"HistogramMinInterval"))
				{
					delimeterBuf = strstr(lineBuf,"=");
					sSO2Parameters->dHistMinInterval = atoi(delimeterBuf+1);
				}

				else if (strstr(lineBuf,"HistogramPercentage"))
				{
					delimeterBuf = strstr(lineBuf,"=");
					sSO2Parameters->dHistPercentage = atoi(delimeterBuf+1);
				}

				else if ( strstr( lineBuf, "InterFrameDelay"))
				{
					delimeterBuf = strstr(lineBuf,"=");
					sSO2Parameters->dInterFrameDelay = atoi(delimeterBuf+1);
				}

				else if ( strstr( lineBuf, "TriggerPulseWidth" ) )
				{
					delimeterBuf = strstr(lineBuf,"=");
					sSO2Parameters->dTriggerPulseWidth = atoi(delimeterBuf+1);
				}

				else if ( strstr( lineBuf, "FixTime" ) )
				{
					delimeterBuf = strstr(lineBuf,"=");
					sSO2Parameters->dFixTime = atoi(delimeterBuf+1);
				}

				else if ( strstr( lineBuf, "ImageSize") )
				{
					delimeterBuf = strstr(lineBuf,"=");
					dTmp = atoi(delimeterBuf+1);
					sSO2Parameters->dfilesize = dTmp;
					sSO2Parameters->dImagesFile = (int)(dTmp/sizeOfImage);
				}

				else if ( strstr( lineBuf, "ExposureTime") )
				{
					delimeterBuf = strstr(lineBuf,"=");
					sSO2Parameters->dExposureTime = atoi(delimeterBuf+1);
				}
				else if(strstr(lineBuf,"FileNamePrefix"))
				{
					delimeterBuf = strstr(lineBuf,"=");
					if(delimeterBuf[1] == ' ')
						sprintf(cTmp,"%s",delimeterBuf+2);
					else
						sprintf(cTmp,"%s",delimeterBuf+1);

					/* remove LF */
					sprintf(sSO2Parameters->cFileNamePrefix,"%s",strtok(cTmp,"\n"));
				}
				else if(strstr(lineBuf,"ImagePath"))
				{
					delimeterBuf = strstr(lineBuf,"=");
					if(delimeterBuf[1] == ' ')
						sprintf(cTmp,"%s",delimeterBuf+2);
					else
						sprintf(cTmp,"%s",delimeterBuf+1);

					/* remove LF */
					sprintf(sSO2Parameters->cImagePath,"%s",strtok(cTmp,"\n"));
				}
			} //end if(lineBuf[0] != '#')
		linenumber++;
		} //end while(fgets(lineBuf, MAXBUF, pFILE) != NULL)
	fclose(pFILE);

	/* not an error but errbuff is used anyway */
	sprintf(errbuff,"Reading config file was successfull %d lines were read",linenumber);
	logMessage(errbuff);
	} //end if(pFILE!=NULL)
	else
	{
		sprintf(errbuff,"opening Configfile: %s failed!",filename);
		logError(errbuff);
		return 1;
	}

	return 0;
}


int triggerConfig(sParameterStruct *sSO2Parameters)
{
	/* in my opinion this function is the most complicated shit in the whole programm ;) */

	etStat          eStat                   = PHX_OK; /* Status variable */
	etParamValue    eParamValue;
	ui32            dwTriggerPulseWidthUs   = sSO2Parameters->dTriggerPulseWidth;
	tHandle			hCamera = sSO2Parameters->hCamera;
	
	/* Enable the CCIO port as an output.
	* This call is benign on Camera Link boards as CCIO is, by definition,
	* an output only port. CCIO := Camera Control Input Output
	*/
	eParamValue = PHX_ENABLE;
	eStat = PHX_ParameterSet( hCamera, PHX_IO_CCIO_OUT, (void *) &eParamValue );
	if ( PHX_OK != eStat )
	{
		logError("opening the CCIO port failed");
		sSO2Parameters->eStat = eStat;
		return eStat;
	}

	/* Initialise the CCIO bit 1 pin as a negative going output driven from the exposure
	* timer 1 with a pre-defined pulse width
	*/
	eParamValue = (etParamValue)(PHX_IO_METHOD_BIT_TIMER_NEG | 1);
	eStat = PHX_ParameterSet( hCamera, PHX_IO_CCIO, (void *) &eParamValue );
	if ( PHX_OK != eStat )
	{
		logError("Initialsing the CCIO bit 1 pin as a negative output for exposure timing failed");
		sSO2Parameters->eStat = eStat;
		return eStat;
	}

	/* the trigger pulse width is define in the config file. min: 1 us */
	eParamValue = (etParamValue)dwTriggerPulseWidthUs;
	eStat = PHX_ParameterSet( hCamera, PHX_IO_TIMER_1_PERIOD, (void *) &eParamValue );
	if ( PHX_OK != eStat )
	{
		logError("setting the trigger pulse width failed");
		sSO2Parameters->eStat = eStat;
		return eStat;
	}

	/* set the framegrabber that exposure is started by software trigger */
	eParamValue = PHX_EXPTRIG_SWTRIG;
	eStat = PHX_ParameterSet( hCamera, PHX_EXPTRIG_SRC, (void *) &eParamValue );
	if ( PHX_OK != eStat )
	{
		logError("setting the exposure start to software trigger failed");
		sSO2Parameters->eStat = eStat;
		return eStat;
	}

	logMessage("trigger configuration was successfull");
	return eStat;
}


int defaultConfig(sParameterStruct *sSO2Parameters)
{
	etStat          eStat = PHX_OK;   /* Status variable */
	etParamValue    eParamValue;
	tHandle			hCamera = sSO2Parameters->hCamera;
	
	/* Camera Communication Settings ( standard serial...)
	 * These settings are 9600 Baud, 8 data, no parity,
	 * 1 stop with no flow control */

	eParamValue = PHX_COMMS_DATA_8;
	eStat = PHX_ParameterSet( hCamera, PHX_COMMS_DATA, &eParamValue );

	eParamValue = PHX_COMMS_STOP_1;
	eStat = PHX_ParameterSet(hCamera, PHX_COMMS_STOP, &eParamValue );

	eParamValue = PHX_COMMS_PARITY_NONE;
	eStat = PHX_ParameterSet(hCamera, PHX_COMMS_PARITY, &eParamValue );

	eParamValue = (etParamValue)9600;
	eStat = PHX_ParameterSet(hCamera, PHX_COMMS_SPEED, &eParamValue );

	eParamValue = PHX_COMMS_FLOW_NONE;
	eStat = PHX_ParameterSet(hCamera, (etParam)(PHX_COMMS_FLOW|PHX_CACHE_FLUSH), &eParamValue );

	if ( PHX_OK != eStat )
	{
		logError("configuration of serial connection to camera failed");
		sSO2Parameters->eStat = eStat;
		return eStat;
	}

	/* Image format settings
	 * 1344x1024, 12-Bit Source, 12-Bit Output,
	 * make use of the Data valid signal providetd
	 * by the CameraLink Camera */


	/* some cameras output a 'Data Enable' control signal to indicate when the data is valid.
	 * this option makes the framegrabber software to use such control signal */
	eParamValue = PHX_ENABLE;
	eStat = PHX_ParameterSet(hCamera, PHX_CAM_DATA_VALID, &eParamValue );
	if ( PHX_OK != eStat )
	{
		logError("make the framegrabber use the -data valid- signal failed");
		sSO2Parameters->eStat = eStat;
		return eStat;
	}

	eParamValue = (etParamValue)12;
	eStat = PHX_ParameterSet(hCamera, PHX_CAM_SRC_DEPTH, &eParamValue );
	if ( PHX_OK != eStat )
	{
		logError("Setting the image depth recieved from camera to 12-bit failed");
		sSO2Parameters->eStat = eStat;
		return eStat;
	}
	else logMessage("Image depth recieved from camera is set to 12-bit");

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
	eStat = PHX_ParameterSet(hCamera, PHX_DST_FORMAT, &eParamValue );
	if ( PHX_OK != eStat )
	{
		logError("Setting the image file depth to 12-bit failed");
		sSO2Parameters->eStat = eStat;
		return eStat;
	}
	else logMessage("Image file depth is set to 12-bit");

	/* Enable FIFO Overflow events */
	eParamValue = PHX_INTRPT_FIFO_OVERFLOW;
	eStat = PHX_ParameterSet( hCamera, PHX_INTRPT_SET, &eParamValue );
	if ( PHX_OK != eStat )
	{
		logError("Set interpretation of FIFO overflows failed");
		sSO2Parameters->eStat = eStat;
		return eStat;
	}
	else logMessage("Right interpretation of FIFO overflows is set");


	/* Setup our own event context */
	eStat = PHX_ParameterSet( hCamera, PHX_EVENT_CONTEXT, (void *) sSO2Parameters );
	if ( PHX_OK != eStat ){
		logError("Setup the control flags structure failed");
		sSO2Parameters->eStat = eStat;
		return eStat;
	}

	logMessage("configuration of the Framegrabber was successfull");
	sSO2Parameters->eStat = eStat;
	return 0;
}


int defaultCameraConfig(sParameterStruct *sSO2Parameters)
{
	etStat			eStat = PHX_OK;
	tHandle			hCamera = sSO2Parameters->hCamera;
	
	// initialise default vaulues
	eStat = sendMessage(hCamera, "INI");
	if ( PHX_OK != eStat )
	{
		sSO2Parameters->eStat = eStat;
		logError("sending INI to camera was unsuccessfull");
		return eStat;
	}

	// freerunning or external control mode:
	// N freerun mode, E external
	eStat = sendMessage(hCamera, "AMD N");
	if ( PHX_OK != eStat )
	{
		sSO2Parameters->eStat = eStat;
		logError("sending AMD N to camera was unsuccessfull");
		return eStat;
	}

	// scanning mode: N Normal, S superpixel
	eStat = sendMessage(hCamera, "SMD N");
	if ( PHX_OK != eStat )
	{
		sSO2Parameters->eStat = eStat;
		logError("sending SMD N to camera was unsuccessfull");
		return eStat;
	}

	//horizontal pixel output: M = 1344
	eStat = sendMessage(hCamera, "SHA M");
	if ( PHX_OK != eStat )
	{
		sSO2Parameters->eStat = eStat;
		logError("sending SHA M to camera was unsuccessfull");
		return eStat;
	}

	//contrast gain: high
	eStat = sendMessage(hCamera, "CEG H");
	if ( PHX_OK != eStat )
	{
		sSO2Parameters->eStat = eStat;
		logError("sending CEG H to camera was unsuccessfull");
		return eStat;
	}

	logMessage("configuration of camera was successfull");
	sSO2Parameters->eStat = eStat;
	return eStat;
}


int sendMessage(tHandle hCamera, char * inputBuffer)
{
	/* error handling is inverted in this function. cant remember why */
	etStat eStat = PHX_OK;
	etParamValue eParamValue;
	int		i, sleepCycleCounter = 1;
	int 	timeout = 500;
	ui32	InputLineBufferLength = 0;
	ui32	OutputLineBufferLength = 0;
	char	inputLineBuffer[_PHX_LINE_SIZE];
	char	outputLineBuffer[_PHX_LINE_SIZE];
	char	*pInputLineBuffer;
	pInputLineBuffer  = inputLineBuffer;
	sprintf(pInputLineBuffer,"%s\r",inputBuffer);

	/* 3 tries before the sending of of the message is considered failed */
	for(i=0;i<3;i++)
	{
		/* Transmit the serial data to the camera*/
		InputLineBufferLength = strlen( inputLineBuffer );
		eParamValue = (etParamValue)InputLineBufferLength;
		eStat = PHX_CommsTransmit( hCamera, (ui8*) inputLineBuffer, (ui32*) &eParamValue, timeout );
		if ( PHX_OK == eStat )
		{
			/* if transmitting was successful program waits for incoming messages
			 * 0.5s is a good compromise between speed and reliability
			 */
 			do
			{
			_PHX_SleepMs(timeout/50);
			sleepCycleCounter++;
			/* Check how many characters are waiting to be read */
			eStat = PHX_ParameterGet( hCamera, PHX_COMMS_INCOMING, &OutputLineBufferLength );
			/* create a timeout signal if 0.5s are over and no data was recieved */
			if(sleepCycleCounter > (timeout/10)) eStat = PHX_WARNING_TIMEOUT;

			} while ( 0 == OutputLineBufferLength && PHX_OK == eStat);

			if(PHX_OK == eStat)
			{
				/* if data is recieved, download the data */
				eParamValue = (etParamValue)OutputLineBufferLength;
				eStat = PHX_CommsReceive( hCamera, (ui8*) outputLineBuffer, (ui32*) &eParamValue, timeout );
				if(PHX_OK == eStat)
				{
					if(strcmp(inputLineBuffer,outputLineBuffer) != 0)
					{
						/* if cameras answer equals input string, exit successfull */
						//printf("DEBUG: send message: %s was successful\n",inputLineBuffer);
						return 0; /* here return of SUCCESS */
					}
					else logError("String send and string receive were not equal");
				} // if(PHX_OK == eStat)
				else logError("nothing was received from camera");
			} // if(PHX_OK == eStat)
			else logError("nothing was received from camera");
		} // if ( PHX_OK == eStat )
		else logError("PHX_CommsTransmit(...) failed");

	} // for(i=0;i<3;i++)
	logError("sending message failed 3 times");
	return eStat; /* here return if something FAILED */
}

