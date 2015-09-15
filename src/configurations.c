#include<string.h>
#include<stdlib.h>
#include "configurations.h"
#include "log.h"
#include "camera.h"

#define MAXBUF 1024

/* local prototypes*/
static int readConfig(char *filename, sConfigStruct * config);
static void getString(char * target, char * source);

int structInit(sParameterStruct *sSO2Parameters, sConfigStruct *config, char identifier)
{
	sSO2Parameters->dExposureTime = config->dExposureTime;
	sSO2Parameters->dTriggerPulseWidth = config->dTriggerPulseWidth;
	sSO2Parameters->identifier = identifier;
	sSO2Parameters->timestampBefore = malloc(sizeof(timeStruct));
	sSO2Parameters->dark = 0;

	return 0;
}

int process_cli_arguments(int argc, char *argv[], sConfigStruct * config)
{
	int i;
	char errstr[512];

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--speedy-gonzales") == 0
		    || strcmp(argv[i], "--noprocessing") == 0) {
			config->processing = 1;
		} else if (strcmp(argv[i], "--png-only") == 0) {
			config->processing = 2;
		} else if (strcmp(argv[i], "--debug") == 0) {
			config->debug = 1;
		} else if (strcmp(argv[i], "--noofimages") == 0 && argv[i + 1]) {
			config->noofimages = strtol(argv[i + 1], NULL, 10);
			i++;
		} else {
			sprintf(errstr, "unknown command line option \"%s\"", argv[i]);
			log_error(errstr);
			return 1;
		}
	}
	return 0;
}

static int readConfig(char *filename, sConfigStruct * config)
{
	FILE *pFILE;          /* filehandle for config file */
	char lineBuf[MAXBUF]; /* buffer that holds the current line of the config file */
	char *delimeterBuf;   /* buffer that holds the line after a specified delimeter */
	int linenumber = 0;
	char errbuff[MAXBUF]; /* a buffer to construct a proper error message */

	pFILE = fopen(filename, "r");

	if (pFILE == NULL) {
		sprintf(errbuff, "opening Configfile: %s failed!", filename);
		log_error(errbuff);
		return 1;
	}

	while (fgets(lineBuf, MAXBUF, pFILE) != NULL) {
		linenumber++;
		/* skip lines which are marked as a commend */
		if (lineBuf[0] == '#') {
			continue;
		}
		delimeterBuf = strstr(lineBuf, "=");

		/* search for corresponding strings */
		if (strstr(lineBuf, "HistogramMinInterval")) {
			config->dHistMinInterval = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "HistogramPercentage")) {
			config->dHistPercentage = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "InterFrameDelay")) {
			config->dInterFrameDelay = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "TriggerPulseWidth")) {
			config->dTriggerPulseWidth = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "FixTime")) {
			config->dFixTime = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "ExposureTime")) {
			config->dExposureTime = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "FileNamePrefix")) {
			getString(config->cFileNamePrefix, delimeterBuf);
		} else if (strstr(lineBuf, "ImagePath")) {
			getString(config->cImagePath, delimeterBuf);
		} else if (strstr(lineBuf, "filterwheel_device")) {
			getString(config->filterwheel_device, delimeterBuf);
		} else if (strstr(lineBuf, "processing")) {
			config->processing = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "darkframeintervall")) {
			config->darkframeintervall = atoi(delimeterBuf + 1);
		}
	}

	fclose(pFILE);

	/* not an error but errbuff is used anyway */
	sprintf(errbuff, "Reading config file was successfull, %d lines were read", linenumber);
	log_message(errbuff);

	return 0;
}

static void getString(char * target, char * source)
{
	char cTmp[MAXBUF];
	if (source[1] == ' ')
		sprintf(cTmp, "%s", source + 2);
	else
		sprintf(cTmp, "%s", source + 1);

	/* remove LF */
	sprintf(target, "%s", strtok(cTmp, "\n"));
}

int load_config(char *filename, sConfigStruct * config)
{
	return readConfig(filename, config);
}
