#include"configurations.h"
#include"common.h"
#define MAXBUF 1024



int structInit(sParameterStruct *sSO2Parameters)
{
	sSO2Parameters->eStat = PHX_OK;
	sSO2Parameters->dTriggerPulseWidth = 15;
	sSO2Parameters->dBufferlength = 1376256;
	sSO2Parameters->dHistMinInterval = 350;
	sSO2Parameters->dHistPercentage = 5;
	sSO2Parameters->dInterFrameDelay = 10;
	return 0;
}

int configurationFunktion(sParameterStruct	*sSO2Parameters,flagStruct *sControlFlags)
{
	int status = 0;
	status = structInit(sSO2Parameters);

	sSO2Parameters->eStat = PHX_CameraConfigLoad( &sSO2Parameters->hCamera,"configurations//c8484.pcf" , (etCamConfigLoad)PHX_BOARD_AUTO | PHX_DIGITAL |  PHX_NO_RECONFIGURE | 1, &PHX_ErrHandlerDefault);

	status = defaultConfig(sSO2Parameters,sControlFlags);
	status = triggerConfig(sSO2Parameters);
	status = defaultCameraConfig(sSO2Parameters);
	/* need to create a variable for paths */
	status = readConfig("configurations//SO2Config.conf",sSO2Parameters);
	return status;
}

int readConfig(char *filename, sParameterStruct *sSO2Parameters)
{
	FILE *pFILE;
	char *lineBuf, *delimeterBuf; // *valueBuf,
	int n=1,i=0,delimIndex=0,linebreak=0, valueSize=0;
	int linenumber=1;
	lineBuf = (char*) malloc(MAXBUF);
	
	pFILE = fopen(filename,"r");

	if (pFILE!=NULL)
	{
		printf("open file -%s-\n",filename);
		while(fgets(lineBuf, MAXBUF, pFILE) != NULL)
		{
			if(lineBuf[0] != '#')
			{

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

				else if ( strstr( lineBuf, "ExposureTime") )
				{
					delimeterBuf = strstr(lineBuf,"=");
					sSO2Parameters->dExposureTime = atoi(delimeterBuf+1);
				}

				else if(strstr(lineBuf,"FileNamePrefix"))
				{
					delimeterBuf = strstr(lineBuf,"=");
					if(delimeterBuf[1] == ' ')
						sprintf(sSO2Parameters->cFileNamePrefix,"%s",delimeterBuf+2);
					else
						sprintf(sSO2Parameters->cFileNamePrefix,"%s",delimeterBuf+1);
				}				
			} //end if(lineBuf[0] != '#')
		linenumber++;
		} //end while(fgets(lineBuf, MAXBUF, pFILE) != NULL)
	fclose(pFILE);
	printf("close file -%s-\n",filename);
	} //end if(pFILE!=NULL)
	else
	{
		printf("opening Configfile: %s failed!\n",filename);
	}
	
	return 0;
}

int triggerConfig(sParameterStruct	*sSO2Parameters)
{
	etStat			eStat		= PHX_OK;	/* Status variable */
	etParamValue	eParamValue;
	tHandle hCamera = sSO2Parameters->hCamera;
	ui32 dwTriggerPulseWidthUs = sSO2Parameters->dTriggerPulseWidth;

	/* Enable the CCIO port as an output.
    * This call is benign on Camera Link boards as CCIO is, by definition,
    * an output only port
    */
   eParamValue = PHX_ENABLE;
   eStat = PHX_ParameterSet( hCamera, PHX_IO_CCIO_OUT, (void *) &eParamValue );
   if ( PHX_OK != eStat ) goto Error;


   /* Initialise the CCIO bit 1 pin as a negative going output driven from the exposure
    * timer 1 with a pre-defined pulse width
    */
   eParamValue = (etParamValue)(PHX_IO_METHOD_BIT_TIMER_NEG | 1);
   eStat = PHX_ParameterSet( hCamera, PHX_IO_CCIO, (void *) &eParamValue );
   if ( PHX_OK != eStat ) goto Error;

   //hier absturz wegen "PHX_IO_TIMER_1_PERIOD" wahrscheinlich weil verbindung zur 
   //nicht läuft siehe altes programm!!!

   eParamValue = (etParamValue)dwTriggerPulseWidthUs;
   eStat = PHX_ParameterSet( hCamera, PHX_IO_TIMER_1_PERIOD, (void *) &eParamValue );
   if ( PHX_OK != eStat ) goto Error;
   eParamValue = PHX_EXPTRIG_SWTRIG;
   eStat = PHX_ParameterSet( hCamera, PHX_EXPTRIG_SRC, (void *) &eParamValue );
   if ( PHX_OK != eStat ) goto Error;

   Error:
   sSO2Parameters->eStat = eStat;
	
	return 0;
}


int defaultConfig(sParameterStruct	*sSO2Parameters, flagStruct *sControlFlags)
{

	etStat         eStat          = PHX_OK;   /* Status variable */
    etParamValue   eParamValue;
	tHandle hCamera = sSO2Parameters->hCamera;

	/* Camera Communication Settings
	 * These settings are 9600 Baud, 8 data, no parity, 
	 * 1 stop with no flow control
	 */
	
	eParamValue = PHX_COMMS_DATA_8;
	eStat = PHX_ParameterSet( hCamera, PHX_COMMS_DATA, &eParamValue );
	if ( PHX_OK != eStat ) goto Error;

	eParamValue = PHX_COMMS_STOP_1;
	eStat = PHX_ParameterSet(hCamera, PHX_COMMS_STOP, &eParamValue );
	if ( PHX_OK != eStat ) goto Error;

	eParamValue = PHX_COMMS_PARITY_NONE;
	eStat = PHX_ParameterSet(hCamera, PHX_COMMS_PARITY, &eParamValue );
	if ( PHX_OK != eStat ) goto Error;

	eParamValue = (etParamValue)9600;
	eStat = PHX_ParameterSet(hCamera, PHX_COMMS_SPEED, &eParamValue );
	if ( PHX_OK != eStat ) goto Error;

	eParamValue = PHX_COMMS_FLOW_NONE;
	eStat = PHX_ParameterSet(hCamera, (etParam)(PHX_COMMS_FLOW|PHX_CACHE_FLUSH), &eParamValue );
	if ( PHX_OK != eStat ) goto Error;
	
	/* Image format settings
	 * 1344x1024, 12-Bit Source, 12-Bit Output,
	 * make use of the Data valid signal providetd
	 * by the CameraLink Camera
	 */

	eParamValue = PHX_ENABLE;
	eStat = PHX_ParameterSet(hCamera, PHX_CAM_DATA_VALID, &eParamValue );
	if ( PHX_OK != eStat ) goto Error;
	
	eParamValue = (etParamValue)12;
	eStat = PHX_ParameterSet(hCamera, PHX_CAM_SRC_DEPTH, &eParamValue );
	if ( PHX_OK != eStat ) goto Error;
	
	/*eParamValue= (etParamValue)1344;
	eStat = PHX_ParameterSet(hCamera, PHX_CAM_ACTIVE_XLENGTH, (etParamValue*) &eParamValue );
	if ( PHX_OK != eStat ) goto Error;

	eParamValue = (etParamValue)1024;
	eStat = PHX_ParameterSet(hCamera, PHX_CAM_ACTIVE_YLENGTH, (etParamValue*) &eParamValue );
	if ( PHX_OK != eStat ) goto Error;
	*/
	eParamValue = PHX_DST_FORMAT_Y12;
	eStat = PHX_ParameterSet(hCamera, PHX_DST_FORMAT, &eParamValue );
	if ( PHX_OK != eStat ) goto Error;

		/* Enable FIFO Overflow events */
	eParamValue = PHX_INTRPT_FIFO_OVERFLOW;
	eStat = PHX_ParameterSet( hCamera, PHX_INTRPT_SET, &eParamValue );
	if ( PHX_OK != eStat ) goto Error;

	/* Setup our own event context */
	eStat = PHX_ParameterSet( hCamera, PHX_EVENT_CONTEXT, (void *) sControlFlags );
	if ( PHX_OK != eStat ) goto Error;
	
Error:
	sSO2Parameters->eStat = eStat;
	return 0;
}



int defaultCameraConfig(sParameterStruct *sSO2Parameters)
{
	etStat			eStat = PHX_OK;
	tHandle			hCamera = sSO2Parameters->hCamera;
	double			exsposureTime = sSO2Parameters->dExposureTime;

	
	// initialise default vaulues
	eStat = sendMessage(hCamera, "INI");
	if ( PHX_OK != eStat )
	{
		sSO2Parameters->eStat = eStat;
		return 1;
	}
	
	// freerunning or external control mode: 
	// N freerun mode, E external
	eStat = sendMessage(hCamera, "AMD N");
	if ( PHX_OK != eStat )
	{
		sSO2Parameters->eStat = eStat;
		return 1;
	}
	
	// scanning mode: N Normal, S superpixel
	eStat = sendMessage(hCamera, "SMD N");
	if ( PHX_OK != eStat )
	{
		sSO2Parameters->eStat = eStat;
		return 1;
	}
		
	//horizontal pixel output: M = 1344
	eStat = sendMessage(hCamera, "SHA M");
	if ( PHX_OK != eStat )
	{
		sSO2Parameters->eStat = eStat;
		return 1;
	}

	//contrast gain: high
	eStat = sendMessage(hCamera, "CEG H");
	if ( PHX_OK != eStat )
	{
		sSO2Parameters->eStat = eStat;
		return 1;
	}

	sSO2Parameters->eStat = eStat;
	return 0;
}

int sendMessage(tHandle hCamera, char * inputBuffer) 
{
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
						/* if cameras answer equals input string, exit successful */
						printf("DEBUG: send message: %s was successful\n",inputLineBuffer);
						return 0;
					}
				}
			}
		}
	
	}
	/* if something went wrong exit with error value */
	printf("DEBUG: send message: %s was not successful\n",inputLineBuffer);
	return eStat;
}

