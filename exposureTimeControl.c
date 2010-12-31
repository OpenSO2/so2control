/* Function for the automatic control of the exposure time 
 * of the Hamamatsu CCD-Camera C8484-16
 * it is based on the Active Silicon Framegrabber SDK 
 *  
 */
#include<stdio.h>

#include"exposureTimeControl.h"
#include"configurations.h"
#include"imageCreation.h"

int setExposureTime( sParameterStruct *sSO2Parameters, flagStruct *sControlFlags)
{
	etStat			eStat 			= PHX_OK; /* Phoenix status variable */
	//int				status; /* Status varable for several return values */
	int				exposureTime	= (int)sSO2Parameters->dExposureTime; /* exposure time in parameter structure */
	int				timeSwitch		= 0; /* Integer switch to switch between exposure modi */
	tHandle			hCamera 		= sSO2Parameters->hCamera; /* hardware handle for camera */
	stImageBuff		stBuffer; /* Buffer where the Framegrabber stores the image */
	
	/* pre-set the buffer with zeros */
	memset( &stBuffer, 0, sizeof(stImageBuff ));
	if(sSO2Parameters->dFixTime != 0)
	{
		/* Check if exposure time is declared fix in the config file if so set it.*/
		eStat = fixExposureTime(sSO2Parameters);
		if (eStat != 0)
		{
			printf("setting fixed exposure time failed. aborting...\n");
			return eStat;
		}
	}
	else
	{
		/* SOME PROBLEMS WITH NOT FINDING THE RIGHT FRAMEBLANKING MODE 
		 * it switches between 1 and 2 because one is to dark and the 
		 * other one is to bright
		 */
		
		/* NMD: N, S, F
		 * N = Normal, S = Electronic Shutter, F = Frameblanking
		 * Speed:
		 * Frameblanking FBL: 1 - 12
		 * Electronic Shutter SHT = 1 - 1055
		 * FBL > SHT 
		 * for conversion to milliseconds see camera manual
		 */
		 
		 /* Pre-set the Camera with a medium exposure time */
		eStat = sendMessage(hCamera,"NMD S");
			if (PHX_OK != eStat )
			{
				sSO2Parameters->eStat = eStat;
				return eStat;
			}

		eStat = sendMessage(hCamera, "SHT 1055");
			if (PHX_OK != eStat )
			{
				sSO2Parameters->eStat = eStat;
				return eStat;
			}
		
		/* Acquire first buffer to decide between FBL or SHT */
		eStat = getOneBuffer(sSO2Parameters, &stBuffer,sControlFlags);
		if (eStat != 0)
		{
			sSO2Parameters->eStat = eStat;
			return eStat;
		}
		/* calculate histogram to test for over or under exposition */
		evalHist(&stBuffer, sSO2Parameters, &timeSwitch);

		switch(timeSwitch)
		{
			case 0 : printf("starting electronic shutter mode\nExposuretime is set\n");
					sSO2Parameters->dExposureTime = 0.0000124+(1055-1)*0.000079275;
					break;
			
			case 1: printf("starting frameblanking mode\n");
					setFrameBlanking(sSO2Parameters, sControlFlags);
					break;
			
			case 2: printf("starting Electronic Shutter mode\n");
					setElektronicShutter(sSO2Parameters, sControlFlags);
					break;
			
			case 3: printf("Exposure time is too high and too low.\nmaybe contrast is too high\n");
					printf("Frameblanking mode is set\nExposuretime is not changed\n");
					break;
			
			default:printf("Oh oh something weird just happened\n");
					printf("%d is under no circumstances a valid value for this Switch\n",timeSwitch);
					return 1;
		}
	}
	Error:
	sSO2Parameters->eStat = eStat;
	return 0;
}

int fixExposureTime(sParameterStruct *sSO2Parameters)
{
	int			exposureTime	= (int)sSO2Parameters->dExposureTime; /* exposure time in parameter structure */
	tHandle		hCamera			= sSO2Parameters->hCamera; /* hardware handle for camera */
	etStat		eStat			= PHX_OK; /* Phoenix status variable */
	int			shutterSpeed	= 1; /* in case something went wrong this value is accepted by both modi */
	char message[9];

	/* before doing anything check if exposure time is within the limits */
	if ( exposureTime < 2.4 || exposureTime > 1004400)
	{
		printf("Exposure time is out of range \n2.4us < Exposure Time > 1004.4ms\n");
		eStat = PHX_ERROR_OUT_OF_RANGE;
		return eStat;
	}

	if (exposureTime <= 83560)
	{	//===========ELECTRONIC=SHUTTER==========
	
		shutterSpeed = roundToInt(((exposureTime - 2.4)/79.275)+1);
		sprintf(message,"SHT %d",shutterSpeed);

		// N normal, S Shutter, F frameblanking
		eStat = sendMessage(hCamera, "NMD S");
		if ( PHX_OK != eStat )
		{
			printf("communication with camera failed. aborting... \n");
			return eStat;
		}
		
		
		// Shutter speed, 1 - 1055
		eStat = sendMessage(hCamera, message);
		if ( PHX_OK != eStat )
		{
			printf("communication with camera failed. aborting... \n");
			return eStat;
		}
		
		exposureTime = (int)(2.4 + (shutterSpeed-1) * 79.275);
		printf("Elektronic shutter mode\nExposure time = %f us\n",exposureTime);
	}
	else
	{	//===========FRAME=BLANKING==========
	
		shutterSpeed = roundToInt(exposureTime/83700);
		sprintf(message,"FBL %d",shutterSpeed);
		

		// N normal, S Shutter, F frameblanking
		eStat = sendMessage(hCamera, "NMD F");
		if ( PHX_OK != eStat )
		{
			printf("communication with camera failed. aborting... \n");
			return eStat;
		}
		
		// Shutter speed, 1 - 12
		 eStat = sendMessage(hCamera, message);
		if ( PHX_OK != eStat )
		{
			printf("communication with camera failed. aborting... \n");
			return eStat;
		}

		exposureTime = shutterSpeed * 83700;
		printf("Frame blanking mode\nExposure time = %f us\n",exposureTime);
		
	}

	sSO2Parameters->eStat = eStat;
	return eStat;
}

int getOneBuffer(sParameterStruct *sSO2Parameters, stImageBuff	*stBuffer, flagStruct *sControlFlags)
{
	/*  this function is very similar to startAquisition( ... ) */
	etStat		eStat			= PHX_OK; /* Status variable */
	tHandle		hCamera			= sSO2Parameters->hCamera; /* hardware handle for camera */
	int 		startErrCount	= 0; /* counting how often the start of capture process failed */
	
	/* Initiate a software trigger of the exposure control signal
	 * see triggerConfig()
	 */
	PHX_Acquire( hCamera, PHX_EXPOSE, NULL );
	
	do
	{
		/* start capture, hand over callback function*/
		eStat = PHX_Acquire( hCamera, PHX_START, (void*) callbackFunction );
		if ( PHX_OK == eStat )
		{
			/* if starting the capture was successful reset error counter to zero */
			startErrCount = 0;
			/* Wait for a user defined period between each camera trigger call*/
			_PHX_SleepMs( sSO2Parameters->dInterFrameDelay );
			
			/* Wait here until either:
			 * (a) The user aborts the wait by pressing a key in the console window
			 *     DEACTIVATE THIS LATER
			 * (b) The BufferReady flag is set indicating that the image is complete
			 * (c) The FIFO overflow flag is set indicating that the image is corrupt.
			 * Keep calling the sleep function to avoid burning CPU cycles
			 */
			while ( !sControlFlags->fBufferReady && !sControlFlags->fFifoOverFlow && !PhxCommonKbHit() ) 
			{
				_PHX_SleepMs(10);
			}
			
			/* if BufferReady flag is set, reset it for next image */
			sControlFlags->fBufferReady = FALSE;
			
			/* download the buffer and place it in 'stBuffer' */
			eStat = PHX_Acquire( hCamera, PHX_BUFFER_GET, stBuffer );
				if ( PHX_OK != eStat )
				{
					printf("acquisition of one buffer failed\n");
					/* stopping the acquisition */
					PHX_Acquire( hCamera, PHX_ABORT, NULL );
					return eStat;
				}
			
			/* stopping the acquisition */
			PHX_Acquire( hCamera, PHX_ABORT, NULL );
		} // if ( PHX_OK == eStat )
		else
		{
			/* if starting the capture failed more than 3 times program stops */
			startErrCount++;
			if(startErrCount >= 3)
			{
				printf("starting the acquisition failed 3 times in a row. Aborting...");
				PHX_Acquire( hCamera, PHX_ABORT, NULL );
				sSO2Parameters->eStat = eStat;
				return eStat;
			}
			PHX_Acquire( hCamera, PHX_ABORT, NULL );
		} // else
	/* loops only if something went wrong */ 
	} while (startErrCount != 0);
	sSO2Parameters->eStat = eStat;
	return eStat;
}

int evalHist(stImageBuff *stBuffer, sParameterStruct *sSO2Parameters, int *timeSwitch)
{
	/* bufferlength = 1344 x 1024 number of pixels 
	 * Image date is stored in 12-bit data within 16-bit data
	 * datatyp 'short' in Visual Studio v6.0 represents 16 bit in memory
	 * this might be different on different compilers.
	 * IF POSSIBLE CHANGE THIS TO SOMETHING LESS DIRTY
	 */

	int		bufferlength 	= sSO2Parameters->dBufferlength;
	int		percentage		= sSO2Parameters->dHistPercentage;
	int		intervalMin		= sSO2Parameters->dHistMinInterval; 
	int		histogram[4096]	={0};
	int		summe 			= 0;
	int		i;
	short	temp			=0; 
	short	*shortBuffer;
	
	/* shortBuffer gets the address of the image data assigned 
	 * since shortBuffer is of datatyp 'short' 
	 * shortbuffer++ will set the pointer 16 bits forward
	 */
	 shortBuffer = stBuffer->pvAddress;
	
	/* scanning the whole buffer and creating a histogram */
	 for(i=0;i<bufferlength;i++)
	{
		temp = *shortBuffer;
		histogram[temp]++;
		shortBuffer++;
	}

	/* sum over a through config file given interval to check if image is underexposed */
	for(i=0;i<intervalMin;i++)
	{
		summe = summe + histogram[i];
	}
	
	/* pre-set the switch to 0 if image is neither over or under exposed it remains 0 */
	*timeSwitch = 0;
	
	/* check if the image is underexposed by testing if the sum of al values in a given interval
	 * is greater than a given confidence value */
	if(summe > (bufferlength*percentage/100)) 
	{
		*timeSwitch = 1;
	}
	/* check if the image is overexposed by testing how often the brightest pixel appears in the image */
	if(histogram[4095] > (bufferlength*percentage/100)) 
	{
		if(*timeSwitch == 1)
		{
			/* If timeSwitch was already set to 1 the picture is underexposed and 
			 * overexposed therefore the timeSwitch is set to 3
			 */
			*timeSwitch = 3;
		}
		else
		{
			/* Image is only overexposed */
			*timeSwitch = 2;
		}
	}
	return 0;
}



int setFrameBlanking(sParameterStruct *sSO2Parameters, flagStruct *sControlFlags)
{
	etStat			eStat		= PHX_OK; /* Phoenix status variable */
	tHandle			hCamera		= sSO2Parameters->hCamera; /* hardware handle for camera */
	stImageBuff		stBuffer; /* Buffer where the Framegrabber stores the image */
	char			message[9]; /* Message buffer for communication with camera */
	int				timeSwitch	= 1; /* Integer switch to switch between exposure modi */
	double			FBvalue		= 6; /* pre-set value 12/2=6 */
	double			divisor		= FBvalue; /* actual value send to camera */
	int 			switchMemory1 = 2; /* we need 2 memories because we */
	int 			switchMemory2 = 3; /* need to store the information over 2 loop-cycles */
	
	/* Switching to Frameblanking mode */
	eStat = sendMessage(hCamera,"NMD F");
	if ( PHX_OK != eStat )
	{
		sSO2Parameters->eStat = eStat;
		return eStat;
	}
	
	
	
	while(timeSwitch != 0 && timeSwitch != 3)
	{
		if(FBvalue > 12 || FBvalue < 1) break;
		
		divisor = divisor / 2;
		/* it seems we are doing this twice not sure why */
		divisor = (double)roundToInt(divisor);
		
		sprintf(message,"FBL %d",roundToInt(FBvalue));
		eStat = sendMessage(hCamera,message);
		if (PHX_OK != eStat )
		{
			sSO2Parameters->eStat = eStat;
			return eStat;
		}
		
		/* Acquire first buffer to decide between FBL or SHT */
		eStat = getOneBuffer(sSO2Parameters, &stBuffer,sControlFlags);
		if ( PHX_OK != eStat )
		{
			sSO2Parameters->eStat = eStat;
			return eStat;
		}
		
		/* calculate histogram to test for over or under exposition */
		evalHist(&stBuffer, sSO2Parameters, &timeSwitch);

		/* a little bit hacky but it works */
		if (switchMemory2 == timeSwitch)
		{
			printf("we are caught in a loop between %d and %d. Breaking out.\n", roundToInt(switchMemory1), roundToInt(switchMemory2));
			timeSwitch = 0;
		}
		
		switch(timeSwitch)
		{
		case 0 : printf("Exposuretime is set!\n"); 
				 break;
		
		case 1: printf("Exposure time is set up\n");
				FBvalue = FBvalue + divisor;

				break;
		
		case 2: printf("Exposure time is set down\n");
				FBvalue = FBvalue - divisor;
				break;
		
		case 3: printf("Exposure time is too high and too low.\nmaybe contrast is too high\n");
				break;
		
		default:printf("Oh oh something weird just happened\n");
				printf("%d is under no circumstances a valid value for this Switch\n",timeSwitch);
				break;
		}
		switchMemory2 = switchMemory1;
		switchMemory1 = timeSwitch;
	
	}
	/* save the exposure time in [ms] to control Struct */
	sSO2Parameters->dExposureTime = FBvalue * 0.0837;

	sSO2Parameters->eStat = eStat;
	return eStat;
}

int setElektronicShutter(sParameterStruct *sSO2Parameters, flagStruct *sControlFlags)
{
	etStat			eStat		= PHX_OK; /* Phoenix status variable */
	tHandle			hCamera		= sSO2Parameters->hCamera; /* hardware handle for camera */
	stImageBuff		stBuffer; /* Buffer where the Framegrabber stores the image */
	char			message[9]; /* Message buffer for communication with camera */
	int				timeSwitch 	= 1; /* Integer switch to switch between exposure modi */
	double			SHTvalue	= 528; /* SHT max = 1055 1055/2=527.5 -> 528 */ 
	double			divisor		= SHTvalue; /* actual value send to camera */
	int 			switchMemory1 = 2; /* we need 2 memories because we */
	int 			switchMemory2 = 3; /* need to store the information over 2 loop-cycles */
	
	/* Switching to Electronic Shutter mode */
	eStat = sendMessage(hCamera,"NMD S");
	if ( PHX_OK != eStat )
	{
		sSO2Parameters->eStat = eStat;
		return eStat;
	}
	
	
	
	while(timeSwitch != 0 && timeSwitch != 3)
	{
		if(SHTvalue > 1055 || SHTvalue < 1) break;
		
		divisor = divisor / 2.;
		
		sprintf(message,"SHT %d",roundToInt(SHTvalue));
		eStat = sendMessage(hCamera,message);
		if (PHX_OK != eStat )
		{
			sSO2Parameters->eStat = eStat;
			return eStat;
		}
		
		/* Acquire first buffer to decide between FBL or SHT */
		eStat = getOneBuffer(sSO2Parameters, &stBuffer,sControlFlags);
		if ( PHX_OK != eStat )
		{
			sSO2Parameters->eStat = eStat;
			return eStat;
		}
		
		/* calculate histogram to test for over or unter exposition */
		evalHist(&stBuffer, sSO2Parameters, &timeSwitch);
		
		/* a little bit hacky but it works */
		if (switchMemory2 == timeSwitch)
		{
			printf("we are caught in a loop between %d and %d. Breaking out.\n", roundToInt(switchMemory1), roundToInt(switchMemory2));
			timeSwitch = 0;
		}
		
		switch(timeSwitch)
		{
		case 0 : break;
		
		case 1: printf("Exposure time is set up\n");
				SHTvalue = SHTvalue + divisor;
				break;
		
		case 2: printf("Exposure time is set down\n");
				SHTvalue = SHTvalue - divisor;
				break;
		
		case 3: printf("Exposure time is too high and too low.\nmaybe contrast is too high\n");
				break;
		
		default:printf("Oh oh something weird just happened\n");
				printf("%d is under no circumstances a valid value for this Switch\n",timeSwitch);
				break;
		}
		switchMemory2 = switchMemory1;
		switchMemory1 = timeSwitch;
	}
	
	/* save the exposure time to control Struct */
	sSO2Parameters->dExposureTime = 0.0000124+(SHTvalue-1)*0.000079275;

	sSO2Parameters->eStat = eStat;
	return eStat;
}

int roundToInt(double value)
{
	/* This function is necessary because round()
	 * seems not implemented in the VC6.0 version of "math.h"
	 */
	int result;
	double temp;
	
	temp = value-floor(value);
	if (temp >= 0.5)
		result = (int)(floor(value)+1);
	else
		result = (int)(floor(value));	

	return result;
}
