#include"configurations.h"
#include"messages.h"
#include"imageCreation.h"
#include"exposureTimeControl.h"
#include"log.h"
/* explanation of prefixes:
 * d = Integer (ui32) (int)
 * e = Status-Value (eStat) or Parameter-Value (etParamValue)
 * f = Flag (tFlag)
 * s = Structurs (struct)
 */

int main( int argc, char* argv[] )
{
	/* definition of basic variables */
	sParameterStruct sParameters_A;
	sParameterStruct sParameters_B;
	int              state;
//	etParamValue     eParamValue;

	/* print welcome message in terminal */
	printOpening();

	/* Initialise control flag structure */
#


	
	
	/* Initialise parameter structures */
	memset( &sParameters_A, 0, sizeof(sParameterStruct ));
	memset( &sParameters_B, 0, sizeof(sParameterStruct ));
	
	/* Load the framegrabber with the phoenix configuration file. The function returns the necessary camera handles */
	state = PHX_CameraConfigLoad( &sParameters_A->hCamera, "configurations//c8484.pcf", (etCamConfigLoad)PHX_BOARD_AUTO | PHX_DIGITAL | PHX_CHANNEL_A | PHX_NO_RECONFIGURE | 1, &PHX_ErrHandlerDefault);
	if(state != 0)
	{
		/* this is critical if this function fails no camera handle is returned */
		logError("function PHX_CameraConfigLoad(...) for Camera A failed");
		return state;
	}
	state = PHX_CameraConfigLoad( &sParameters_B->hCamera, "configurations//c8484.pcf", (etCamConfigLoad)PHX_BOARD_AUTO | PHX_DIGITAL | PHX_CHANNEL_B | PHX_NO_RECONFIGURE | 1, &PHX_ErrHandlerDefault);
	if(state != 0)
	{
		/* this is critical if this function fails no camera handle is returned */
		logError("function PHX_CameraConfigLoad(...) for Camera B failed");
		return state;
	}
	
	//function for initialising basic values for sParameterStruct
	state = configurations(&sParameters_A);
	state = configurations(&sParameters_B);
	if (state != 0)
	{
		logError("configuration failed");
		return 1;
	}
	// dunkelstromMessung(&sParameterStruct);
	setExposureTime(&sParameters_A);
	setExposureTime(&sParameters_B);


	state = startAquisition(&sParameters_A, &sParameters_A);
	if (state != 0)
	{
		logError("Aquisition failed");
		return 1;
	}

	/* Now cease all captures */
	if ( sParameters_A.hCamera ) PHX_Acquire( sParameters_A.hCamera,  PHX_ABORT, NULL );
	if ( sParameters_B.hCamera ) PHX_Acquire( sParameters_B.hCamera, PHX_ABORT, NULL );

	/* Release the Phoenix board */
	if ( sParameters_A.hCamera ) PHX_CameraRelease( &sParameters_A.hCamera );
	if ( sParameters_B.hCamera ) PHX_CameraRelease( &sParameters_B.hCamera );

	logExit();

	return 0;
}
