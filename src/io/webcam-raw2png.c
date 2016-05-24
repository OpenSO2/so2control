#include <opencv/highgui.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char * infile;
	char * outfile;
	FILE * f;
	IplImage * img;
	unsigned int length;
	int read_bytes;
	int w = 1280;
	int h = 720;
	short *buffer = NULL;

	if (argc != 3) {
		printf("usage: %s <infile.raw> <outfile.png> \n", argv[0]);
		return 1;
	}

	infile = argv[1];
	outfile = argv[2];

	f = fopen(infile, "rb");
	if (!f) {
		printf("failed to open file\n");
		return -1;
	}

	(void)fseek(f, 0, SEEK_END);
	length = ftell(f);
	if (length < 1) {
		printf("file to small or unreadable\n");
		fclose(f);
		return -1;
	}

	(void)fseek(f, 0, SEEK_SET);
	buffer = malloc(length);
	if (!buffer) {
		printf("failed to create buffer\n");
		free(buffer);
		fclose(f);
		return -1;
	}

	read_bytes = fread(buffer, sizeof(char), length, f);
	if (length != read_bytes * sizeof(char)) {
		printf("failed to read into buffer\n");
		free(buffer);
		fclose(f);
		return -1;
	}

	fclose(f);

	img = cvCreateImageHeader(cvSize(w, h), IPL_DEPTH_8U, 3);
	cvSetData(img, buffer, img->widthStep);

	cvFlip(img, img, -1);

	cvSaveImage(outfile, img, NULL);

	return 0;
}
