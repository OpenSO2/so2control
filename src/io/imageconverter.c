#include <stdio.h>
#include <libgen.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "make_png_header.c"
#include "bufferToImage.c"
#include "getBufferFromFile.c"

#ifdef BENCHMARK
#include <time.h>
#endif

void parse_filename_to_date(char *infile, char *date)
{
	char *inf = basename(infile);
	// infile is of the form testing_2014_09_22-23_43_55_984_cam_bot.raw
	//                       0123456789012345678901234567890123456789012
	// date is of the form YYYY-MM-DDThh:mm:ss.sssZ (ISO 8601)
	//                     012345678901234567890123
	date[0] = inf[8];	// 'Y'
	date[1] = inf[9];	// 'Y'
	date[2] = inf[10];	// 'Y'
	date[3] = inf[11];	// 'Y'
	date[4] = '-';
	date[5] = inf[13];	// 'M'
	date[6] = inf[14];	// 'M'
	date[7] = '-';
	date[8] = inf[16];	// 'D'
	date[9] = inf[17];	// 'D'
	date[10] = 'T';
	date[11] = inf[19];	// 'h'
	date[12] = inf[20];	// 'h'
	date[13] = ':';
	date[14] = inf[22];	// 'm'
	date[15] = inf[23];	// 'm'
	date[16] = ':';
	date[17] = inf[25];	// 's'
	date[18] = inf[26];	// 's'
	date[19] = '.';
	date[20] = inf[28];	// 's'
	date[21] = inf[29];	// 's'
	date[22] = inf[30];	// 's'
	date[23] = 'Z';
	date[24] = '\0';
}

int main(int argc, char *argv[])
{
	char *buffer = 0;
	char *infile = argv[1];
	char *outfile = argv[2];
	float startTime;

	if (argc != 3) {
		puts("Usage: imageviewer in.raw out.png\n");
		return 1;
	}
#ifdef BENCHMARK
	startTime = (float)clock() / CLOCKS_PER_SEC;
#endif
	buffer = getBufferFromFile(infile);
#ifdef BENCHMARK
	printf("reading file took %fms \n",
	       ((float)clock() / CLOCKS_PER_SEC - startTime) * 1000);
#endif

#ifdef BENCHMARK
	startTime = (float)clock() / CLOCKS_PER_SEC;
#endif
	IplImage *img = bufferToImage(buffer);
#ifdef BENCHMARK
	printf("bufferToImage took %fms \n",
	       ((float)clock() / CLOCKS_PER_SEC - startTime) * 1000);
#endif

	// encode image as png to buffer
	// playing with the compression is a huge waste of time with no benefit
#ifdef BENCHMARK
	startTime = (float)clock() / CLOCKS_PER_SEC;
#endif
	CvMat *png = cvEncodeImage(".png", img, 0);
#ifdef BENCHMARK
	printf("encoding took %fms \n",
	       ((float)clock() / CLOCKS_PER_SEC - startTime) * 1000);
#endif

	int header_length = 60;
	int l = png->rows * png->cols;
	int l_pad = l + header_length;
	int i;

	char *padded_png = malloc(l_pad * 2);
	memcpy(padded_png, png->data.ptr, l);
	png->data.ptr = padded_png;

	// copy end of png
	for (i = 8; i > 0; i--) {
		png->data.ptr[l_pad - i] = png->data.ptr[l - i];
	}

#ifdef BENCHMARK
	startTime = (float)clock() / CLOCKS_PER_SEC;
#endif
	char date[25];
	char text[39];
	char name[14] = "Creation Time ";
	parse_filename_to_date(infile, date);
	strcpy(text, name);
	strcat(text, date);
	text[13] = 0;
	int head[header_length];
	make_png_header(text, 38, head);

	// fill in
	for (i = 0; i < header_length; i++) {
		png->data.ptr[l - 12 + i] = head[i];
	}
#ifdef BENCHMARK
	printf("setting png text chunk took %fms \n",
	       ((float)clock() / CLOCKS_PER_SEC - startTime) * 1000);
#endif

	// save image to disk
	FILE *fp = fopen(outfile, "wb");
	if (fp) {
#ifdef BENCHMARK
		startTime = (float)clock() / CLOCKS_PER_SEC;
#endif
		fwrite(png->data.ptr, 1, l_pad, fp);
#ifdef BENCHMARK
		printf("writing png took %fms \n",
		       ((float)clock() / CLOCKS_PER_SEC - startTime) * 1000);
#endif
		printf("Converted %s to %s\n", infile, outfile);
	} else {
		puts("Something wrong writing to File.\n");
	}

	return 0;
}
