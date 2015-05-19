#include<string.h>
#include<stdlib.h>
#include"configurations.h"
#include"log.h"

#include<phx_api.h>
#include<phx_os.h>

#define MAXBUF 1024


int structInit(sParameterStruct *sSO2Parameters, char identifier)
{
	sSO2Parameters->dImageCounter = 0;
	sSO2Parameters->dTriggerPulseWidth = 15;
	sSO2Parameters->dBufferlength = 1376256;
	sSO2Parameters->dHistMinInterval = 350;
	sSO2Parameters->dHistPercentage = 5;
	sSO2Parameters->dInterFrameDelay = 10;
	sSO2Parameters->fid = NULL;
	sSO2Parameters->identifier = identifier;
	return 0;
}

int configurations(sParameterStruct *sSO2Parameters)
{
	int		status = 0; /* status variable for return values */

	/* name of Configfile is hard coded maybe change this sometime */
	status = readConfig("configurations//SO2Config.conf", sSO2Parameters);
	if(status != 0) logError("readConfig(...) failed");

	/* load the default configurations for the framegrabber */
	status = camera_config(sSO2Parameters);
	if(status != 0)
	{
		logError( "configuring camera failed" );
		return status;
	}
	return status;
}

int readConfig(char *filename, sParameterStruct *sSO2Parameters)
{
	FILE *pFILE; /* filehandle for config file */
	char *lineBuf;/* buffer that holds the current line of the config file */
	char *delimeterBuf; /* buffer that holds the line after a specified delimeter */
	int n=1,i=0,delimIndex=0,linebreak=0, valueSize=0;
	int linenumber=1;
	double sizeOfImage = 2.7;
	int dTmp;
	char cTmp[MAXBUF]; /* a temporal buffer used when strings are read from the config file */
	char errbuff[MAXBUF]; /* a buffer to construct a proper error message */

	lineBuf = (char*) malloc(MAXBUF);

	pFILE = fopen(filename,"r");

	if (pFILE!=NULL)
	{
		while(fgets(lineBuf, MAXBUF, pFILE) != NULL)
		{
			/* skip lines which are marked as a commend */
			if(lineBuf[0] != '#')
			{
				/* search for corresponding strings */
				if (strstr(lineBuf,"HistogramMinInterval"))
				{
					delimeterBuf = strstr(lineBuf,"=");
					sSO2Parameters->dHistMinInterval = atoi(delimeterBuf+1);
				}

				else if (strstr(lineBuf,"HistogramPercentage"))
				{
					delimeterBuf = strstr(lineBuf,"=");
					sSO2Parameters->dHistPercentage = atoi(delimeterBuf+1);
				}

				else if ( strstr( lineBuf, "InterFrameDelay"))
				{
					delimeterBuf = strstr(lineBuf,"=");
					sSO2Parameters->dInterFrameDelay = atoi(delimeterBuf+1);
				}

				else if ( strstr( lineBuf, "TriggerPulseWidth" ) )
				{
					delimeterBuf = strstr(lineBuf,"=");
					sSO2Parameters->dTriggerPulseWidth = atoi(delimeterBuf+1);
				}

				else if ( strstr( lineBuf, "FixTime" ) )
				{
					delimeterBuf = strstr(lineBuf,"=");
					sSO2Parameters->dFixTime = atoi(delimeterBuf+1);
				}

				else if ( strstr( lineBuf, "ImageSize") )
				{
					delimeterBuf = strstr(lineBuf,"=");
					dTmp = atoi(delimeterBuf+1);
					sSO2Parameters->dfilesize = dTmp;
					sSO2Parameters->dImagesFile = (int)(dTmp/sizeOfImage);
				}

				else if ( strstr( lineBuf, "ExposureTime") )
				{
					delimeterBuf = strstr(lineBuf,"=");
					sSO2Parameters->dExposureTime = atoi(delimeterBuf+1);
				}
				else if(strstr(lineBuf,"FileNamePrefix"))
				{
					delimeterBuf = strstr(lineBuf,"=");
					if(delimeterBuf[1] == ' ')
						sprintf(cTmp,"%s",delimeterBuf+2);
					else
						sprintf(cTmp,"%s",delimeterBuf+1);

					/* remove LF */
					sprintf(sSO2Parameters->cFileNamePrefix,"%s",strtok(cTmp,"\n"));
				}
				else if(strstr(lineBuf,"ImagePath"))
				{
					delimeterBuf = strstr(lineBuf,"=");
					if(delimeterBuf[1] == ' ')
						sprintf(cTmp,"%s",delimeterBuf+2);
					else
						sprintf(cTmp,"%s",delimeterBuf+1);

					/* remove LF */
					sprintf(sSO2Parameters->cImagePath,"%s",strtok(cTmp,"\n"));
				}
			} //end if(lineBuf[0] != '#')
		linenumber++;
		} //end while(fgets(lineBuf, MAXBUF, pFILE) != NULL)
	fclose(pFILE);

	/* not an error but errbuff is used anyway */
	sprintf(errbuff,"Reading config file was successfull %d lines were read",linenumber);
	logMessage(errbuff);
	} //end if(pFILE!=NULL)
	else
	{
		sprintf(errbuff,"opening Configfile: %s failed!",filename);
		logError(errbuff);
		return 1;
	}

	return 0;
}
