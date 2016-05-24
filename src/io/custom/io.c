/*
 * This file implements the file output system, responsible for writing
 * the gathered data to disk and to process the data into reasonable
 * file formats. This could also do i.e. packaging files into a .tar
 * file.
 *
 * Currently, there are three write modes:
 * - dumb mode, the camera data and all relevent headers are dumped to
 *   files
 * - png mode, the camera data is converted to pngs, and relevant
 *   headers are written to ancillary text chunks
 * - default: both
 *
 * PNG mode is a lot slower, but produces smaller files which can be
 * easily viewed and used for further processing.
 * Dumb mode requires less processing and does not alter the original
 * data, but requires additional work for viewing and evalution.
 *
 */
#include<stdio.h>
#include<time.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<string.h>
#include<opencv/cv.h>
#include<opencv/highgui.h>
#include "common.h"
#include "configurations.h"
#include "../io.h"
#include "make_png_header.c"

/* local prototypes */
int createFilename(sConfigStruct * config, char * filename, int filenamelength, timeStruct *time, char *camname, char * filetype);
IplImage *bufferToImage(short *buffer);
int dateStructToISO8601(timeStruct * time, char *iso_date);
int insertValue(char **png, char *name, float value, int png_length);
int insertStringValue(char **png, char *name, char *value, int png_length);
int insertHeader(char **png, char *name, char *content, int png_length);
int insertHeaders(char **png, sParameterStruct * sSO2Parameters, sConfigStruct * config, int png_length);
int io_writeImage(sParameterStruct * sSO2Parameters, sConfigStruct * config);
int io_writeDump(sParameterStruct * sSO2Parameters, sConfigStruct * config);

/*
 * `io_init`
 * Initialize the IO functionality. Currently, this does nothing, but this could be the place to
 * set up folders and start tar files.
 */
int io_init(sConfigStruct * config)
{
	static time_t time_ptr;
	static struct tm now;
	time(&time_ptr);
	now = *gmtime(&time_ptr);

	struct stat st = {0};
	if (stat(config->cImagePath, &st) == -1) {
		mkdir(config->cImagePath, 0700);
	}

	sprintf(config->cImagePath, "%s/%04d-%02d-%02d_%02d_%02d/",
		config->cImagePath, now.tm_year + 1900, now.tm_mon,
		now.tm_mday, now.tm_hour, now.tm_min);

	if (stat(config->cImagePath, &st) == -1) {
		mkdir(config->cImagePath, 0700);
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

int io_writeWebcam(sWebCamStruct * webcam, sConfigStruct * config)
{
	int state;
	char filename[512];
	char iso_date[25];
	int filenamelength = 512;
	FILE * f;

	state = createFilename(config, filename, filenamelength, webcam->timestampBefore, "webcam", "raw");
	if (state) {
		log_error("could not create webcam filename");
		return state;
	}

	f = fopen(filename, "wb");

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
	if (f){
		fprintf(f, "bufferSize %i\n", webcam->bufferSize);
		dateStructToISO8601(webcam->timestampBefore, iso_date);
		fprintf(f, "timestampBefore %s\n", iso_date);
		dateStructToISO8601(webcam->timestampAfter, iso_date);
		fprintf(f, "timestampAfter %s\n", iso_date);
	}
	fclose(f);


	return 0;
}

int io_spectrum_save_calib(sSpectrometerStruct * spectro, sConfigStruct * config)
{
return 0;
	FILE * pFile;
	int i;
	pFile = fopen("dark-current.dat", "wt");
	if (pFile){
		for(i = 0; i < spectro->spectrum_length; i++){
			fprintf(pFile, "%f %f \n", spectro->wavelengths[i], spectro->dark_current[i]);
		}
	}
	pFile = fopen("electronic-offset.dat", "wt");
	if (pFile){
		for(i = 0; i < spectro->spectrum_length; i++){
			fprintf(pFile, "%f %f \n", spectro->wavelengths[i], spectro->electronic_offset[i]);
		}
	}
}

int io_spectrum_save(sSpectrometerStruct * spectro, sConfigStruct * config)
{
return 0;
	FILE * f;
	int i;
	char iso_date[25];
	char filename[512];
	int filenamelength = 512;

	createFilename(config, filename, filenamelength, spectro->timestampBefore, "spectrum", "dat");

	/* save spectrum */
	f = fopen(filename, "wt");
	if (f){
		for(i = 0; i < spectro->spectrum_length; i++){
			fprintf(f, "%f %f \n", spectro->wavelengths[i], spectro->dark_current[i]);
		}
	}
	fclose(f);

	/* save meta */
	createFilename(config, filename, filenamelength, time, "spectrum_meta", "txt");
	f = fopen(filename, "wt");
	if (f){
		fprintf(f, "max %f\n", spectro->max);
		fprintf(f, "integration_time_micros %i\n", spectro->integration_time_micros);
		fprintf(f, "spectrum_length %i\n", spectro->spectrum_length);
		dateStructToISO8601(spectro->timestampBefore, iso_date);
		fprintf(f, "timestampBefore %s\n", iso_date);
		dateStructToISO8601(spectro->timestampAfter, iso_date);
		fprintf(f, "timestampAfter %s\n", iso_date);
	}
	fclose(f);
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
	FILE *fp;
	char headerfile[512];
	int headerfilelength = 512;
	char rawfile[512];
	int rawfilelength = 512;
	int fwriteReturn;
	int state = 0;
	char iso_date[25];

	/* generate filenames */
	char id = sSO2Parameters->identifier;
	timeStruct *time = sSO2Parameters->timestampBefore;	// Datum und Uhrzeit

	/* identify Camera for filename Prefix */
	char *camname = sSO2Parameters->dark ? (id == 'a' ? "top_dark" : "bot_dark") : (id == 'a' ? "top" : "bot");

	state = createFilename(config, headerfile, headerfilelength, time, camname,  "txt");
	if(state){
		log_error("could not create txt filename");
	}

	state = createFilename(config, rawfile, rawfilelength, time, camname, "raw");
	if (state) {
		log_error("could not create txt filename");
	}

	/* Open a new file for the image (writeable, binary) */
	imageFile = fopen(rawfile, "wb");
	if (imageFile != NULL) {
		fwriteReturn = fwrite(sSO2Parameters->stBuffer, 1, config->dBufferlength * 2, imageFile);
		if(fwriteReturn != config->dBufferlength * 2){
			log_debug("could not write raw file %i != %i", config->dBufferlength, fwriteReturn);
			log_error("could not write raw file");
		}
		fclose(imageFile);
	} else {
		log_error("could not open raw file");
	}

	/* write a text file containing header information */
	fp = fopen(headerfile, "ab");
	if (fp != NULL) {
		fprintf(fp, "dBufferlength %i\n", config->dBufferlength);
		fprintf(fp, "dHistMinInterval %i\n", config->dHistMinInterval);
		fprintf(fp, "dHistPercentage %i\n", config->dHistPercentage);
		fprintf(fp, "dDarkCurrent %i\n", (int)sSO2Parameters->dDarkCurrent);
		fprintf(fp, "dImageCounter %i\n", (int)config->dImageCounter);
		fprintf(fp, "dInterFrameDelay %i\n", (int)config->dInterFrameDelay);
		fprintf(fp, "dTriggerPulseWidth %i\n", (int)sSO2Parameters->dTriggerPulseWidth);
		fprintf(fp, "dExposureTime %f\n", sSO2Parameters->dExposureTime);
		fprintf(fp, "cConfigFileName %s\n", config->cConfigFileName);
		fprintf(fp, "cFileNamePrefix %s\n", config->cFileNamePrefix);
		fprintf(fp, "cImagePath %s\n", config->cImagePath);
		fprintf(fp, "dFixTime %i\n", config->dFixTime);
		dateStructToISO8601(sSO2Parameters->timestampBefore, iso_date);
		fprintf(fp, "timestampBefore %s\n", iso_date);
		dateStructToISO8601(sSO2Parameters->timestampAfter, iso_date);
		fprintf(fp, "timestampAfter %s\n", iso_date);

		fclose(fp);
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

	stBuffer = sSO2Parameters->stBuffer;

	/* generate filenames */
	char id = sSO2Parameters->identifier;
	timeStruct *time = sSO2Parameters->timestampBefore;	// Datum und Uhrzeit

	/* identify Camera for filename Prefix */
	char *camname = sSO2Parameters->dark ? (id == 'a' ? "top_dark" : "bot_dark") : (id == 'a' ? "top" : "bot");

	state = createFilename(config, filename, filenamelength, time, camname,  "png");
	if(state){
		log_error("could not create txt filename");
	}

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

int insertHeaders(char **png, sParameterStruct *sSO2Parameters, sConfigStruct *config, int png_length){
	char iso_date[25];
	dateStructToISO8601(sSO2Parameters->timestampBefore, iso_date);
	png_length = insertHeader(png, "Creation Time ",    iso_date, png_length);
	png_length = insertStringValue(png, "timestampBefore",    iso_date, png_length);

	dateStructToISO8601(sSO2Parameters->timestampAfter, iso_date);
	png_length = insertStringValue(png, "timestampAfter",     iso_date, png_length);

	png_length = insertValue(png, "dBufferlength",      (float)config->dBufferlength,      png_length);
	png_length = insertValue(png, "dHistMinInterval",   (float)config->dHistMinInterval,   png_length);
	png_length = insertValue(png, "dHistPercentage",    (float)config->dHistPercentage,    png_length);
	png_length = insertValue(png, "dDarkCurrent",       (float)sSO2Parameters->dDarkCurrent,       png_length);
	png_length = insertValue(png, "dImageCounter",      (float)config->dImageCounter,      png_length);
	png_length = insertValue(png, "dInterFrameDelay",   (float)config->dInterFrameDelay,   png_length);
	png_length = insertValue(png, "dTriggerPulseWidth", (float)sSO2Parameters->dTriggerPulseWidth, png_length);
	png_length = insertValue(png, "dExposureTime",      (float)sSO2Parameters->dExposureTime,      png_length);
	png_length = insertValue(png, "dFixTime",           (float)config->dFixTime,           png_length);

	return png_length;
}

int insertValue(char **png, char *name, float value, int png_length)
{
	char text[200];
	sprintf(text, "%s: %f", name, value);
	return insertHeader(png, "Comment ", text, png_length);
}

int insertStringValue(char **png, char *name, char * value, int png_length)
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

int createFilename(sConfigStruct * config, char * filename, int filenamelength, timeStruct *time, char *camname, char * filetype)
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
