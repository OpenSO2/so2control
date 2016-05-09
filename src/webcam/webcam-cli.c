#include "webcam.h"
#include "configurations.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
	int stat = 0;
	char filename[256];
	FILE *fid;
	sWebCamStruct camStruct;
	sConfigStruct config;

	printf("start test programm\n");

	/* Setup Webcam Structure */
	memset(&camStruct,0,sizeof(sWebCamStruct));
	sprintf(camStruct.filePath,"./");
	sprintf(camStruct.filePrefix, "webcam");

	camStruct.timestampBefore = malloc(sizeof(timeStruct));

	config.webcam_xRes = 1280;
	config.webcam_yRes = 720;

	/* testing functions */
    stat = webcam_init(&camStruct);
    stat = webcam_get(&camStruct);

    /* saving image */
    stat = sprintf(filename,
    			"%swebcam_%04d_%02d_%02d-%02d_%02d_%02d_%03d.raw",
				camStruct.filePath, camStruct.timestampBefore->year,
				camStruct.timestampBefore->mon, camStruct.timestampBefore->day,
				camStruct.timestampBefore->hour, camStruct.timestampBefore->min,
				camStruct.timestampBefore->sec, camStruct.timestampBefore->milli);
    if (stat<0) {
    	return -1;
    }
    fid = fopen(filename,"wb");
    stat = fwrite(camStruct.buffer,sizeof(char),camStruct.bufferSize,fid);
    if (stat != camStruct.bufferSize) {
    	printf("failed to save image\n");
    	printf("buffersize stat = %d\n",camStruct.bufferSize);
    	printf("fwrite stat = %d\n",stat);
    } else {
    	printf("IMAGE: %s saved successful\n",filename);
    }

    fclose(fid);

    stat = webcam_uninit(&camStruct);
    printf("end test programm\n");
    return 0;
}
