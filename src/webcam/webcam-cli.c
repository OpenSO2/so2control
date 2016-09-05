#include "webcam.h"
#include "configurations.h"
#include <opencv/highgui.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int stat = 0;
	char * filename;
	FILE * fid;
	sWebCamStruct webcam;
	sConfigStruct config;
	IplImage * img;
	int generate_png = 1;

	printf("start test programm\n");

	if (argc != 2) {
		printf("usage: %s <outputfile> \n", argv[0]);
		return 1;
	}

	filename = argv[1];

	/* Setup Webcam Structure */
	memset(&webcam, 0, sizeof(sWebCamStruct));

	config.webcam_xRes = 1280;
	config.webcam_yRes = 720;

	/* testing functions */
	stat = webcam_init(&config, &webcam);
	if (stat) {
		printf("failed to init webcam\n");
		return -1;
	}

	stat = webcam_get(&webcam);
	if (stat) {
		printf("failed to get image from webcam\n");
		return -1;
	}

	if(generate_png){
		img = cvCreateImageHeader(cvSize(config.webcam_xRes, config.webcam_yRes), IPL_DEPTH_8U, 3);
		cvSetData(img, webcam.buffer, img->widthStep);

		cvSaveImage(filename, img, NULL);
	} else {
		fid = fopen(filename, "wb");
		stat = fwrite(webcam.buffer, sizeof(char), webcam.bufferSize,fid);
		if (stat != webcam.bufferSize) {
			printf("failed to save image\n");
			printf("buffersize stat = %d\n", webcam.bufferSize);
			printf("fwrite stat = %d\n", stat);
		} else {
			printf("IMAGE: %s saved successful\n", filename);
		}

		fclose(fid);
	}

	stat = webcam_uninit(&config, &webcam);
	printf("end test programm\n");

	return 0;
}
