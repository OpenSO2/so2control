#include "camera.h"
#include "configurations.h"
#include "opencv/highgui.h"
#include <stdio.h>
#include <string.h>

static void callback(sParameterStruct * sSO2Parameters);
static void callback(sParameterStruct * sSO2Parameters)
{
	sSO2Parameters->fBufferReady = TRUE;
}

int main(int argc, char *argv[])
{
	int stat = 0;
	char filename[256];
	FILE *fid;
	sConfigStruct config;
	char * type;
	int generate_png = 1;
	IplImage* img;
	static sParameterStruct sSO2Parameters;


	if (argc < 2) {
		printf("usage: %s <a|b>\n", argv[0]);
		return 1;
	}

	sSO2Parameters.timestampBefore = malloc(sizeof(timeStruct));

	config_init_sParameterStruct(&sSO2Parameters, &config, (char)argv[1][0]);

	/* testing functions */
	stat = camera_init(&sSO2Parameters);
	if (stat) {
		printf("failed to init camera\n");
		return -1;
	}
	stat = camera_config(&sSO2Parameters);
	if (stat) {
		printf("failed to config camera\n");
		return -1;
	}

	/* set exposure time to a very high value */
	sSO2Parameters.dExposureTime = 1000000;
	camera_setExposure(&sSO2Parameters, &config);

	/* trigger image and wait for result */
	camera_trigger(&sSO2Parameters, callback);

	while (!sSO2Parameters.fBufferReady){
		sleepMs(10);
	}

	stat = camera_get(&sSO2Parameters);
	if (stat) {
		printf("failed to get image from webcam\n");
		return -1;
	}

	if (argc == 3) {
		stat = sprintf(filename, "%s", argv[2]);
	} else {
		type = generate_png ? "png" : "raw";

		/* saving image */
		stat = sprintf(filename, "outfile.%s", type);
	}
	if (stat < 0) {
		return -1;
	}

	if(generate_png){
		/* create new image to hold the loaded data */
		img = cvCreateImageHeader(cvSize(1344, 1024), IPL_DEPTH_16U, 1);
		cvSetData(img, sSO2Parameters.stBuffer, img->widthStep);

		/* rotate image */
		cvFlip(img, img, -1);

		/* upsample image to use full 16bit range (the image will be very dark otherwise) */
		cvConvertScale(img, img, 32, 0);	/* conversion factor 12bit -> 16bit = 2^16/2^12 = 2^4 = 16 */

		cvSaveImage(filename, img, NULL);
	} else {
		fid = fopen(filename, "wb");
		stat = fwrite(sSO2Parameters.stBuffer, sizeof(char), 1344*1024, fid);
		if (stat != 1344*1024) {
			printf("failed to save image\n");
			printf("fwrite stat = %d\n", stat);
		} else {
			printf("IMAGE: %s saved successful\n",filename);
		}

		fclose(fid);
	}

	stat = camera_uninit(&sSO2Parameters);
	printf("end test programm\n");

	return 0;
}
