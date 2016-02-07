/*
 * This file implements the configuration handling facilities.
 * Configuration values are taken from three sources:
 * - from the command line arguments, eg. --noofimages N
 * - from an config file
 * - internal presets
 *
 * All function pertaining configuration are prefixed with "config_".
 */
#include<string.h>
#include<stdlib.h>
#include "configurations.h"
#include "log.h"

#define MAXBUF 1024

/* local prototypes*/
char * getString(char * source);

/*
 * Set up a sParameterStruct structure with values taken from the
 * configuration
 */
void config_init_sParameterStruct(sParameterStruct *sSO2Parameters, sConfigStruct *config, char identifier)
{
	sSO2Parameters->dExposureTime = config->dExposureTime;
	sSO2Parameters->dTriggerPulseWidth = config->dTriggerPulseWidth;
	sSO2Parameters->identifier = identifier;
	sSO2Parameters->timestampBefore = malloc(sizeof(timeStruct));
	sSO2Parameters->dark = 0;
}

/*
 * inits a sConfigStruct with zeros or empty strings. This is useful to
 * be able to check if one of the values has been set later on
 */
void config_init_sConfigStruct(sConfigStruct *config){
	config->processing = -1;
	config->debug = -1;
	config->noofimages = -1;
	config->dBufferlength = -1;
	config->dHistMinInterval = -1;
	config->dHistPercentage = -1;
	config->dTriggerPulseWidth = 0;
	config->dExposureTime = -1;
	config->dImageCounter = -1;
	config->dInterFrameDelay = -1;
	config->cFileNamePrefix = "";
	config->cImagePath = "";
	config->dFixTime = -1;
	config->cConfigFileName = "";
	config->darkframeintervall = -1;
	config->filterwheel_device = "";
}

/*
 * handle command line properties and translate into config properties
 */
int config_process_cli_arguments(int argc, char *argv[], sConfigStruct * config)
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
		} else if (strcmp(argv[i], "--configfile") == 0 && argv[i + 1]) {
			config->cConfigFileName = argv[i + 1];
			i++;
		} else {
			sprintf(errstr, "unknown command line option \"%s\"", argv[i]);
			log_error(errstr);
			return 1;
		}
	}
	return 0;
}

/*
 * Load and parse the config file at config->cConfigFileName or
 * configurations/SO2Config.conf
 */
int config_load_configfile(sConfigStruct * config)
{
	FILE *pFILE;          /* filehandle for config file */
	char lineBuf[MAXBUF]; /* buffer that holds the current line of the config file */
	char *delimeterBuf;   /* buffer that holds the line after a specified delimeter */
	int linenumber = 0;
	char errbuff[MAXBUF]; /* a buffer to construct a proper error message */

	if (!strlen(config->cConfigFileName)){
		config->cConfigFileName = "configurations/SO2Config.conf";
	}

	pFILE = fopen(config->cConfigFileName, "r");

	if (pFILE == NULL) {
		sprintf(errbuff, "opening Configfile: %s failed!", config->cConfigFileName);
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
		if (strstr(lineBuf, "HistogramMinInterval") && config->dHistMinInterval != -1) {
			config->dHistMinInterval = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "HistogramPercentage") && config->dHistPercentage != -1) {
			config->dHistPercentage = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "InterFrameDelay") && config->dInterFrameDelay != -1) {
			config->dInterFrameDelay = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "TriggerPulseWidth") && config->dTriggerPulseWidth != 0) {
			config->dTriggerPulseWidth = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "FixTime") && config->dFixTime != -1) {
			config->dFixTime = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "ExposureTime") && config->dExposureTime < 0) {
			config->dExposureTime = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "FileNamePrefix") && !strlen(config->cFileNamePrefix)) {
			config->cFileNamePrefix = getString(delimeterBuf);
		} else if (strstr(lineBuf, "ImagePath") && !strlen(config->cImagePath)) {
			config->cImagePath = getString(delimeterBuf);
		} else if (strstr(lineBuf, "filterwheel_device") && !strlen(config->filterwheel_device)) {
			config->filterwheel_device = getString(delimeterBuf);
		} else if (strstr(lineBuf, "processing") && config->processing != -1) {
			config->processing = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "darkframeintervall") && config->darkframeintervall != -1) {
			config->darkframeintervall = atoi(delimeterBuf + 1);
		}
	}

	fclose(pFILE);

	/* not an error but errbuff is used anyway */
	sprintf(errbuff, "Reading config file was successfull, %d lines were read", linenumber);
	log_message(errbuff);

	return 0;
}

/*
 * load default config values.
 */
void config_load_default(sConfigStruct *config)
{
	config->dHistMinInterval   = config->dHistMinInterval != -1 ? config->dHistMinInterval : 350;
	config->dHistPercentage    = config->dHistPercentage  != -1 ? config->dHistPercentage  : 5;
	config->dInterFrameDelay   = config->dInterFrameDelay != -1 ? config->dInterFrameDelay : 10;
	config->dBufferlength      = config->dBufferlength    != -1 ? config->dBufferlength    : 1376256;
	config->debug              = config->debug            != -1 ? config->debug            : 0;

	config->filterwheel_device = strlen(config->filterwheel_device) ? config->filterwheel_device : "/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_AI02PNA1-if00-port0";
	config->cImagePath         = strlen(config->cImagePath)         ? config->cImagePath         : "";
	config->cFileNamePrefix    = strlen(config->cFileNamePrefix)    ? config->cFileNamePrefix    : "";
}

char * getString(char * source)
{
	char * tmp;
	size_t l = strspn(source, " =");
	tmp = source + l; /* remove leading whitespace */
	tmp = strtok(tmp, "\n");/* remove LF */
	return strdup(tmp);
}
