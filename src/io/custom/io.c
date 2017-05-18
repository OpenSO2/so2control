#include<stdio.h>
#include<time.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<string.h>
#include<opencv/cv.h>
#include<opencv/highgui.h>
#include "configurations.h"
#include "../io.h"
#include "timehelpers.h"
#include "make_png_header.c"
#include "comm.h"

/* local prototypes */
int createFilename(sConfigStruct * config, char *filename, int filenamelength, timeStruct * time, char *camname, char *filetype);
IplImage *bufferToImage(short *buffer);
IplImage *bufferToImageCam(char *buffer);
int dateStructToISO8601(timeStruct * time, char *iso_date);
int insertValue(char **png, char *name, float value, int png_length);
int insertStringValue(char **png, char *name, char *value, int png_length);
int insertHeader(char **png, char *name, char *content, int png_length);
int insertHeaders(char **png, sParameterStruct * sSO2Parameters, sConfigStruct * config, int png_length);
int io_writeImage(sParameterStruct * sSO2Parameters, sConfigStruct * config);
int io_writeDump(sParameterStruct * sSO2Parameters, sConfigStruct * config);
int io_writeWebcamDump(sWebCamStruct * webcam, sConfigStruct * config);
int io_writeWebcamImage(sWebCamStruct * webcam, sConfigStruct * config);

/*
 * `io_init`
 * Initialize the IO functionality. Currently, this does nothing, but this could be the place to
 * set up folders and start tar files.
 */
int io_init(sConfigStruct * config)
{
#define SUBFOLDER_STR_LEN 18
	int status;
	static timeStruct now;
	struct stat st = { 0 };
	char *subfolder = NULL;
	getTime(&now);

	if (strcmp(config->cImagePath, "-") == 0) {
		// short circuit
		log_debug("skip io init");
		free(subfolder);
		return 0;
	}
	// check for and remove trailing "/" to avoid ugly "//" in imagePath
	if (config->cImagePath[(strlen(config->cImagePath) - 1)] == '/') {
		config->cImagePath[(strlen(config->cImagePath) - 1)] = '\0';
	}
	// eg. /2016-08-08_12_12/
	//     123456789012345678
	if (config->createsubfolders) {
		subfolder = (char *)malloc(SUBFOLDER_STR_LEN + 1);
		sprintf(subfolder, "/%04d-%02d-%02d_%02d_%02d/",
			now.year,
			now.mon,
			now.day,
			now.hour,
			now.min);
	} else {
		subfolder = (char *)malloc(1 + 1);
		sprintf(subfolder, "/");
	}
	config->cImagePath = (char *)realloc(config->cImagePath, strlen(config->cImagePath) + strlen(subfolder) + 1);
	strcat(config->cImagePath, subfolder);

	free(subfolder);

	// create targetfolder if not exist
	if (stat(config->cImagePath, &st) == -1) {
		status = mkdir(config->cImagePath, 0700);

		if (status != 0) {
			log_error("output folder (%s) does not exist and could not be created; create parent folder to fix this issue", config->cImagePath);
			return 1;
		}
	}

	return 0;
}

/*
 * `io_write`
 * Writes the current image buffer to disk. Delegates the real work to
 * `io_writeImage` or `io_writeDump`, depending on the value of processing
 * which was set in `io_init`.
 */
int io_write(sParameterStruct * sSO2Parameters, sConfigStruct * config)
{
	/*
	 * processing:
	 *   1 = dumb
	 *   2 = png
	 *   3 = both
	 * else = both
	 *
	 * e.g.
	 *  !1 &&  2 = 2
	 *  !2 &&  1 = 1
	 *  !1 && !2 = 3
	 */
	int state = 0;
	if (config->processing != 1) {
		state = io_writeImage(sSO2Parameters, config);
		if (state != 0) {
			log_error("failed to write png");
			return state;
		}
	}
	if (config->processing != 2) {
		state = io_writeDump(sSO2Parameters, config);

		if (state != 0) {
			log_error("failed to write raw dump");
			return state;
		}
	}

	return 0;
}

/*
 * `io_write`
 * Writes the current webcam image buffer to disk. Delegates the real work to
 * `io_writeWebcamImage` or `io_writeWebcamDump`, depending on the value of processing
 * which was set in `io_init`.
 */
int io_writeWebcam(sWebCamStruct * webcam, sConfigStruct * config)
{
	/*
	 * processing:
	 *   1 = dumb
	 *   2 = png
	 *   3 = both
	 * else = both
	 *
	 * e.g.
	 *  !1 &&  2 = 2
	 *  !2 &&  1 = 1
	 *  !1 && !2 = 3
	 */
	int state = 0;
	if (config->processing != 1) {
		state = io_writeWebcamImage(webcam, config);
		if (state != 0) {
			log_error("failed to write webcam png");
			return state;
		}
	}
	if (config->processing != 2) {
		state = io_writeWebcamDump(webcam, config);

		if (state != 0) {
			log_error("failed to write raw webcam dump");
			return state;
		}
	}

	return 0;
}

int io_writeWebcamImage(sWebCamStruct * webcam, sConfigStruct * config)
{
	int png_length;
	int writen_bytes;
	FILE *fp;
	char *buffer;
	int state;
	char filename[512];
	char iso_date[25];
	FILE *f;
	int filenamelength = 512;
	IplImage *img;
	CvMat *png;

	/* create new image to hold the loaded data*/
	CvSize mSize;

	mSize.height = webcam->height;
	mSize.width = webcam->width;

	img = cvCreateImage(mSize, IPL_DEPTH_8U, 3);
	if (!img) {
		log_error("failed to decode image");
		return -1;
	}

	memcpy(img->imageData, webcam->buffer, webcam->bufferSize);

	/*
	 * encode image as png to buffer
	 * playing with the compression is a huge waste of time with no benefit
	 */
	png = cvEncodeImage(".png", img, 0);

	png_length = png->rows * png->cols;
	cvReleaseImage(&img);

	/* pry the actual buffer pointer from png */
	buffer = (char *)malloc(png_length);
	memcpy(buffer, png->data.s, png_length);
	cvReleaseMat(&png);

	/* add headers */
	dateStructToISO8601(webcam->timestampBefore, iso_date);
	png_length = insertHeader(&buffer, "Creation Time ", iso_date, png_length);
	png_length = insertStringValue(&buffer, "timestampBefore", iso_date, png_length);

	dateStructToISO8601(webcam->timestampAfter, iso_date);
	png_length = insertStringValue(&buffer, "timestampAfter", iso_date, png_length);

#ifdef VERSION
	png_length = insertStringValue(&buffer, "version", VERSION, png_length);
#endif

	comm_set_buffer("cam", buffer, png_length);

	if (strcmp(config->cImagePath, "-") == 0) {
		// short circuit
		log_debug("do not save png webcam image");

		/* cleanup */
		free(buffer);

		return 0;
	}

	state = createFilename(config, filename, filenamelength, webcam->timestampBefore, "webcam", "png");
	if (state) {
		log_error("could not create webcam filename");
		return state;
	}

	/* save image to disk */
	log_debug("open new png file %i", png_length);

	fp = fopen(filename, "wb");
	if (fp) {
		writen_bytes = fwrite(buffer, 1, png_length, fp);
		state = writen_bytes == png_length ? 0 : 1;
		if (state) {
			log_error("PNG image wasn't written correctly");
		}
		fclose(fp);
	} else {
		state = 1;
		log_error("Failed to open file to save webcam image. File name was %s", filename);
	}

	/* cleanup */
	free(buffer);

	if (!state) {
		log_message("webcam png image written");
	}

	return 0;
}

int io_writeWebcamDump(sWebCamStruct * webcam, sConfigStruct * config)
{
	int state;
	char filename[512];
	char iso_date[25];
	FILE *f;
	int filenamelength = 512;

	if (strcmp(config->cImagePath, "-") == 0) {
		// short circuit
		log_debug("do not save raw webcam image dump");
		return 0;
	}

	state = createFilename(config, filename, filenamelength, webcam->timestampBefore, "webcam", "raw");
	if (state) {
		log_error("could not create webcam filename");
		return state;
	}

	f = fopen(filename, "wb");
	if (!f) {
		log_error("Failed to open file to save webcam image");
		log_debug("Filename was %s", filename);
		return 1;
	}

	state = fwrite(webcam->buffer, sizeof(char), webcam->bufferSize, f);
	if (state != webcam->bufferSize) {
		log_error("failed to save image");
		log_debug("buffersize stat = %d; fwrite state = %d", webcam->bufferSize, state);
		return 1;
	} else {
		log_debug("IMAGE: %s saved successful", filename);
	}

	fclose(f);

	state = createFilename(config, filename, filenamelength, webcam->timestampBefore, "webcam_meta", "txt");
	if (state) {
		log_error("could not create webcam filename");
		return state;
	}

	f = fopen(filename, "wt");
	if (f) {
		fprintf(f, "height %i\n", webcam->height);
		fprintf(f, "width %i\n", webcam->width);
		dateStructToISO8601(webcam->timestampBefore, iso_date);
		fprintf(f, "timestampBefore %s\n", iso_date);
		dateStructToISO8601(webcam->timestampAfter, iso_date);
		fprintf(f, "timestampAfter %s\n", iso_date);
#ifdef VERSION
		fprintf(f, "version %s\n", VERSION);
#endif
	}
	fclose(f);

	return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int io_spectrum_save_calib(sSpectrometerStruct * spectro, sConfigStruct * config)
{
	FILE *f;
	int i;
	int state = 1;
	char filename[512];
	int filenamelength = 512;

	state = createFilename(config, filename, filenamelength, spectro->timestampBefore, "dark-current", "dat");
	if (state) {
		log_error("could not create spectroscopy calibration filename (dark current)");
		return state;
	}

	f = fopen(filename, "wt");
	if (f) {
		for (i = 0; i < spectro->spectrum_length; i++) {
			fprintf(f, "%f %f \n", spectro->wavelengths[i], spectro->dark_current[i]);
		}
		fclose(f);
	} else {
		log_error("failed to write dark current");
	}

	state = createFilename(config, filename, filenamelength, spectro->timestampBefore, "electronic-offset", "dat");
	if (state) {
		log_error("could not create spectroscopy calibration filename (electronic offset)");
		return state;
	}

	f = fopen(filename, "wt");
	if (f) {
		for (i = 0; i < spectro->spectrum_length; i++) {
			fprintf(f, "%f %f \n", spectro->wavelengths[i], spectro->electronic_offset[i]);
		}
		fclose(f);
	} else {
		log_error("failed to write electronic offset");
	}

	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

int io_spectrum_save(sSpectrometerStruct * spectro, sConfigStruct * config)
{
	FILE *f;
	int i;
	char iso_date[25];
	char filename[512];
	int filenamelength = 512;

	comm_set_buffer("spc", (char *)spectro->lastSpectrum, spectro->spectrum_length * sizeof(char));

	if (strcmp(config->cImagePath, "-") == 0) {
		// short circuit
		log_debug("do not save spectrum");
		return 0;
	}

	createFilename(config, filename, filenamelength, spectro->timestampBefore, "spectrum", "dat");

	/* save spectrum */
	f = fopen(filename, "wt");
	if (f) {
		for (i = 0; i < spectro->spectrum_length; i++) {
			fprintf(f, "%f %f \n", spectro->wavelengths[i], spectro->lastSpectrum[i]);
		}
		fclose(f);
	}

	/* save meta */
	createFilename(config, filename, filenamelength, spectro->timestampBefore, "spectrum_meta", "txt");
	f = fopen(filename, "wt");
	if (f) {
		fprintf(f, "max %f\n", spectro->max);
		fprintf(f, "integration_time_micros %i\n", spectro->integration_time_micros);
		fprintf(f, "spectrum_length %i\n", spectro->spectrum_length);
		dateStructToISO8601(spectro->timestampBefore, iso_date);
		fprintf(f, "timestampBefore %s\n", iso_date);
		dateStructToISO8601(spectro->timestampAfter, iso_date);
		fprintf(f, "timestampAfter %s\n", iso_date);
		fprintf(f, "scans %i\n", spectro->scans);

#ifdef VERSION
		fprintf(f, "version %s\n", VERSION);
#endif
		fclose(f);
	}

	return 0;
}

/*
 * io_uninit
 * uninitialize/stops the IO functionality. Currently, this does nothing,
 * but could be the place to finish tar archives, calculate checksums,
 * return file handlers and do general clean up.
 */
#pragma GCC diagnostic ignored "-Wunused-parameter"
int io_uninit(sConfigStruct * config)
{
	return 0;
}

#pragma GCC diagnostic warning "-Wunused-parameter"

/*--------------------------------------------------------------------
 * Additional, private functions that are used within this file
 * (compilation unit) but are not available elsewhere.
 */

int io_writeDump(sParameterStruct * sSO2Parameters, sConfigStruct * config)
{
	FILE *imageFile;
	FILE *f;
	char headerfile[512];
	int headerfilelength = 512;
	char rawfile[512];
	int rawfilelength = 512;
	int fwriteReturn;
	int state = 0;
	char iso_date[25];
	char id = sSO2Parameters->identifier;
	timeStruct *time = sSO2Parameters->timestampBefore;	// Datum und Uhrzeit

	if (strcmp(config->cImagePath, "-") == 0) {
		// short circuit
		log_debug("do not save raw image dump");
		return 0;
	}

	/* identify Camera for filename Prefix */
	char *camname = sSO2Parameters->dark ? (id == 'a' ? (char *)"top_dark" : (char *)"bot_dark") : (id == 'a' ? (char *)"top" : (char *)"bot");

	state = createFilename(config, headerfile, headerfilelength, time, camname, "txt");
	if (state) {
		log_error("could not create txt filename");
	}

	/* generate filenames */
	state = createFilename(config, rawfile, rawfilelength, time, camname, "raw");
	if (state) {
		log_error("could not create txt filename");
	}

	/* Open a new file for the image (writeable, binary) */
	imageFile = fopen(rawfile, "wb");
	if (imageFile != NULL) {
		fwriteReturn = fwrite(sSO2Parameters->stBuffer, 1, config->dBufferlength * 2, imageFile);
		if (fwriteReturn != config->dBufferlength * 2) {
			log_debug("could not write raw file %i != %i", config->dBufferlength, fwriteReturn);
			log_error("could not write raw file");
		}
		fclose(imageFile);
	} else {
		log_error("could not open raw file");
	}

	/* write a text file containing header information */
	f = fopen(headerfile, "ab");
	if (f != NULL) {
		fprintf(f, "dBufferlength %i\n", config->dBufferlength);
		fprintf(f, "dImageCounter %i\n", (int)config->dImageCounter);
		fprintf(f, "dInterFrameDelay %i\n", (int)config->dInterFrameDelay);
		fprintf(f, "dExposureTime %f\n", sSO2Parameters->dExposureTime);
		fprintf(f, "cConfigFileName %s\n", config->cConfigFileName);
		fprintf(f, "cFileNamePrefix %s\n", config->cFileNamePrefix);
		fprintf(f, "cImagePath %s\n", config->cImagePath);
		fprintf(f, "dFixTime %i\n", config->dFixTime);
		dateStructToISO8601(sSO2Parameters->timestampBefore, iso_date);
		fprintf(f, "timestampBefore %s\n", iso_date);
		dateStructToISO8601(sSO2Parameters->timestampAfter, iso_date);
		fprintf(f, "timestampAfter %s\n", iso_date);
#ifdef VERSION
		fprintf(f, "version %s\n", VERSION);
#endif

		fclose(f);
	} else {
		log_error("could not open text file");
	}

	log_message("dumb image written");

	return 0;
}

int io_writeImage(sParameterStruct * sSO2Parameters, sConfigStruct * config)
{
	FILE *fp;
	short *stBuffer;
	IplImage *img;
	CvMat *png;
	int l;
	int writen_bytes;
	char filename[512];
	int filenamelength = 512;
	int state;
	char *buffer;
	char *camname;
	timeStruct *time;
	char id = sSO2Parameters->identifier;

	stBuffer = sSO2Parameters->stBuffer;

	time = sSO2Parameters->timestampBefore;	// Datum und Uhrzeit

	/* identify camera for filename prefix */
	camname = sSO2Parameters->dark ? (id == 'a' ? (char *)"top_dark" : (char *)"bot_dark") : (id == 'a' ? (char *)"top" : (char *)"bot");

	log_debug("filename created: %s", filename);

	/* convert the image buffer to an openCV image */
	// TODO: check if this has already been done
	// TODO: check return code
	img = bufferToImage(stBuffer);

	/*
	 * encode image as png to buffer
	 * playing with the compression is a huge waste of time with no benefit
	 */
	png = cvEncodeImage(".png", img, 0);

	l = png->rows * png->cols;
	cvReleaseImage(&img);

	/* pry the actual buffer pointer from png */
	buffer = (char *)malloc(l);
	memcpy(buffer, png->data.s, l);
	cvReleaseMat(&png);

	comm_set_buffer(camname, buffer, l);

	if (strcmp(config->cImagePath, "-") == 0) {
		// short circuit
		log_debug("do not save png image");

		/* cleanup */
		free(buffer);

		return 0;
	}

	state = createFilename(config, filename, filenamelength, time, camname, "png");
	if (state) {
		log_error("could not create txt filename");
	}

	/* add headers */
	log_debug("insert headers %i", l);
	l = insertHeaders(&buffer, sSO2Parameters, config, l);

	/* save image to disk */
	log_debug("open new png file %i", l);
	fp = fopen(filename, "wb");

	if (fp) {
		writen_bytes = fwrite(buffer, 1, l, fp);
		state = writen_bytes == l ? 0 : 1;
		if (state) {
			log_error("PNG image wasn't written correctly");
		}
		fclose(fp);
	} else {
		state = 1;
		log_error("Couldn't open png file");
	}

	/* cleanup */
	free(buffer);

	if (!state) {
		log_message("png image written");
	}

	return state;
}

int insertHeaders(char **png, sParameterStruct * sSO2Parameters, sConfigStruct * config, int png_length)
{
	char iso_date[25];
	dateStructToISO8601(sSO2Parameters->timestampBefore, iso_date);
	png_length = insertHeader(png, "Creation Time ", iso_date, png_length);
	png_length = insertStringValue(png, "timestampBefore", iso_date, png_length);

	dateStructToISO8601(sSO2Parameters->timestampAfter, iso_date);
	png_length = insertStringValue(png, "timestampAfter", iso_date, png_length);

	png_length = insertValue(png, "dBufferlength",      (float)config->dBufferlength,      png_length);
	png_length = insertValue(png, "dImageCounter",      (float)config->dImageCounter,      png_length);
	png_length = insertValue(png, "dInterFrameDelay",   (float)config->dInterFrameDelay,   png_length);
	png_length = insertValue(png, "dExposureTime",      (float)sSO2Parameters->dExposureTime,      png_length);
	png_length = insertValue(png, "dFixTime",           (float)config->dFixTime,           png_length);
#ifdef VERSION
	png_length = insertStringValue(png, "version", VERSION, png_length);
#endif

	return png_length;
}

int insertValue(char **png, char *name, float value, int png_length)
{
	char text[200];
	sprintf(text, "%s: %f", name, value);
	return insertHeader(png, "Comment ", text, png_length);
}

int insertStringValue(char **png, char *name, char *value, int png_length)
{
	char text[200];
	sprintf(text, "%s: %s", name, value);
	return insertHeader(png, "Comment ", text, png_length);
}

int insertHeader(char **png, char *name, char *content, int png_length)
{
	int head[200];
	char *text;
	int png_length_padded;
	int name_length = strlen(name);
	int l, i;
	int header_length;
	char *padded_png;

	l = strlen(name) + strlen(content);
	text = (char *)calloc(sizeof(char), l + 1);
	if (text == NULL) {
		log_error("unable to allocate memory for header text");
		return 2;
	}

	text = strncpy(text, name, strlen(name));
	text = strncat(text, content, strlen(content));

	/*
	 * the header length is the header content length, plus 12 bytes
	 * for the chunks signature (see below)
	 */
	header_length = l + 12;

	/*
	 * In PNG text chunks, the keyword and content is separated by a
	 * NULL byte. This will also means that strlen and printf("%s")
	 * wont work anymore after this point, since they depend on the
	 * NULL byte as the string terminator.
	 */
	text[name_length - 1] = 0;

	// TODO: handle return code
	make_png_header(text, l, head, header_length);

	/* the new PNG length is the old one, plus the header */
	png_length_padded = png_length + header_length;

	/* cleanup */
	free(text);

	log_debug("trying to realloc %i chars at %i", png_length_padded, png);
	/* Resize the PNG buffer to accommodate for the additional text chunk */
	padded_png = (char *)realloc(*png, png_length_padded * sizeof(char));
	if (padded_png == NULL) {
		log_error("Could not resize PNG memory buffer");
		free(padded_png);
		return png_length;
	} else {
		*png = padded_png;
	}

	/*
	 * Every PNG ends with the sequence 00 00 00 00 I E N D ae 42 60 82,
	 * and so does the new one we are creating. To do this, copy the old
	 * end sequence to the new place, moved by the diff
	 * between header_length & png_length_padded
	 *
	 * Every chunk is at least 12 bytes long (if the content is empty).
	 * 4 bytes content length (00 00 00 00) = empty
	 * 4 bytes type (IEND)
	 * 0 bytes for content, because there is none
	 * 4 bytes crc (ae 42 60 82)
	 *
	 * Note: we continue to use padded_png instead of *png because they
	 * point to the same thing anyway and double pointers make my brain hurt.
	 */
	for (i = 12; i > 0; i--) {
		padded_png[png_length_padded - i] = padded_png[png_length - i];
	}

	/*
	 * copy the new header into the PNG buffer
	 */
	for (i = 0; i < header_length; i++) {
		padded_png[png_length - 12 + i] = head[i];
	}

	return png_length_padded;
}

/*
 * convert timeStruct to ISO 8601 as in http://www.w3.org/TR/NOTE-datetime
 * which is the standard time format for PNG Creation Time text chunks
 *
 * Time is always in UTC.
 *
 * This should conform to /\d{4}-[01]\d-[0-3]\dT[0-2]\d:[0-5]\d:[0-5]\d\.\d+([+-][0-2]\d:[0-5]\d|Z)/
 */
int dateStructToISO8601(timeStruct * time, char iso_date[25])
{
	int strl;
	// date is of the form YYYY-MM-DDThh:mm:ss.sssZ (ISO 8601)
	strl = sprintf(iso_date, "%04i-%02i-%02iT%02i:%02i:%02i.%03iZ",
		time->year,
		time->mon,
		time->day,
		time->hour,
		time->min,
		time->sec,
		time->milli
	);

	log_debug("iso_date %s", iso_date);

	return strl > 0 ? 1 : 0;
}

int createFilename(sConfigStruct * config, char *filename, int filenamelength, timeStruct *time, char *camname, char *filetype)
{
	int state = sprintf(filename,
		"%s%s_%04d_%02d_%02d-%02d_%02d_%02d_%03d_cam_%s.%s",
		config->cImagePath, config->cFileNamePrefix,
		time->year, time->mon, time->day, time->hour, time->min,
		time->sec, time->milli, camname, filetype);

	if (state > filenamelength) {
		log_error("The filename I was ask to generate is longer then allowed.");
		log_debug("Filename length is %i", state);
	}

	return state > 0 ? 0 : 1;
}
