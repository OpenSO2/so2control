#include "webcam.h"
#include "ucam1/webcam.c"


void main(void){
	#define PACKAGE_SIZE 512
	char data[PACKAGE_SIZE];
	FILE * fp;
	memset(data, 0, PACKAGE_SIZE);

	webcam_init();

	webcam_get(data, PACKAGE_SIZE);
	fwrite(data, PACKAGE_SIZE, fp);

	webcam_uninit();
}
