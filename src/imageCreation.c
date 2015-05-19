#ifdef WIN32
#include<windows.h>
#else
#define _POSIX_C_SOURCE 200809L
#endif


#include<string.h> /* memset */
#include<stdlib.h>

#include<time.h>
#include"configurations.h"
#include"imageCreation.h"
#include"log.h"
#include"imageFunctions.h"


#define HEADER_SIZE 64

void callbackFunction( sParameterStruct *sSO2Parameters )
{
	sSO2Parameters->fBufferReady = TRUE;
	sSO2Parameters->dBufferReadyCount++; /* Increment the Display Buffer Ready Count */
}

int startAquisition(sParameterStruct *sParameters_A, sParameterStruct *sParameters_B)
{
	char     filename_A[128] = "";
	char     filename_B[128] = "";

	logMessage("Starting acquisition...\n");
	logMessage("Press a key to exit\n");

	while ( !kbhit() && !(sParameters_A->fFifoOverFlow || sParameters_B->fFifoOverFlow) )
	{
		aquire( sParameters_A, sParameters_B, filename_A, filename_B);
	}

	return 0;
}


int aquire(sParameterStruct *sParameters_A, sParameterStruct *sParameters_B, char *filename_A, char *filename_B)
{
	FILE*		fid				= NULL;
	int			saveErrCount	= 0; /* counting how often saving an image failed */
	int			startErrCount	= 0; /* counting how often the start of capture process failed */
	int			status			= 0; /* status variable */
	tHandle		hCamera_A		= sParameters_A->hCamera;  /* hardware handle of first camera */
	tHandle		hCamera_B		= sParameters_B->hCamera; /* hardware handle of second camera */
	timeStruct  timeNow;

	/* @FIXME: bin mir nicht sicher ob das notwendig ist... C... */
	memset(&timeNow, 0, sizeof(timeNow));

	/* Now start our capture, return control immediately back to program */
	status = camera_trigger( sParameters_A, (void*) &callbackFunction );
	status = camera_trigger( sParameters_B, (void*) &callbackFunction );

	if ( !status )
	{
		/* get current time with milliseconds precision */
		getTime(&timeNow);
		/* if starting the capture was successful reset error counter to zero */
		startErrCount = 0;
		/* Wait for a user defined period between each camera trigger call*/
		/* should be identical in both parameter structures */
		sleepMs( sParameters_A->dInterFrameDelay );

		/* Wait here until either:
		 * (a) The user aborts the wait by pressing a key in the console window
		 * (b) The BufferReady event occurs indicating that the image is complete
		 * (c) The FIFO overflow event occurs indicating that the image is corrupt.
		 * Keep calling the sleep function to avoid burning CPU cycles */
		while ( !(sParameters_A->fBufferReady && sParameters_B->fBufferReady) && !(sParameters_A->fFifoOverFlow && sParameters_B->fFifoOverFlow) && !kbhit() )
		{
			sleepMs(10);
		}
		/* Reset the buffer ready flags to false for next cycle */
		sParameters_A->fBufferReady = FALSE;
		sParameters_B->fBufferReady = FALSE;

		/* save the captured image */
		status = writeImage(sParameters_A, filename_A, timeNow, 'A');
		status = writeImage(sParameters_B, filename_B, timeNow, 'B');
		if ( 0 != status )
		{
			logError("Saving an image failed. This is not fatal");
			/* if saving failed somehow more than 3 times program stops */
			saveErrCount++;
			if(saveErrCount >= 3)
			{
				logError("Saving 3 images in a row failed. This is fatal");
				camera_abort(sParameters_A);
				camera_abort(sParameters_B);
				return 1;
			}
		}
		else
		{
			/* if saving was successful error counter is reset to zero */
			/* image counter is set +1 */
			sParameters_A->dImageCounter++;
			sParameters_B->dImageCounter++;

			saveErrCount = 0;
		}
		camera_abort(sParameters_A);
		camera_abort(sParameters_B);
	}
	else
	{
		logError("Starting the acquisition failed. This is not fatal");
		/* if starting the capture failed more than 3 times program stops */
		startErrCount++;
		if(startErrCount >= 3)
		{
			logError("starting the acquisition failed 3 times in a row. this is fatal");
			camera_abort(sParameters_A);
			camera_abort(sParameters_B);
			return 2;
		}
	} // else

	return 0;
}



int writeImage(sParameterStruct *sSO2Parameters, char *filename, timeStruct timeThisImage, char cameraIdentifier)
{
	stImageBuff  stBuffer;              /* Buffer in which the image data is stored by the framegrabber */
	int          status;                /* Status variable for several return values */
	int          imageByteCount = 1344 * 1024 * 16/8; /* number of pixels times 16 Bit depth in Byte */
	int          fwriteReturn;          /* Return value for the write functions */
	FILE         *imageFile;            /* File handle for current image */
	char         headerString[HEADER_SIZE];
	char         errbuff[512];
	char         messBuff[512];
	tHandle	     hCamera = sSO2Parameters->hCamera;

	if(sSO2Parameters->dImagesFile == 0 && strlen(filename) ){
		logError("dImagesFile cannot be 0. This is fatal.");
		return 1;
	}

	if ( strlen(filename) == 0 || sSO2Parameters->dImageCounter % sSO2Parameters->dImagesFile == 0 || sSO2Parameters->dImageCounter == 0)
	{
		// @FIXME filename should have a camera parameter (e.g. _camera1.rbf)
		status = createFilename(sSO2Parameters, filename, timeThisImage,cameraIdentifier);
		if (status != 0)
		{
			/*creating filename failed or filename has length 0 */
			logError("creating filename failed.");
			return 1;
		}
		else
		{
			/* reset status if creating a filename was successful */
			status = 0;
			sprintf(messBuff, "%09d Image pairs are saved starting a new File", sSO2Parameters->dImageCounter);
			logMessage(messBuff);

			printf("%09d Images are saved. Press a key to exit.\n", sSO2Parameters->dImageCounter);
		}
	}

	status = createFileheader(sSO2Parameters, headerString, &timeThisImage);
	if (status != 0)
	{
		logError("creating fileheader failed");
		return 2;
	}

	/*Open a new file for the image (writeable, binary) */
	imageFile = fopen(filename, "wb");

	//fseek(imageFile, 0,SEEK_END);

	if (imageFile == NULL)
	{
		sprintf(errbuff,"create %s on harddrive failed",filename);
		logError(errbuff);
		return 3;
	}

	/* download the image from the framegrabber */
	status = camera_get(sSO2Parameters, &stBuffer);

	if ( 0 == status )
	{
		/* save the whole header byte per byte to file */
		fwriteReturn = fwrite(headerString, 1, HEADER_SIZE, imageFile);
		if( (fwriteReturn - HEADER_SIZE) != 0 )
		{
			logError("Writing image header failed");
			fclose(imageFile);
			return 4;
		}

		/* rotate one of the images */
		/* imageByteCount/2 = number of pixels */
		if(cameraIdentifier == 'A'){
			rotateImage(stBuffer.pvAddress, imageByteCount/2);
		}

		/* save image data byte per byte to file 12-bit information in 2 bytes */
		// pvAddress => Virtual address of the image buffer
		fwriteReturn = fwrite(stBuffer.pvAddress, 1, imageByteCount, imageFile);

		//fflush(imageFile);
		fclose(imageFile);
		if ( imageByteCount != fwriteReturn )
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


int createFilename(sParameterStruct *sSO2Parameters, char * filename, timeStruct time, char cameraIdentifier)
{
	int status;
	char * camname = (cameraIdentifier == 'A') ? "top" : "bot"; /* identify Camera for filename Prefix */

	/* write header string with information from system time for camera B. */
	status = sprintf(filename, "%s%s_%04d_%02d_%02d-%02d_%02d_%02d_%03d_cam_%s.raw", sSO2Parameters->cImagePath,
		sSO2Parameters->cFileNamePrefix, time.year, time.mon, time.day, time.hour,
		time.min, time.sec, time.milli, camname);
	return status > 0 ? 0 : 1;
}


int createFileheader(sParameterStruct *sSO2Parameters, char * header, timeStruct *time)
{
	/* create a hokawo compatible header */

	short	wID			= 23130;	// Hex 5A5A
	short	wByteOrder	= 18761;	// ASCII 'II'
	short	wVersion	= 12597;	// Version des RAW-Formats
	short	wWidth		= 1344;		// Bildbreite in Pixel
	short	wHeight		= 1024;		// Bildhoehe in Pixel
	short	wBPP		= 16;		// Bits pro Pixel
	short	wColorType	= 1;		// Farbtyp: 2 = Graustufen, 4 = RGB Farbe... ist leider = 1 in beispiel datei aus hokawo software
	short	wPalEntryNo = 0;		// Anzahl von Paletteneintraegen (immer 0 )
	time_t	tDateTime	= TimeFromTimeStruct(time);	// Datum und Uhrzeit
	int dwTimestamp = time->milli;		// Zeitstempel in ms
	double	dExposureTime = sSO2Parameters->dExposureTime;
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

	/* @FIXME: wie macht man das mit floats??? */
//	header[24] = (char)dExposureTime;
//	header[25] = (char)(dExposureTime >> 8);
//	header[26] = (char)(dExposureTime >> 16);
//	header[27] = (char)(dExposureTime >> 24);

	return 0;
}


time_t TimeFromTimeStruct(const timeStruct * pTime)
{
	struct tm tm;
	memset(&tm, 0, sizeof(tm));

	tm.tm_year = pTime->year - 1900;
	tm.tm_mon  = pTime->mon - 1;
	tm.tm_mday = pTime->day;
	tm.tm_hour = pTime->hour;
	tm.tm_min  = pTime->min;
	tm.tm_sec  = pTime->sec;

	return mktime(&tm);
}


#ifdef WIN32


/* WINDOWS VERSION */
int getTime(timeStruct *pTS)
{
	SYSTEMTIME time;
	GetSystemTime(&time);
	pTS->year 	= time.wYear;
	pTS->mon  	= time.wMonth;
	pTS->day  	= time.wDay;
	pTS->hour 	= time.wHour;
	pTS->min  	= time.wMinute;
	pTS->sec 	= time.wSecond;
	pTS->milli	= time.wMilliseconds;
	return 0;
}

#else

/* POSIX VERSION */
int getTime(timeStruct *pTS)
{
	time_t				seconds;
	long				milliseconds;
	struct tm			*tm;
	struct timespec		spec;
	int 				stat;

	stat = clock_gettime(CLOCK_REALTIME, &spec);
	if (stat != 0)
	{
		logError("clock_gettime failed. (posix) \n");
		return 1;
	}

	milliseconds = round(spec.tv_nsec / 1.0e6);
	seconds = spec.tv_sec;
	tm = gmtime(&seconds);

	pTS->year  = tm->tm_year + 1900;
	pTS->mon   = tm->tm_mon +1;
	pTS->day   = tm->tm_mday;
	pTS->hour  = tm->tm_hour;
	pTS->min   = tm->tm_min;
	pTS->sec   = tm->tm_sec;
	pTS->milli = milliseconds;
	return 0;
}
#endif

