#include"configurations.h"
#include"imageCreation.h"
#include<windows.h>
#define HEADER_SIZE 64

void callbackFunction(
	tHandle		hCamera,			/* Camera handle. */
	ui32		dwInterruptMask,	/* Interrupt mask. */
	void		*pvParams			/* Pointer to user supplied context */
	)
{
	flagStruct *psControlFlags = (flagStruct*) pvParams;

	(void) hCamera;

	/* Handle the Buffer Ready event */
	if ( PHX_INTRPT_BUFFER_READY & dwInterruptMask ) {
		/* Increment the Display Buffer Ready Count */
		psControlFlags->fBufferReady = TRUE;
		psControlFlags->dBufferReadyCount++;
	}
	/* Fifo Overflow */
	if ( PHX_INTRPT_FIFO_OVERFLOW & dwInterruptMask ) {
		psControlFlags->fFifoOverFlow = TRUE;
	}

	/* Note:
	 * The callback routine may be called with more than 1 event flag set.
	 * Therefore all possible events must be handled here.
	 */
	if ( PHX_INTRPT_FRAME_END & dwInterruptMask )
	{
	}
}



int startAquisition(sParameterStruct *sSO2Parameters, flagStruct *sControlFlags)
{
	etStat 				eStat			= PHX_OK;	/* Status variable */
	tHandle 			hCamera			= sSO2Parameters->hCamera;	/* hardware handle of camera */
	int					saveErrCount	= 0;	/* counting how often saving an image failed */
	int 				startErrCount	= 0;	/* counting how often the start of capture process failed */
	
	printf("Starting acquisition...\n");
	printf("Press a key to exit\n");
	
	/* Starting the acquisition with the exposure parameter set in configurations.c and exposureTimeControl.c */
	PHX_Acquire( hCamera, PHX_EXPOSE, NULL );
	while ( !PhxCommonKbHit() && !sControlFlags->fFifoOverFlow ) 
	{
		
		/* Now start our capture, return control immediately back to program */
		eStat = PHX_Acquire( hCamera, PHX_START, (void*) callbackFunction );
		if ( PHX_OK == eStat )
		{
			/* if starting the capture was successful reset error counter to zero */
			startErrCount = 0;
			/* Wait for a user defined period between each camera trigger call*/
			_PHX_SleepMs( sSO2Parameters->dInterFrameDelay );
			
			/* Wait here until either:
			 * (a) The user aborts the wait by pressing a key in the console window
			 * (b) The BufferReady event occurs indicating that the image is complete
			 * (c) The FIFO overflow event occurs indicating that the image is corrupt.
			 * Keep calling the sleep function to avoid burning CPU cycles */
			while ( !sControlFlags->fBufferReady && !sControlFlags->fFifoOverFlow && !PhxCommonKbHit() ) 
			{
				_PHX_SleepMs(10);
			} 
			/* Reset the buffer ready flag to false for next cycle */
			sControlFlags->fBufferReady = FALSE;
			
			/* save the captured image */
			eStat = writeImage(sSO2Parameters);
			if ( PHX_OK != eStat )
			{
				/* if saving failed somehow more than 3 times program stops */
				saveErrCount++;
				if(saveErrCount >= 3)
				{
					printf("saving 3 images in a row failed. Aborting...");
					PHX_Acquire( hCamera, PHX_ABORT, NULL );
					sSO2Parameters->eStat = eStat;
					return 1;
				}
			}
			else
			{
				/* if saving was successful error counter is reset to zero */
				/* image counter is set +1 */
				sSO2Parameters->dImageCounter++;
				saveErrCount = 0;
			}
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
				return 2;
			}
		} // else
	} // while ( !PhxCommonKbHit() && !sControlFlags->fFifoOverFlow ) 

	sSO2Parameters->eStat = eStat;
	return eStat;
}



int writeImage(sParameterStruct *sSO2Parameters)
{
	stImageBuff			stBuffer;	/* Buffer in which the image data is stored by the framegrabber */
	int					status;		/* Status variable for several return values */ 
	char				filename[PHX_MAX_FILE_LENGTH]; 
	int					fwriteCount=2752512; /* (1344*1024*16)/8 = 2752512 Bytes per image 12 bits saved in 16 bits */
	int					fwriteReturn; /* Return value for the write functions */
	FILE				*imageBuffer; /* FIle handle for current image */
	char				headerString[HEADER_SIZE];
	SYSTEMTIME			timeThisImage; /* System time windows.h dependency */

	/* get creation time of image windows.h dependency*/
	GetSystemTime(&timeThisImage);
	
	/* create a filename with milliseconds precession -> caution <windows.h> is used her */ 
	status = createFilename(sSO2Parameters, filename, timeThisImage);
	if (status <= 0)
	{
		/*creating filename failed or filename has length 0 */
		printf("creating filename failed.\n");
		return 1;
	}
	else
	{
		/* reset status if creating a filename was successful */
		status = 0;
	}
	/* create a Fileheader caution <windows.h> is used her */ 
	status = createFileheader(sSO2Parameters, headerString, timeThisImage);
	if (status != 0)
	{
		printf("creating fileheader failed\n");
		return 2;
	}
	
	/*Open a new file for the image (writeable, binary) */
	imageBuffer = fopen(filename,"wb");
	if (imageBuffer == NULL)
	{
		printf("opening image %s failed",filename);
		return 3;
	}
	
	/* download the image from the framegrabber */
	sSO2Parameters->eStat = PHX_Acquire( sSO2Parameters->hCamera, PHX_BUFFER_GET, &stBuffer );
	if ( PHX_OK == sSO2Parameters->eStat )
	{	
		/* save the whole header byte per byte to file */
		fwriteReturn = fwrite(headerString,1,HEADER_SIZE,imageBuffer);
		if( (fwriteReturn - HEADER_SIZE) != 0 )
		{
			printf("Error: writing image header failed.\n");
			fclose(imageBuffer);
			return 4;
		}
		/* save image data byte per byte to file 12-bit information in 2 bytes */
		fwriteReturn = fwrite(stBuffer.pvAddress,1,fwriteCount,imageBuffer);
		fclose(imageBuffer);
		if ((fwriteCount-fwriteReturn) == 0)
			printf("Saving Image %s was successful\n",filename);
		else
		{
			printf("Saving Image %s failed\n",filename);
			return 5;
		}
	}
	else
	{
		printf("downloading Image %s failed\n",filename);
		return 6;
	}
	return 0;
}


int createFilename(sParameterStruct *sSO2Parameters,char * filename, SYSTEMTIME time)
{
	int		status;
	
	/* write header string with information from system time. windows.h dependency */ 
	status = sprintf(filename,"%s%s_%04d_%02d_%02d-%02d_%02d_%02d_%03d.raw",sSO2Parameters->cImagePath,
		sSO2Parameters->cFileNamePrefix, time.wYear, time.wMonth, time.wDay, time.wHour,
		time.wMinute, time.wSecond, time.wMilliseconds);
	
	return status;
}

int createFileheader(sParameterStruct *sSO2Parameters, char * headerstring, SYSTEMTIME time)
{
	int		i;
	
	/* all this does is create an empty 64 bytes long string. later this function
	 * shall create a string equal the header from the hokawo software by Hamamatsu */
	for (i=0; i < HEADER_SIZE; i++)
	{
		headerstring[i]=0;
	}
	return 0;
}

