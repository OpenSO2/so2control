#include"configurations.h"
#include"imageCreation.h"
#define HEADER_SIZE 64

void callbackFunction(
	tHandle hCamera,		/* Camera handle. */
	ui32 dwInterruptMask,	/* Interrupt mask. */
	void *pvParams			/* Pointer to user supplied context */
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
	etStat eStat			= PHX_OK;   /* Status variable */
	tHandle hCamera			= sSO2Parameters->hCamera;
	int	imageCounter		= 0; /* derzeit Ohne verwendung */
	printf("Starting acquisition...\n");
	printf("Press a key to exit\n");
	PHX_Acquire( hCamera, PHX_EXPOSE, NULL );
	while ( !PhxCommonKbHit() && !sControlFlags->fFifoOverFlow ) {

		sSO2Parameters->dImageCounter++;

		/* Now start our capture, return control immediately back zu program */
		eStat = PHX_Acquire( hCamera, PHX_START, (void*) callbackFunction );
		if ( PHX_OK != eStat ) goto Error;

		/* Wait for a user defined period between each camera trigger call*/
		_PHX_SleepMs( sSO2Parameters->dInterFrameDelay );
			
		/* Wait here until either:
		 * (a) The user aborts the wait by pressing a key in the console window
		 * (b) The BufferReady event occurs indicating that the image is complete
		 * (c) The FIFO overflow event occurs indicating that the image is corrupt.
		 * Keep calling the sleep function to avoid burning CPU cycles
		 */
		while ( !sControlFlags->fBufferReady && !sControlFlags->fFifoOverFlow && !PhxCommonKbHit() ) {
			_PHX_SleepMs(10);
		} 
		sControlFlags->fBufferReady = FALSE;

		eStat = writeImage(sSO2Parameters);
		if ( PHX_OK != eStat ){
			printf("saving the image failed\n");
			return 1;
		}
		PHX_Acquire( hCamera, PHX_ABORT, NULL );
	}
Error:
	sSO2Parameters->eStat = eStat;
	
	return 0;
}



int writeImage(sParameterStruct *sSO2Parameters)
{
	stImageBuff			stBuffer;
	char				filename[PHX_MAX_FILE_LENGTH];
	int					fwriteCount, fwriteReturn;
	FILE				*bufferDump;
	char				headerString[HEADER_SIZE];
	int					fileheader[HEADER_SIZE];
	//createFilename(filename);
	/* function createFilename needs to be rewritten!!! */
	/* need to create a variable for paths */
	sprintf(filename,"Images\\Image_%d.raw",sSO2Parameters->dImageCounter);
	
	//createFilename(filename);
	createFileheader(headerString);
	printf("sizeof headerstring= %d \n",sizeof(headerString));
	//createHeader(fileheader);

	bufferDump = fopen(filename,"wb");

	sSO2Parameters->eStat = PHX_Acquire( sSO2Parameters->hCamera, PHX_BUFFER_GET, &stBuffer );
	if ( PHX_OK != sSO2Parameters->eStat ) goto Error;
	
	/* 2752512 Bytes pro Bild
	 * (1344*1024*16)/8 
	 * 12 Bit in 16 Bit Daten!!!!
	 */
	fwriteCount = 2752512; 
	fwrite(headerString,1,HEADER_SIZE,bufferDump);
	fwriteReturn = fwrite(stBuffer.pvAddress,1,fwriteCount,bufferDump);
	fclose(bufferDump);
	printf("Write Image complete\n");

Error:
	return 0;
}

/* 'createFilename' needs to be rewritten in 'C' */

/*
void createFilename(char* filename) {
	
	int		count;
	char*	buffer;
	string	strFilename;
	string	fileEnding = ".raw";
	mvTime	dateTime;

	dateTime = dateTime.Now();
	strFilename = dateTime.GetTimeStamp();
	
	for (count = 0; count < strFilename.length();count++) 
	{
			if (strFilename[count] == ':' ||strFilename[count] == '.' ) 
			{
				strFilename[count] = '-';
			}
	}

	strFilename +=fileEnding;

	//const_cast<> remove if possible
	 buffer = const_cast<char*>(strFilename.c_str());
	strcpy(filename,buffer);

	return;
}*/

int createFilename(char * filename)
{
	
	return 0;
}

int createFileheader(char * headerstring)
{
	int i;
	for (i=0; i < HEADER_SIZE; i++)
	{
		headerstring[i]=0;
	}
	return 0;
}

int createHeader(int * fileheader)
{
	int i;
	for (i=0; i < HEADER_SIZE; i++)
	{
		fileheader[i]=0;
	}
	/* CREATE HERE HEADER */
	return 0;
}