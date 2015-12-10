/**
 * This file manages spectroscopy
 *
 *
 */

#include "spectroscopy.h"

static int noOfMeasurementsLeft = 0;
static int noOfMeasurements = 0;
double * spectrum;

int spectroscopy_init(sSpectrometerStruct * spectro)
{
	int status = spectrometer_init(spectro);
	if(status){
		printf("init spectrometer failed\n");
		return 1;
	}

	spectrum = calloc(spectro->spectrum_length, sizeof(double));
	spectro->correction = calloc(spectro->spectrum_length, sizeof(double));

	return 0;
}

int spectroscopy_calibrate(sSpectrometerStruct * spectro)
{
	int i = 0;
	FILE * pFile;

	// measure electronic offset
	spectroscopy_meanAndSubstract(10000, 3000, spectro);
//~ printf("39\n");
	// copy last spectrum into correction buffer
	for (i = 0; i < spectro->spectrum_length; i++) {
		spectro->correction[i] = spectrum[i];
	}
	pFile = fopen("electronic-offset.dat", "wt");
	if (pFile){
		for(i=0; i < spectro->spectrum_length; i++){
			printf("50: %f\n", spectro->wavelengths[i]);
			fprintf(pFile, "%f %f \n", spectro->wavelengths[i], spectro->correction[i]);
		}
	}
//~ printf("50\n");
	// measure dark current
	spectroscopy_meanAndSubstract(1, 60000000, spectro);
//~ printf("53\n");
	// copy last spectrum into correction buffer and scale to 1s and correct by electronic offset
	for (i = 0; i < spectro->spectrum_length; i++) {
		spectro->correction[i] = spectro->lastSpectrum[i]/60  + spectro->correction[i];
	}
//~ printf("58\n");
	pFile = fopen("dark-current.dat", "wt");
	if (pFile){
		//~ printf("write to %s\n", "electronic-offset.dat");
		for(i = 0; i < spectro->spectrum_length; i++){
			fprintf(pFile, "%f %f \n", spectro->wavelengths[i], spectro->correction[i]);
		}
	}
	return 0;
}

static int done = 1;

static void callback(sSpectrometerStruct * spectro)
{
	int status;
	int i;
//~ printf("75 %i\n", noOfMeasurementsLeft);
	if(noOfMeasurementsLeft == 0) {
		//~ printf("=============\n");
		//~ printf("noOfMeasurements %i\n", noOfMeasurements);
		for (i = 0; i < spectro->spectrum_length; i++) {
			spectrum[i] /= noOfMeasurements;
		}
		//~ i = 100;
		//~ printf("%i %i %i\n", i, (int)spectrum[i], (int)(spectro->lastSpectrum[i]));
		done = 0;
	} else {
		noOfMeasurementsLeft--;
//~ printf("87 \n");
		//~ printf(":: %i %i %i %i\n", noOfMeasurementsLeft, (int)spectrum[100], (int)spectro->lastSpectrum[100], (int)spectro->correction[100]);
		//~ printf(":: %i %i %i %i\n", noOfMeasurementsLeft, (int)spectro->correction[100]);
		for (i = 0; i < spectro->spectrum_length; i++) {
			//~ spectrum[i] += spectro->lastSpectrum[i]/noOfMeasurements - spectro->correction[i];
			spectrum[i] += spectro->lastSpectrum[i] - spectro->correction[i];
		}
//~ printf("== %i \n", noOfMeasurementsLeft);
		spectrometer_trigger(spectro, callback);

 	}
	//~ printf("done callback\n");
}

int spectroscopy_meanAndSubstract(int number_of_spectra, int integration_time_micros, sSpectrometerStruct * spectro)
{
	done = 1;
	//~ printf("meanAndSubstract \n");
	int status = 0;
	spectro->integration_time_micros = integration_time_micros;
	noOfMeasurements = number_of_spectra;
	noOfMeasurementsLeft = noOfMeasurements;
//~ printf("107\n");
	/* start callback loop */
	callback(spectro);

	while(done){
		//~ printf("sleep 1000\n");
		sleepMs(1000);
	}
	printf("v done\n");

	return 0;
}

int spectroscopy_measure(sSpectrometerStruct * spectro)
{
	FILE * pFile;
	int i;

	// measure dark current
	spectroscopy_meanAndSubstract(10, 10000000, spectro);

	// copy last spectrum into correction buffer and scale to 1s and correct by electronic offset
	for (i = 0; i < spectro->spectrum_length; i++) {
		//~ spectro->correction[i] = spectro->lastSpectrum[i]/10  + spectro->correction[i];
		spectrum[i] /= 10;
	}

	pFile = fopen("measurement.dat", "wt");
	if (pFile){
		//~ printf("write to %s\n", "electronic-offset.dat");
		for(i=0; i < spectro->spectrum_length; i++){
			fprintf(pFile, "%f %f \n", spectro->wavelengths[i], spectrum[i]);
		}
	}

}

int spectroscopy_run(sSpectrometerStruct * spectro)
{

}

int spectroscopy_uninit(sSpectrometerStruct * spectro)
{

}
