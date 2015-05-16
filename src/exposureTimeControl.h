/* Header Datei fuer die Steuerung der Automatischen
 * Belichtungszeitsteuerung
 */

#ifndef _EXPOSURETIMECONTROL_
#define _EXPOSURETIMECONTROL_
#include"configurations.h"
	int setExposureTime( sParameterStruct *sSO2Parameters);
	int evalHist(stImageBuff *stBuffer, sParameterStruct *sSO2Parameters, int *timeSwitch);
#endif