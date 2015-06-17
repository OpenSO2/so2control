#include<stdio.h>
#include<time.h>
#include<string.h>
#include<opencv/cv.h>
#include "common.h"
#include "configurations.h"
#include "../io.h"
#include "make_png_header.c"

#define HEADERLENGTH 60

/*prototypes*/
static int createFilename(sParameterStruct *sSO2Parameters, char *filename, char *filetype);
IplImage *bufferToImage(short *buffer);

/* io_init
 *
 */
#pragma GCC diagnostic ignored "-Wunused-parameter"
int io_init(sParameterStruct * sSO2Parameters){
	log_message("io_init");
	return 0;
}
#pragma GCC diagnostic ignored "-Wunused-parameter"


//~
//~ sSO2Parameters->cFileNamePrefix in config
//~ sSO2Parameters->cImagePath in config
int createFilename(sParameterStruct *sSO2Parameters, char *filename, char *filetype)
{
	int status;
	char id = sSO2Parameters->identifier;
	timeStruct *time = sSO2Parameters->timestampBefore;	// Datum und Uhrzeit

	/* identify Camera for filename Prefix */
	char * camname = id == 'a' ? "top" : "bot";

	/* write header string with information from system time for camera B. */
	status =
	    sprintf(filename,
		    "%s%s_%04d_%02d_%02d-%02d_%02d_%02d_%03d_cam_%s.%s",
		    sSO2Parameters->cImagePath, sSO2Parameters->cFileNamePrefix,
		    time->year, time->mon, time->day, time->hour, time->min,
		    time->sec, time->milli, camname, filetype);

	return status > 0 ? 0 : 1;
}

int io_writeDump(sParameterStruct * sSO2Parameters)
{
	FILE * imageFile;
	FILE * fp;
	char headerfile[100];
	char rawfile[100];
	int fwriteReturn;
	int status = 0;

	/* generate filenames */
	status = createFilename(sSO2Parameters, headerfile, "txt");
	if(status){
		log_error("could not create txt filename");
	}
	status = createFilename(sSO2Parameters, rawfile, "raw");
	if(status){
		log_error("could not create txt filename");
	}

	/* Open a new file for the image (writeable, binary) */
	imageFile = fopen(rawfile, "wb");
	if(imageFile != NULL){
		fwriteReturn = fwrite(sSO2Parameters->stBuffer, 1, sSO2Parameters->dBufferlength * 2, imageFile);
		if(fwriteReturn != sSO2Parameters->dBufferlength * 2){
			log_error("could not write raw file");
		}
		fclose(imageFile);
	} else {
		log_error("not opened raw file");
	}
	/* write a text file containing header information */
	fp = fopen(headerfile, "ab");
	if (fp != NULL)
	{
		fprintf(fp, "dBufferlength %i\n", sSO2Parameters->dBufferlength);
		fprintf(fp, "dHistMinInterval %i\n", sSO2Parameters->dHistMinInterval);
		fprintf(fp, "dHistPercentage %i\n", sSO2Parameters->dHistPercentage);
		fprintf(fp, "dDarkCurrent %i\n", (int)sSO2Parameters->dDarkCurrent);
		fprintf(fp, "dImageCounter %i\n", (int)sSO2Parameters->dImageCounter);
		fprintf(fp, "dInterFrameDelay %i\n", (int)sSO2Parameters->dInterFrameDelay);
		fprintf(fp, "dTriggerPulseWidth %i\n", (int)sSO2Parameters->dTriggerPulseWidth);
		fprintf(fp, "dExposureTime %f\n", sSO2Parameters->dExposureTime);
		fprintf(fp, "cConfigFileName %s\n", sSO2Parameters->cConfigFileName);
		fprintf(fp, "cFileNamePrefix %s\n", sSO2Parameters->cFileNamePrefix);
		fprintf(fp, "cImagePath %s\n", sSO2Parameters->cImagePath);
		fprintf(fp, "dFixTime %i\n", sSO2Parameters->dFixTime);
		fprintf(fp, "dfilesize %i\n", sSO2Parameters->dfilesize);
		fprintf(fp, "dImagesFile %i\n", sSO2Parameters->dImagesFile);
		// @TODO: add time

		fclose(fp);
	} else {
		log_error("not opened text file");
	}

	log_message("dumb image written");

	return 0;
}

/*
 *
 */
int io_writeImage(sParameterStruct * sSO2Parameters){
	FILE * fp;
	short * stBuffer;
	IplImage *img;
	CvMat *png;
	int l_pad;
	int l;
	int ii;
	char filename[100];
	int status;

	/*
	 * int head[HEADERLENGTH];
	 * int l, l_pad, i;
	 * char *name;
	 *
	 * unsigned char *padded_png;
	 * char date[25];
	 * char text[39];
	 */

	stBuffer = sSO2Parameters->stBuffer;

	/* generate filenames */
	status = createFilename(sSO2Parameters, filename, "png");
	if(status){
		log_error("could not create txt filename");
	}

	/* convert the image buffer to an openCV image */
	// TODO: check if this has already been done
	img = bufferToImage(stBuffer);

	/*
	 * encode image as png to buffer
	 * playing with the compression is a huge waste of time with no benefit
	 */
	png = cvEncodeImage(".png", img, 0);
	l = png->rows * png->cols;
	l_pad = l;

	/* add headers */
	l_pad = insertHeaders(png->data.ptr, sSO2Parameters, l);

printf("done ");
printf("%c", png->data.ptr[l - 9]);
printf("%c", png->data.ptr[l - 8]);
printf("%c", png->data.ptr[l - 7]);
printf("%c", png->data.ptr[l - 6]);
printf("%c", png->data.ptr[l - 5]);
printf("%c", png->data.ptr[l - 4]);
printf("%c", png->data.ptr[l - 3]);
printf("%c", png->data.ptr[l - 2]);
printf("%c", png->data.ptr[l - 1]);
printf("\n");

printf("done ");
printf("%c", png->data.ptr[l_pad - 9]);
printf("%c", png->data.ptr[l_pad - 8]);
printf("%c", png->data.ptr[l_pad - 7]);
printf("%c", png->data.ptr[l_pad - 6]);
printf("%c", png->data.ptr[l_pad - 5]);
printf("%c", png->data.ptr[l_pad - 4]);
printf("%c", png->data.ptr[l_pad - 3]);
printf("%c", png->data.ptr[l_pad - 2]);
printf("%c", png->data.ptr[l_pad - 1]);
printf("\n");

	/* save image to disk*/
	fp = fopen(filename, "wb");
	if (fp) {
		ii = fwrite(png->data.ptr, 1, l_pad, fp);
//		log_debug("write image l_pad %i, return %i\n", l_pad, ii);
		log_debug("write image l_pad %i, return %i\n");
		// FIXME: check return value
	} else {
		log_error("Something wrong writing to File.");
	}

	log_message("png image written");

	return 0;
}

int insertHeaders(char * png, sParameterStruct * sSO2Parameters, int l){
	int l_pad = l;

	char name[14] =    "Creation Time ";
	char content[15] = "hello my friend";
	l_pad = insertHeader(png, name, content, l);

	return l_pad;
}

int insertHeader(char * png, char * name, char * content, int l){
	int head[HEADERLENGTH];
	char text[40]; // can be of arbitrary length, but must be shorter than HEADERLENGTH
	int l_pad, i;
	l_pad = l + HEADERLENGTH;

	char * padded_png = (unsigned char *)malloc(l_pad * 2);
	memcpy(padded_png, png, l_pad);
	//~ png = padded_png;

	// copy end of png
	printf("l: %i; l_pad: %i\n", l, l_pad);
	for (i = 8; i > 0; i--) {
		png[l_pad - i] = png[l - i];

		printf("nocpy char at %i: %c; %i: %c\n", l - i, png[l - i], l_pad - i, png[l_pad - i]);
	}

	strcpy(text, name);
	strcat(text, content);
	text[13] = 0;
int content_length = 15;
printf("name=%s\ncontent=%s\ntext=%s\n", name, content, text);

	make_png_header(text, content_length, head, HEADERLENGTH);

	// fill in
	for (i = 0; i < HEADERLENGTH; i++) {
		png[l - 12 + i] = head[i];
	}

	return l_pad;
}

/*
 *
 */
int io_uninit(sParameterStruct * sSO2Parameters){
	log_message("io_uninit");
	return 0;
}
