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
char *getString(char *);
void print_help(char *);

/*
 * Set up a sParameterStruct structure with values taken from the
 * configuration
 */
void config_init_sParameterStruct(sParameterStruct * sSO2Parameters, sConfigStruct * config, char identifier)
{
	if (identifier == 'a') {
		sSO2Parameters->dExposureTime = config->dExposureTime_a;
	} else {
		sSO2Parameters->dExposureTime = config->dExposureTime_b;
	}
	sSO2Parameters->identifier = identifier;
	sSO2Parameters->timestampBefore = malloc(sizeof(timeStruct));
	sSO2Parameters->timestampAfter = malloc(sizeof(timeStruct));
	sSO2Parameters->dark = 0;
}

/*
 * inits a sConfigStruct with zeros or empty strings. This is useful to
 * be able to check if one of the values has been set later on
 */
void config_init_sConfigStruct(sConfigStruct * config)
{
	config->processing = -1;
	config->debug = -1;
	config->noofimages = -1;
	config->dBufferlength = -1;
	config->dExposureTime_a = -1;
	config->dExposureTime_b = -1;
	config->dImageCounter = -1;
	config->dInterFrameDelay = -1;
	config->cFileNamePrefix = "";
	config->cImagePath = "";
	config->dFixTime = -1;
	config->cConfigFileName = "";
	config->darkframeintervall = -1;
	config->spectrometer_calibrate_intervall = -1;
	config->spectroscopy_roi_upper = -1;
	config->spectroscopy_roi_lower = -1;
	config->filterwheel_device = "";
	config->comm_port = -1;
	config->enableWebcam = -1;
	config->enableSpectroscopy = -1;
	config->rotate_a = -1;
	config->rotate_b = -1;
	config->rotate_webcam = -1;
}

/*
 * http://courses.cms.caltech.edu/cs11/material/general/usage.html
 */
void print_help(char *name)
{
	fprintf(stderr, "Usage %s [--noprocessing] [--png-only] [--debug] [--noofimages n]\n", name);
	fprintf(stderr, "      [--configfile filename] [--imagepath folder] [--port portno]\n");
	fprintf(stderr, "      [--disableWebcam] [--disableSpectroscopy]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "SO2-Camera Control Software \n");
	fprintf(stderr, "\n");
	fprintf(stderr, "This software comes with ABSOLUTELY NO WARRANTY. This is free software and you\n");
	fprintf(stderr, "are welcome to modify and redistribute it under certain conditions.\n");
	fprintf(stderr, "See the MIT Licence for details. \n");
	fprintf(stderr, "\n");
	fprintf(stderr, "   --help, -h                    Print this usage information and exit\n");
	fprintf(stderr, "   --noprocessing                Skip processing as much as possible and only save raw images\n");
	fprintf(stderr, "   --png-only                    Skip saving of raw files\n");
	fprintf(stderr, "   --debug                       Print debug output\n");
	fprintf(stderr, "   --noofimages n                Only save n UV image sets and exit\n");
	fprintf(stderr, "   --configfile /path/file.conf  Load config file from path. If not set config files are searched for at the usual places\n");
	fprintf(stderr, "   --imagepath /path/outfolder   Save images and logs in path\n");
	fprintf(stderr, "   --port portno                 Set port for liveview. Default: 7009\n");
	fprintf(stderr, "   --disableWebcam               Disable processing and saving of webcam images\n");
	fprintf(stderr, "   --disableSpectroscopy         Disable processing and saving of spectra\n");

	exit(0);
}

/*
 * handle command line properties and translate into config properties
 */
int config_process_cli_arguments(int argc, char *argv[], sConfigStruct * config)
{
	int i;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			print_help(argv[0]);
		} else if (strcmp(argv[i], "--speedy-gonzales") == 0 || strcmp(argv[i], "--noprocessing") == 0) {
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
		} else if (strcmp(argv[i], "--imagepath") == 0 && argv[i + 1]) {
			config->cImagePath = (char *)calloc(sizeof(char), strlen(argv[i + 1]) + 1);
			strcpy(config->cImagePath, argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "--port") == 0 && argv[i + 1]) {
			config->comm_port = strtol(argv[i + 1], NULL, 10);
			i++;
		} else if (strcmp(argv[i], "--disableWebcam") == 0) {
			config->enableWebcam = 0;
		} else if (strcmp(argv[i], "--disableSpectroscopy") == 0) {
			config->enableSpectroscopy = 0;
		} else {
			log_error("unknown command line option \"%s\"", argv[i]);
			print_help(argv[0]);
			return 1;
		}
	}
	return 0;
}

/*
 * Find, load and parse a config file.
 *
 * The first config file that is found is used, it is searched for (in
 * this order), at:
 * - config->cConfigFileName, if set (e.g. from the cli)
 * - from the default source path (./configurations/so2-camera.conf)
 * - home path
 *   - XDG_CONFIG_HOME/so2-camera/so2-camera.conf (linux only)
 *   - PROGRAMDATA/so2-camera/config/so2-camera.conf (windows only)
 * - system path
 *   - XDG_CONFIG_DIRS/so2-camera/so2-camera.conf (linux only)
 *
 * After a config file has been found, config->cConfigFileName is set
 * to that path.
 *
 * The config file is parsed as a simple key=value syntax. Lines
 * starting with '#' are ignored, as are unknown keys.
 *
 */
int config_load_configfile(sConfigStruct * config)
{
	FILE *pFILE;		/* filehandle for config file */
	char lineBuf[MAXBUF];	/* buffer that holds the current line of the config file */
	char *delimeterBuf;	/* buffer that holds the line after a specified delimeter */
	int linenumber = 0;

	char user_conffile[256] = "";
	char system_conffile[256] = "";
	char *token;
	char *xdg_home_configdir = getenv("XDG_CONFIG_HOME");
	char *home = getenv("HOME");
	char *programdata = getenv("PROGRAMDATA");
	char *xdg_configdir = getenv("XDG_CONFIG_DIRS");

	/* source path */
	if (!strlen(config->cConfigFileName)) {
		config->cConfigFileName = "./configurations/so2-camera.conf";
	}

	/* home path */
	if (xdg_home_configdir) {	/* linux only */
		sprintf(user_conffile, "%s/so2-camera/so2-camera.conf", xdg_home_configdir);
	} else if (programdata) {	/* windows only */
		sprintf(user_conffile, "%s/so2-camera/config/so2-camera.conf", programdata);
	} else if (home) {
		sprintf(user_conffile, "%s/.config/so2-camera/so2-camera.conf", home);
	}

	if ((pFILE = fopen(config->cConfigFileName, "r"))) {
		/* source path */
		log_debug("read configfile from source path: %s", config->cConfigFileName);
	} else if ((pFILE = fopen(user_conffile, "r"))) {
		/* user path */
		log_debug("read configfile from home path: %s", user_conffile);
		config->cConfigFileName = user_conffile;
	} else {
		/* system path */
#ifdef POSIX
		while ((token = strsep(&xdg_configdir, ":"))) {
			sprintf(system_conffile, "%s/so2-camera/so2-camera.conf", token);
			log_debug("search for config file at %s", system_conffile);
			if ((pFILE = fopen(system_conffile, "r"))) {
				log_debug("read configfile from system path: %s", system_conffile);
				config->cConfigFileName = system_conffile;
				break;
			}
		}
#endif
	}

	if (!pFILE) {
		log_error("opening config file failed!");
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
		if (strstr(lineBuf, "InterFrameDelay") && config->dInterFrameDelay == -1) {
			config->dInterFrameDelay = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "FixTime") && config->dFixTime == -1) {
			config->dFixTime = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "ExposureTime_a") && config->dExposureTime_a < 0) {
			config->dExposureTime_a = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "ExposureTime_b") && config->dExposureTime_b < 0) {
			config->dExposureTime_b = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "FileNamePrefix") && !strlen(config->cFileNamePrefix)) {
			config->cFileNamePrefix = getString(delimeterBuf);
		} else if (strstr(lineBuf, "ImagePath") && !strlen(config->cImagePath)) {
			config->cImagePath = getString(delimeterBuf);
		} else if (strstr(lineBuf, "filterwheel_device") && !strlen(config->filterwheel_device)) {
			config->filterwheel_device = getString(delimeterBuf);
		} else if (strstr(lineBuf, "processing") && config->processing == -1) {
			config->processing = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "darkframeintervall") && config->darkframeintervall == -1) {
			config->darkframeintervall = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "spectrometer_calibrate_intervall") && config->spectrometer_calibrate_intervall == -1) {
			config->spectrometer_calibrate_intervall = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "spectroscopy_roi_upper") && config->spectroscopy_roi_upper == -1) {
			config->spectroscopy_roi_upper = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "spectroscopy_roi_lower") && config->spectroscopy_roi_lower == -1) {
			config->spectroscopy_roi_lower = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "spectrometer_shutter_device")) {
			config->spectrometer_shutter_device = getString(delimeterBuf);
		} else if (strstr(lineBuf, "spectrometer_shutter_channel")) {
			config->spectrometer_shutter_channel = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "port") && config->comm_port == -1) {
			config->comm_port = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "enableSpectroscopy") && config->enableSpectroscopy == -1) {
			config->enableSpectroscopy = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "enableWebcam") && config->enableWebcam == -1) {
			config->enableWebcam = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "rotate_a") && config->rotate_a == -1) {
			config->rotate_a = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "rotate_b") && config->rotate_b == -1) {
			config->rotate_b = atoi(delimeterBuf + 1);
		} else if (strstr(lineBuf, "rotate_webcam") && config->rotate_webcam == -1) {
			config->rotate_webcam = atoi(delimeterBuf + 1);
		//~ } else {
			//~ log_message("not understood: %s  rota: %i", lineBuf, strstr(lineBuf, "rotate_a"));
		}
	}

	fclose(pFILE);

	log_message("Reading config file was successfull, %d lines were read from %s", linenumber, config->cConfigFileName);

	return 0;
}

/*
 * load default config values.
 */
void config_load_default(sConfigStruct * config)
{
	config->dInterFrameDelay   = config->dInterFrameDelay != -1 ? config->dInterFrameDelay : 10;
	config->dBufferlength      = config->dBufferlength    != -1 ? config->dBufferlength    : 1376256;
	config->debug              = config->debug            != -1 ? config->debug            : 0;
	config->comm_port          = config->comm_port        != -1 ? config->comm_port        : 7003;

	config->filterwheel_device = strlen(config->filterwheel_device) ? config->filterwheel_device : "/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_AI02PNA1-if00-port0";
	config->cImagePath         = strlen(config->cImagePath)         ? config->cImagePath         : "";
	config->cFileNamePrefix    = strlen(config->cFileNamePrefix)    ? config->cFileNamePrefix    : "";
	config->createsubfolders   = 1;
	config->enableSpectroscopy = config->enableSpectroscopy != -1   ? config->enableSpectroscopy : 1;
	config->enableWebcam       = config->enableWebcam != -1         ? config->enableWebcam       : 1;
}

char *getString(char *source)
{
	char *tmp;
	size_t l = strspn(source, " =");
	tmp = source + l;	/* remove leading whitespace */
	tmp = strtok(tmp, "\n");	/* remove LF */
	return strdup(tmp);
}
