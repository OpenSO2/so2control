#include<windows.h>
#include<time.h>
#include"configurations.h"
#include"imageCreation.h"
#include"log.h"
#include"imageFunctions.h"

#define HEADER_SIZE 64

void callbackFunction(
	tHandle     hCamera,           /* Camera handle. */
	ui32        dwInterruptMask,   /* Interrupt mask. */
	void        *pvParams          /* Pointer to user supplied context */
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
	etStat   eStat             = PHX_OK; /* Status variable */
	tHandle  hCamera_A         = sSO2Parameters->hCamera_A;  /* hardware handle of first camera */
	tHandle  hCamera_B         = sSO2Parameters->hCamera_B; /* hardware handle of second camera */
	char     filename[PHX_MAX_FILE_LENGTH] = "";
	char     filename2[PHX_MAX_FILE_LENGTH] = "";

	printf("Starting acquisition...\n");
	printf("Press a key to exit\n");

	/* Starting the acquisition with the exposure parameter set in configurations.c and exposureTimeControl.c */
	// @FIXME: set exposure times independently
	PHX_Acquire( hCamera_A,  PHX_EXPOSE, NULL );
	PHX_Acquire( hCamera_B, PHX_EXPOSE, NULL );

	while ( !PhxCommonKbHit() && !sControlFlags->fFifoOverFlow )
	{
		//~ @FIXME: return codes
		aquire( sSO2Parameters, sControlFlags, filename, filename2);
		// merge both images to correct for particles


	} // while ( !PhxCommonKbHit() && !sControlFlags->fFifoOverFlow )

	sSO2Parameters->eStat = eStat;
	return eStat;
}


int aquire(sParameterStruct *sSO2Parameters, flagStruct *sControlFlags, char *filename, char *filename2)
{
	FILE*    fid             = NULL;
	int      saveErrCount    = 0; /* counting how often saving an image failed */
	int      startErrCount   = 0; /* counting how often the start of capture process failed */
	int      status          = 0; /* status variable */
	etStat   eStat           = PHX_OK; /* Status variable */
	tHandle  hCamera_A         = sSO2Parameters->hCamera_A;  /* hardware handle of first camera */
	tHandle  hCamera_B        = sSO2Parameters->hCamera_B; /* hardware handle of second camera */
	SYSTEMTIME   timeNow;         /* System time windows.h dependency */

	/* Now start our capture, return control immediately back to program */
	eStat = PHX_Acquire( hCamera_A, PHX_START, (void*) callbackFunction );
	eStat = PHX_Acquire( hCamera_B, PHX_START, (void*) callbackFunction ); /* PROBLEM HIER MIT 2 MAL CALLBACK FUNCTION??????? */
	if ( PHX_OK == eStat )
	{
		/* get time of image, windows.h dependency*/
		GetSystemTime(&timeNow);
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
		eStat = writeImage(sSO2Parameters, filename, hCamera_A,timeNow);
		eStat = writeImage(sSO2Parameters, filename2, hCamera_B, timeNow);
		if ( PHX_OK != eStat )
		{
			logError("Saving an image failed. This is not fatal");
			/* if saving failed somehow more than 3 times program stops */
			saveErrCount++;
			if(saveErrCount >= 3)
			{
				logError("Saving 3 images in a row failed. This is fatal");
				PHX_Acquire( hCamera_A, PHX_ABORT, NULL );
				PHX_Acquire( hCamera_B, PHX_ABORT, NULL );
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
		PHX_Acquire( hCamera_A, PHX_ABORT, NULL );
		PHX_Acquire( hCamera_B, PHX_ABORT, NULL );
	} // if ( PHX_OK == eStat )
	else
	{
		logError("Starting the acquisition failed. This is not fatal");
		/* if starting the capture failed more than 3 times program stops */
		startErrCount++;
		if(startErrCount >= 3)
		{
			logError("starting the acquisition failed 3 times in a row. this is fatal");
			PHX_Acquire( hCamera_A, PHX_ABORT, NULL );
			PHX_Acquire( hCamera_B, PHX_ABORT, NULL );
			sSO2Parameters->eStat = eStat;
			return 2;
		}
	} // else

	return 1;
}







int writeImage(sParameterStruct *sSO2Parameters, char *filename, tHandle hCamera, SYSTEMTIME timeThisImage)
{
	stImageBuff  stBuffer;              /* Buffer in which the image data is stored by the framegrabber */
	int          status;                /* Status variable for several return values */
	int          imageBitCount = 1344 * 1024 * 16/8; /* number of pixels times 16 Byte depth in bit */
	int          fwriteReturn;          /* Return value for the write functions */
	FILE         *imageFile;            /* File handle for current image */
	char         headerString[HEADER_SIZE];
	char         errbuff[512];
	char         messBuff[512];


	/* create a filename with milliseconds precession -> caution <windows.h> is used her */
	if ( strlen(filename) == 0 || sSO2Parameters->dImageCounter%sSO2Parameters->dImagesFile == 0 || sSO2Parameters->dImageCounter == 0)
	{
		// @FIXME filename should have a camera parameter (e.g. _camera1.rbf)
		status = createFilename(sSO2Parameters, filename, timeThisImage, hCamera);
		if (status <= 0)
		{
			/*creating filename failed or filename has length 0 */
			logError("creating filename failed.");
			return 1;
		}
		else
		{
			/* reset status if creating a filename was successful */
			status = 0;
			sprintf(messBuff,"%09d Image pairs are saved starting a new File",sSO2Parameters->dImageCounter);
			logMessage(messBuff);
			printf("%09d Images are saved. Press a key to exit.\n",sSO2Parameters->dImageCounter);
		}
	}

	/* create a Fileheader caution <windows.h> is used here */
	status = createFileheader(headerString, &timeThisImage);
	if (status != 0)
	{
		logError("creating fileheader failed");
		return 2;
	}

	/*Open a new file for the image (writeable, binary) */
	imageFile = fopen(filename,"wb");

	//fseek(imageFile, 0,SEEK_END);

	if (imageFile == NULL)
	{
		sprintf(errbuff,"create %s on harddrive failed",filename);
		logError(errbuff);
		return 3;
	}

	/* download the image from the framegrabber */
	sSO2Parameters->eStat = PHX_Acquire( hCamera, PHX_BUFFER_GET, &stBuffer );
	if ( PHX_OK == sSO2Parameters->eStat )
	{
		/* save the whole header byte per byte to file */
		fwriteReturn = fwrite(headerString, 1, HEADER_SIZE, imageFile);
		if( (fwriteReturn - HEADER_SIZE) != 0 )
		{
			logError("Writing image header failed");
			fclose(imageFile);
			return 4;
		}

		/* rotate the upper of the images */
		if(hCamera == sSO2Parameters->hCamera_A){
			rotate180(stBuffer.pvAddress);
		}

		/* save image data byte per byte to file 12-bit information in 2 bytes */
		// pvAddress => Virtual address of the image buffer
		fwriteReturn = fwrite(stBuffer.pvAddress, 1, imageBitCount, imageFile);

		//fflush(imageFile);
		fclose(imageFile);
		if ( imageBitCount != fwriteReturn )
		{
			sprintf(errbuff,"Saving Image %s failed\n", filename);
			logError(errbuff);
			return 5;
		}
	}
	else
	{
		sprintf(errbuff,"downloading Image %s from framegrabber failed",filename);
		logError(errbuff);
		return 6;
	}
	return 0;
}


int createFilename(sParameterStruct *sSO2Parameters, char * filename, SYSTEMTIME time, tHandle hCamera)
{
	int status;
	char * camname = (hCamera == sSO2Parameters->hCamera_A) ? "top" : "bot"; /* identify Camera via handle for filename Prefix */

	/* write header string with information from system time for camera B. windows.h dependency */
	status = sprintf(filename, "%s%s_%04d_%02d_%02d-%02d_%02d_%02d_%03d_cam_%s.raw", sSO2Parameters->cImagePath,
		sSO2Parameters->cFileNamePrefix, time.wYear, time.wMonth, time.wDay, time.wHour,
		time.wMinute, time.wSecond, time.wMilliseconds, camname);

	return status;
}

time_t TimeFromSystemTime(const SYSTEMTIME * pTime)
{
	/* copied from: http://forums.codeguru.com/showthread.php?139510-Converting-SYSTEMTIME-to-time_t */
	struct tm tm;
	memset(&tm, 0, sizeof(tm));

	tm.tm_year = pTime->wYear - 1900;
	tm.tm_mon  = pTime->wMonth - 1;
	tm.tm_mday = pTime->wDay;
	tm.tm_hour = pTime->wHour;
	tm.tm_min  = pTime->wMinute;
	tm.tm_sec  = pTime->wSecond;

	return mktime(&tm);
}

int createFileheader(char * header, SYSTEMTIME *time)
{
	/* create a hokawo compatible header */

	WORD	wID			= 23130;	// Hex 5A5A
	WORD	wByteOrder	= 18761;	// ASCII 'II'
	WORD	wVersion	= 12597;	// Version des RAW-Formats
	WORD	wWidth		= 1344;		// Bildbreite in Pixel
	WORD	wHeight		= 1024;		// Bildhöhe in Pixel
	WORD	wBPP		= 16;		// Bits pro Pixel
	WORD	wColorType	= 1;		// Farbtyp 2 = Graustufen, 4 = RGB Farbe ist leider = 1 in beispiel datei
	WORD	wPalEntryNo = 0;		// Anzahl von Paletteneinträgen (immer 0 )
	time_t	tDateTime	= TimeFromSystemTime(time);	// Datum und Uhrzeit
	DWORD  dwTimestamp = time->wMilliseconds;		// Zeitstempel in ms

	/* Preset the whole string with zeros */
	memset(header,'\0',HEADER_SIZE);

	/* setting the header parameters */
	header[0] = (char)wID;
	header[1] = (char)(wID >> 8);
	header[2] = (char)wByteOrder;
	header[3] = (char)(wByteOrder >> 8);
	header[4] = (char)wVersion;
	header[5] = (char)(wVersion >> 8);
	header[6] = (char)wWidth;
	header[7] = (char)(wWidth >> 8);
	header[8] = (char)wHeight;
	header[9] = (char)(wHeight >> 8);
	header[10] = (char)wBPP;
	header[11] = (char)(wBPP >> 8);
	header[12] = (char)wColorType;
	header[13] = (char)(wColorType >> 8);
	header[14] = (char)wPalEntryNo;
	header[15] = (char)(wPalEntryNo >> 8);
	header[16] = (char)tDateTime;
	header[17] = (char)(tDateTime >> 8);
	header[18] = (char)(tDateTime >> 16);
	header[19] = (char)(tDateTime >> 24);
	header[20] = (char)dwTimestamp;
	header[21] = (char)(dwTimestamp >> 8);
	header[22] = (char)(dwTimestamp >> 16);
	header[23] = (char)(dwTimestamp >> 24);

	return 0;
}
