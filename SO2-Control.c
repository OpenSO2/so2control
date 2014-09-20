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
	int              state;
//	etParamValue     eParamValue;

	/* print welcome message in terminal */
	printOpening();

	/* Initialise control flag structure */
#


	
	
	/* Initialise parameter structure */
	memset( &sSO2Parameters, 0, sizeof(sParameterStruct ));

	//function for initialising basic values for sParameterStruct
	state = configurationFunction(&sSO2Parameters);
	if (state != 0)
	{
		logError("configuration failed");
		return 1;
	}
	// dunkelstromMessung(&sParameterStruct);
	setExposureTime(&sSO2Parameters, sSO2Parameters.hCamera_A );

	/*	Dies ueberschreibt den Wert vom letzten Aufrauf. Entweder benoetigen wir 2 Belichtungszeiten
		in der Struktur oder wir benutzen eine belichtungszeit fuer beider Kameras */
	setExposureTime(&sSO2Parameters, sSO2Parameters.hCamera_B);

	state = startAquisition(&sSO2Parameters);
	if (state != 0)
	{
		logError("Aquisition failed");
		return 1;
	}

	/* Now cease all captures */
	if ( sSO2Parameters.hCamera_A ) PHX_Acquire( sSO2Parameters.hCamera_A,  PHX_ABORT, NULL );
	if ( sSO2Parameters.hCamera_B ) PHX_Acquire( sSO2Parameters.hCamera_B, PHX_ABORT, NULL );

	/* Release the Phoenix board */
	if ( sSO2Parameters.hCamera_A ) PHX_CameraRelease( &sSO2Parameters.hCamera_A );
	if ( sSO2Parameters.hCamera_B ) PHX_CameraRelease( &sSO2Parameters.hCamera_B );

	logExit();

	return 0;
}
