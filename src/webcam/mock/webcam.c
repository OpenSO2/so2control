#include <stdlib.h>
#include "log.h"
#include "webcam.h"
#include "configurations.h"
#include "timehelpers.h"

static char *buffer;

#pragma GCC diagnostic ignored "-Wunused-parameter"
int webcam_init(sConfigStruct * config, sWebCamStruct * webcam)
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

	webcam->bufferSize = fsize;
	webcam->timestampBefore = (timeStruct*)malloc(sizeof(timeStruct));
	webcam->timestampAfter = (timeStruct*)malloc(sizeof(timeStruct));

	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

int webcam_get(sWebCamStruct * webcam)
{
	webcam->buffer = buffer;
	sleep(.1);
	return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int webcam_uninit(sConfigStruct * config, sWebCamStruct * webcam)
{
	if (buffer != NULL) {
		free(buffer);
	}

	if (webcam->timestampBefore != NULL) {
		free(webcam->timestampBefore);
	}

	if (webcam->timestampAfter != NULL) {
		free(webcam->timestampAfter);
	}

	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

