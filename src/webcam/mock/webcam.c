#include <stdio.h>
#include <stdlib.h>
#include "webcam.h"
#include "configurations.h"
#include "timehelpers.h"

static char *buffer;

int webcam_init(sConfigStruct * config)
{
	return 0;
}

int webcam_get(sWebCamStruct *camStruct)
{
	int stat = 0;
	int fsize = 0;

	/* open mock image */
	FILE *fid = fopen("mock.raw","rb");
	if (fid = 0)
	{
		fprintf(stderr,"image >mock.raw< could not be loaded\n");
		return -1;
	}

	/* get filesize */
	fseek (fid , 0 , SEEK_END);
	fsize = ftell (fid);
	rewind (fid);

	/* allocate buffer */
	camStruct->buffer = (char*) malloc (sizeof(char)*fsize);
	if (camStruct->buffer == NULL)
	{
		fprintf(stderr,"error allocating enough memory\n");
		return -1;
	}

	stat = fread(camStruct->buffer,1,fsize,fid);
	if (stat != fsize)
	{
		fprintf(stderr,"couldnt open mock image\n");
		return -1;
	}
	fclose(fid);

	return 0;
}

int webcam_uninit(sConfigStruct * config)
{
	return 0;
}

