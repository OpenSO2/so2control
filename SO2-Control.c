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
	sParameterStruct sSO2Parameters;
	flagStruct       sControlFlags;
	int              state;
//	etParamValue     eParamValue;

	/* print welcome message in terminal */
	printOpening();

	/* Initialise control flag structure */
	memset( &sControlFlags, 0, sizeof(flagStruct ));
	/* Initialise parameter structure */
	memset( &sSO2Parameters, 0, sizeof(sParameterStruct ));

	//function for initialising basic values for sParameterStruct
	state = configurationFunktion(&sSO2Parameters,&sControlFlags);
	if (state != 0)
	{
		logError("configuration failed");
		return 1;
	}
	// dunkelstromMessung(&sParameterStruct);
	setExposureTime(&sSO2Parameters,&sControlFlags);

	state = startAquisition(&sSO2Parameters,&sControlFlags);
	if (state != 0)
	{
		logError("Aquisition failed");
		return 1;
	}

	/* Now cease all captures */
	if ( sSO2Parameters.hCamera ) PHX_Acquire( sSO2Parameters.hCamera, PHX_ABORT, NULL );

	/* Release the Phoenix board */
	if ( sSO2Parameters.hCamera ) PHX_CameraRelease( &sSO2Parameters.hCamera );

	logExit();

	return 0;
}
