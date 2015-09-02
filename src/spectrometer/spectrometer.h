#if !defined(spectrometerh)
#define spectrometerh 1
int spectrometer_init(void);
int spectrometer_uninit(void);
int spectrometer_get(double * wavelengths, double * spectra, int * length);
#endif
