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
	etStat eStat = PHX_OK;
	int exposureTime = (int)sSO2Parameters->dExposureTime;
	tHandle hCamera = sSO2Parameters->hCamera;
	stImageBuff stBuffer;
	int timeSwitch=0;
	memset( &stBuffer, 0, sizeof(stImageBuff ));
	if(sSO2Parameters->dFixTime != 0)
	{
		/* fix exposure time */

		fixExposureTime(sSO2Parameters);
	}
	else
	{
		/* variable exposure time */
		
		/* SOME PROBLEMS WITH NOT FINDING THE RIGHT FRAMEBLANKING MODE 
		 * it switches between 1 and 2 becaus one is to dark and the 
		 * other one is to bright
		 */
		

		/* Preset the Camera with a medium exposuretime
		 * NMD: N, S, F
		 * N = Normal, S = Electronic Shutter, F = Frameblanking
		 * Speed:
		 * Frameblanking FBL: 1 - 12
		 * Electronic Shutter SHT = 1 - 1055
		 * FBL > SHT 
		 * conversion to seconds see Manual
		 */
		eStat = sendMessage(hCamera,"NMD S");
			if (PHX_OK != eStat ) goto Error;
		eStat = sendMessage(hCamera, "SHT 1055");
			if (PHX_OK != eStat ) goto Error;
		
		/* Acquire first buffer to decide between FBL or SHT */
		getOneBuffer(sSO2Parameters, &stBuffer,sControlFlags);
		
		/* calculate histogram to test for over or unter exposition */
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
					break;
		}
	}
	Error:
	sSO2Parameters->eStat = eStat;
	return 0;
}

int fixExposureTime(sParameterStruct *sSO2Parameters)
{
	int exposureTime = (int)sSO2Parameters->dExposureTime;
	tHandle hCamera = sSO2Parameters->hCamera;
	etStat eStat = PHX_OK;
	int		shutterSpeed = 1; /* in case something went wrong this value is accepted by both modi */
	char message[9];

	if ( exposureTime < 2.4 || exposureTime > 1004400)
	{
		printf("Exposure time is out of range \n2.4us < Exposure Time > 1004.4ms\n");
		eStat = PHX_ERROR_OUT_OF_RANGE;
		goto Error;
	}

	if (exposureTime <= 83560)
	{	//===========ELECTRONIC=SHUTTER==========
		shutterSpeed = roundToInt(((exposureTime - 2.4)/79.275)+1);
		sprintf(message,"SHT %d",shutterSpeed);

		// N normal, S Shutter, F frameblanking
		eStat = sendMessage(hCamera, "NMD S");
		if ( PHX_OK != eStat ) goto Error;
		
		// Shutter speed, 1 - 1055
		eStat = sendMessage(hCamera, message);
		if ( PHX_OK != eStat ) goto Error;
		exposureTime = (int)(2.4 + (shutterSpeed-1) * 79.275);
		printf("Elektronic shutter mode\nExposure time = %f us\n",exposureTime);
	}
	else
	{	//===========FRAME=BLANKING==========
		shutterSpeed = roundToInt(exposureTime/83700);
		sprintf(message,"FBL %d",shutterSpeed);
		

		// N normal, S Shutter, F frameblanking
		eStat = sendMessage(hCamera, "NMD F");
		if ( PHX_OK != eStat ) goto Error;
		
		// Shutter speed, 1 - 12
		 eStat = sendMessage(hCamera, message);
		if ( PHX_OK != eStat ) goto Error;

		exposureTime = shutterSpeed * 83700;
		printf("Frame blanking mode\nExposure time = %f us\n",exposureTime);
		
	}

	Error:
	sSO2Parameters->eStat = eStat;
	return 0;
}
int getOneBuffer(sParameterStruct *sSO2Parameters, stImageBuff	*stBuffer, flagStruct *sControlFlags)
{
	etStat eStat			= PHX_OK;   /* Status variable */
	tHandle hCamera			= sSO2Parameters->hCamera;
	
	/* Initiate a software trigger of the exposure control signal
	 * see triggerConfig()
	 */
	PHX_Acquire( hCamera, PHX_EXPOSE, NULL );
	
	/* start capture, hand over callback function*/
	eStat = PHX_Acquire( hCamera, PHX_START, (void*) callbackFunction );
		if ( PHX_OK != eStat ) goto Error;
	
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
		if ( PHX_OK != eStat ) goto Error;
	
	/* stopping the acquisition */
	PHX_Acquire( hCamera, PHX_ABORT, NULL );
	
	Error:
	sSO2Parameters->eStat = eStat;
	return 0;
}

int evalHist(stImageBuff *stBuffer, sParameterStruct *sSO2Parameters, int *timeSwitch)
{
	/* bufferlength = 1344 x 1024 number of pixels 
	 * Image date is stored in 12-bit data within 16-bit data
	 * datatyp 'short' in Visual Studio v6.0 represents 16 bit in memory
	 * this might be different on different compilers.
	 * IF POSSIBLE CHANGE THIS TO SOMETHING LESS DIRTY
	 */
	int i;
	int bufferlength = sSO2Parameters->dBufferlength;
	short temp=0; 
	short * shortBuffer;
	int summe = 0;
	int histogram[4096]={0};
	int percentage = sSO2Parameters->dHistPercentage;
	int intervalMin = sSO2Parameters->dHistMinInterval; 
	/* While debugging 2 files are created at this point
	 * fwriteCount is 2 x bufferlength because
	 * we save the image date byte per byte
	 */
	int fwriteCount= bufferlength*2;//, fwriteReturn;
//	FILE *output, *bufferDump;
	
	/* opening the debug files */
//	output = fopen("histogram.txt","w");
//	bufferDump = fopen("histoImage.raw","wb");
	
	
	/* shortBuffer gets the address of the image data assigned 
	 * since shortBuffer is of datatyp 'short' 
	 * shortbuffer++ will set the pointer 16 bits forward
	 */
	 shortBuffer=stBuffer->pvAddress;
	
	/* scanning the whole buffer and creating a histogram */
	 for(i=0;i<bufferlength;i++)
	{
		temp=*shortBuffer;
		histogram[temp]++;
		shortBuffer++;
	}
	 
	 /* For debugging: save histogram to file */
/*	for(i=0;i<4096;i++)
	{
		fprintf(output,"%04d %06d\n",i,histogram[i]);
	}
*/	
	/* check if image is over or underexposed */
	for(i=0;i<intervalMin;i++)
	{
		summe = summe + histogram[i];
	}
	*timeSwitch = 0;
	if(summe > (bufferlength*percentage/100)) 
	{
		//printf("Underexposed!\nsumme = %d \n",summe);
		*timeSwitch = 1;
	}
	
	if(histogram[4095] > (bufferlength*percentage/100)) 
	{
		//printf("Overexposed!\nHistogram[4095]= %d\n",histogram[4095]);
		if(*timeSwitch == 1)
		{
			/* If timeSwitch was already set to the picture is underexposed and 
			 * overexposed therefor the timeSwitch is set to 3
			 */
			//printf("to much Contrast\n");
			*timeSwitch = 3;
		}
		else
		{
			/* Image is only overexposed */
			*timeSwitch = 2;
		}
	}

	 
	 /* for debugging the captured image is save to harddrive
	  * to save it we have to reset the pointer. The easiest way
	  * to do so is to assign it again to the original buffer
	  */
	  	shortBuffer=stBuffer->pvAddress;
	//fwriteCount = 2752512; 
//	fwriteReturn = fwrite(shortBuffer,1,fwriteCount,bufferDump);
//	fclose(bufferDump);
//	fclose(output);
	return 0;
}



int setFrameBlanking(sParameterStruct *sSO2Parameters, flagStruct *sControlFlags)
{
	etStat eStat = PHX_OK;
	tHandle hCamera	= sSO2Parameters->hCamera;
	stImageBuff stBuffer;
	char message[9];
	/* Just a Value != 0,1,2,3 */
	int timeSwitch  = 4;
	/* FBL max = 12 12/2=6 */
	double FBvalue = 6., divisor = FBvalue;
	
	/* Switching to Frameblanking mode */
	eStat = sendMessage(hCamera,"NMD F");
		if ( PHX_OK != eStat ) goto Error;
	
	
	
	while(timeSwitch != 0 && timeSwitch != 3)
	{
		if(FBvalue > 12 || FBvalue < 1) break;
		printf("timeSwitch = %d \n",timeSwitch);
		printf("%f\n",FBvalue);
		divisor = divisor / 2;
		divisor = (double)roundToInt(divisor);
		sprintf(message,"FBL %d",roundToInt(FBvalue));
		eStat = sendMessage(hCamera,message);
			if (PHX_OK != eStat ) goto Error;
		
		/* Acquire first buffer to decide between FBL or SHT */
		getOneBuffer(sSO2Parameters, &stBuffer,sControlFlags);
		
		/* calculate histogram to test for over or unter exposition */
		evalHist(&stBuffer, sSO2Parameters, &timeSwitch);
		
		

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
	}
	/* save the exposure time to control Struct */
	sSO2Parameters->dExposureTime = FBvalue * 0.0837;
	Error:
	sSO2Parameters->eStat = eStat;
	return 0;
}

int setElektronicShutter(sParameterStruct *sSO2Parameters, flagStruct *sControlFlags)
{
	etStat eStat = PHX_OK;
	tHandle hCamera	= sSO2Parameters->hCamera;
	stImageBuff stBuffer;
	char message[9];
	/* Just a Value !=0,1,2,3 */
	int timeSwitch  = 4;
	/* SHT max = 1055 1055/2=527.5 -> 528 */ 
	double SHTvalue = 528., divisor = SHTvalue;
	
	/* Switching to Electronic Shutter mode */
	eStat = sendMessage(hCamera,"NMD S");
		if ( PHX_OK != eStat ) goto Error;
	
	
	
	while(timeSwitch != 0 && timeSwitch != 3)
	{
		if(SHTvalue > 1055 || SHTvalue < 1) break;
		
		divisor = divisor / 2.;
		sprintf(message,"SHT %d",roundToInt(SHTvalue));
		printf("%s\n",message);
		eStat = sendMessage(hCamera,message);
			if (PHX_OK != eStat ) goto Error;
		
		/* Acquire first buffer to decide between FBL or SHT */
		getOneBuffer(sSO2Parameters, &stBuffer,sControlFlags);
		
		/* calculate histogram to test for over or unter exposition */
		evalHist(&stBuffer, sSO2Parameters, &timeSwitch);
		
		
		
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
	}
	/* save the exposure time to control Struct */
	/* GENAUIGKEIT VON FLOAT!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
	sSO2Parameters->dExposureTime = 0.0000124+(SHTvalue-1)*0.000079275;
	Error:
	sSO2Parameters->eStat = eStat;
	return 0;
}

int roundToInt(double value)
{
	/* This function is necessary because round()
	 * seems not implemented in this version of "math.h"
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
