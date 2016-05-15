#include "webcam.h"
#include "configurations.h"
#include <opencv/highgui.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int stat = 0;
	char filename[256];
	FILE *fid;
	sWebCamStruct camStruct;
	sConfigStruct config;
	IplImage* img;
	char * type;
	int generate_png = 1;

	printf("start test programm\n");

	/* Setup Webcam Structure */
	memset(&camStruct, 0, sizeof(sWebCamStruct));
	sprintf(camStruct.filePath, "./");
	sprintf(camStruct.filePrefix, "webcam");

	camStruct.timestampBefore = malloc(sizeof(timeStruct));

	config.webcam_xRes = 1280;
	config.webcam_yRes = 720;

	/* testing functions */
	stat = webcam_init(&config);
	if (stat) {
		printf("failed to init webcam\n");
		return -1;
	}

	stat = webcam_get(&camStruct);
	if (stat) {
		printf("failed to get image from webcam\n");
		return -1;
	}

	if (argc == 2) {
		stat = sprintf(filename, "%s", argv[1]);
	} else {
		type = generate_png ? "png" : "raw";

		/* saving image */
		stat = sprintf(filename,
			"%swebcam_%04d_%02d_%02d-%02d_%02d_%02d_%03d.%s",
			camStruct.filePath, camStruct.timestampBefore->year,
			camStruct.timestampBefore->mon, camStruct.timestampBefore->day,
			camStruct.timestampBefore->hour, camStruct.timestampBefore->min,
			camStruct.timestampBefore->sec, camStruct.timestampBefore->milli, type);
	}
	if (stat < 0) {
		return -1;
	}

	if(generate_png){
		img = cvCreateImageHeader(cvSize(config.webcam_xRes, config.webcam_yRes), IPL_DEPTH_8U, 3);
		cvSetData(img, camStruct.buffer, img->widthStep);

		cvSaveImage(filename, img, NULL);
	} else {
		fid = fopen(filename, "wb");
		stat = fwrite(camStruct.buffer, sizeof(char), camStruct.bufferSize,fid);
		if (stat != camStruct.bufferSize) {
			printf("failed to save image\n");
			printf("buffersize stat = %d\n",camStruct.bufferSize);
			printf("fwrite stat = %d\n",stat);
		} else {
			printf("IMAGE: %s saved successful\n",filename);
		}

		fclose(fid);
	}

	stat = webcam_uninit(&config);
	printf("end test programm\n");

	return 0;
}
