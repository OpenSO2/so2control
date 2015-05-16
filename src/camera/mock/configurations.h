#ifndef _CONFIGURATIONS_
#define _CONFIGURATIONS_

/**********************************************************
 *
 *	Header-File with miscellaneous configuration functions
 *	for Program, Camera and Framegrabber
 *
 **********************************************************/

/******************************
 *   HEADER INCLUDES
 ******************************/

#include<stdio.h>
#include<math.h>
#include"common.h"



/******************************
 *   MACROS
 ******************************/

#define MAX_STRING_LENGTH	256
#define _PHX_LINE_SIZE		256
#define szDefaultCmdAppend	"\r"
/******************************
 *   Structures
 ******************************/
typedef struct
{
/* Camera parameters */
	/* Number of pixels in 1 Image */
	int			dBufferlength;
	/* when controling the exposure time a histogram is made
	 * from 1 image. the lower part is evaluated for underexposure
	 * this give the evaluated invervall.
	 */
	int			dHistMinInterval;
	/* when controling the exposure time a histogram is made
	 * from 1 image. This value gibs the percentage of pixels
	 * that are aloud to be under or overexposed.
	 */
	int			dHistPercentage;
	/* not used right now. should contain a value for image noise */
	int		dDarkCurrent;
	/* number of images take */
	int			dImageCounter;
	/* delay between two frames in [ms] */
	int		dInterFrameDelay;
	/* length of the triggerpulse in [ms] */
	int		dTriggerPulseWidth;
	/* contains the Exposuretime in [ms] */
	double		dExposureTime;
	/* contains the name of a Config-file */
	char		cConfigFileName[MAX_STRING_LENGTH];
	/* not used right now, should contains a prefix for all images */
	char		cFileNamePrefix[MAX_STRING_LENGTH];
	/* path to image diretory */
	char		cImagePath[MAX_STRING_LENGTH];
	/* A handle to identify the camera */
	tHandle		hCamera;
	/* A status variable inherited from the SDK */
	//~ etStat		eStat;
	/* A switch to set the exposuretime fix to the value given in the config file */
	int			dFixTime;
	/* File identifier for current images */
	FILE*		fid;
	/* Size of each file */
	int			dfilesize;
	/* Images per file. Calculated from size of file*/
	int			dImagesFile;

	/* ~Callback stuff~ */

	/* Event Flags */
	volatile tFlag	fBufferReady;
	volatile int	dBufferReadyCount;

	/* Control Flags */
	volatile tFlag fFifoOverFlow;

	/* Camera identifier */
	char identifier;
}sParameterStruct;


 /******************************
 *   FUNCTIONS
 ******************************/
int readConfig				(char *filename, sParameterStruct *sSO2Parameters);
int configurations			(sParameterStruct *sSO2Parameters, char identifier);
int structInit				(sParameterStruct *sSO2Parameters, char identifier);
int triggerConfig			(sParameterStruct *sSO2Parameters);
int defaultConfig			(sParameterStruct *sSO2Parameters);
int defaultCameraConfig		(sParameterStruct *sSO2Parameters);
int sendMessage				(tHandle hCamera, char * inputBuffer);
#endif