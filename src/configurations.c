#include<string.h>
#include<stdlib.h>
#include "configurations.h"
#include "log.h"
#include "camera.h"

#if defined(PHX)
#include<phx_api.h>
#include<phx_os.h>
#endif

#define MAXBUF 1024

/* local prototypes*/
int readConfig(char *filename, sConfigStruct * config);

int structInit(sParameterStruct * sSO2Parameters, sConfigStruct * config, char identifier)
{
	sSO2Parameters->dTriggerPulseWidth = config->dTriggerPulseWidth;
	sSO2Parameters->identifier = identifier;
	sSO2Parameters->timestampBefore = malloc(sizeof(timeStruct));

	return 0;
}

int process_cli_arguments(int argc, char * argv[], sConfigStruct * config)
{
	int i;
	char errstr[512];

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--speedy-gonzales") == 0
		    || strcmp(argv[i], "--noprocessing") == 0) {
			config->processing = 0;
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

int readConfig(char *filename, sConfigStruct * config)
{
	FILE *pFILE;		/* filehandle for config file */
	char *lineBuf;		/* buffer that holds the current line of the config file */
	char *delimeterBuf;	/* buffer that holds the line after a specified delimeter */
	int linenumber = 1;
	char cTmp[MAXBUF];	/* a temporal buffer used when strings are read from the config file */
	char errbuff[MAXBUF];	/* a buffer to construct a proper error message */

	lineBuf = (char *)malloc(MAXBUF);

	pFILE = fopen(filename, "r");

	if (pFILE == NULL) {
		sprintf(errbuff, "opening Configfile: %s failed!", filename);
		log_error(errbuff);
		return 1;
	}

	while (fgets(lineBuf, MAXBUF, pFILE) != NULL) {
		/* skip lines which are marked as a commend */
		if (lineBuf[0] != '#') {
			delimeterBuf = strstr(lineBuf, "=");

			/* search for corresponding strings */
			if (strstr(lineBuf, "HistogramMinInterval")) {
				config->dHistMinInterval = atoi(delimeterBuf + 1);
			}

			else if (strstr(lineBuf, "HistogramPercentage")) {
				config->dHistPercentage = atoi(delimeterBuf + 1);
			}

			else if (strstr(lineBuf, "InterFrameDelay")) {
				config->dInterFrameDelay = atoi(delimeterBuf + 1);
			}

			else if (strstr(lineBuf, "TriggerPulseWidth")) {
				config->dTriggerPulseWidth = atoi(delimeterBuf + 1);
			}

			else if (strstr(lineBuf, "FixTime")) {
				config->dFixTime = atoi(delimeterBuf + 1);
			}

			else if (strstr(lineBuf, "ExposureTime")) {
				config->dExposureTime = atoi(delimeterBuf + 1);
			}

			else if (strstr(lineBuf, "FileNamePrefix")) {
				if (delimeterBuf[1] == ' ')
					sprintf(cTmp, "%s", delimeterBuf + 2);
				else
					sprintf(cTmp, "%s", delimeterBuf + 1);

				/* remove LF */
				sprintf(config->cFileNamePrefix,
					"%s", strtok(cTmp, "\n"));
			}

			else if (strstr(lineBuf, "ImagePath")) {
				if (delimeterBuf[1] == ' ')
					sprintf(cTmp, "%s", delimeterBuf + 2);
				else
					sprintf(cTmp, "%s", delimeterBuf + 1);

				/* remove LF */
				sprintf(config->cImagePath,
					"%s", strtok(cTmp, "\n"));
			}
		} /* end if(lineBuf[0] != '#') */
		linenumber++;
	} /* end while(fgets(lineBuf, MAXBUF, pFILE) != NULL) */
	fclose(pFILE);

	/* not an error but errbuff is used anyway */
	sprintf(errbuff,
		"Reading config file was successfull %d lines were read",
		linenumber);
	log_message(errbuff);

	return 0;
}

int load_config(char * filename, sConfigStruct * config)
{
	return readConfig(filename, config);
}

