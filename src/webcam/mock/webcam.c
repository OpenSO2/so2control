#include <stdlib.h>
#include "log.h"
#include "webcam.h"
#include "configurations.h"
#include "timehelpers.h"

static char *buffer;

#pragma GCC diagnostic ignored "-Wunused-parameter"
int webcam_init(sConfigStruct * config)
{
	int stat = 0;
	int fsize = 0;

	/* open mock image */
	FILE *fid = fopen(WEBCAM_MOCK_RAW, "rb");
	if (!fid) {
		log_error("image >mock.raw< could not be loaded");
		return -1;
	}

	/* get filesize */
	fseek (fid , 0 , SEEK_END);
	fsize = ftell (fid);
	rewind (fid);

	/* allocate buffer */
	buffer = (char*) malloc (sizeof(char)*fsize);
	if (buffer == NULL) {
		log_error("error allocating enough memory");
		return -1;
	}

	stat = fread(buffer, 1, fsize, fid);
	if (stat != fsize) {
		log_error("couldn't read mock webcam image");
		return -1;
	}
	fclose(fid);
	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

int webcam_get(sWebCamStruct *camStruct)
{
	camStruct->buffer = buffer;
	return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int webcam_uninit(sConfigStruct * config)
{
	free(buffer);
	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

