#include <opencv/cv.h>
#include <stdio.h>

IplImage *bufferToImage(short *buffer);
IplImage *bufferToImage(short *buffer)
{
	IplImage *img;
	int BUFFERSIZE = 1344 * 1024 * 16 / 8;
	// create new image to hold the loaded data
	CvSize mSize;
	mSize.height = 1024;
	mSize.width = 1344;

	img = cvCreateImage(mSize, IPL_DEPTH_16U, 1);

	if (!img) {
		printf("%s", "failed to decode image\n");
		return img;
	}

	memcpy(img->imageData, buffer, BUFFERSIZE);

	// rotate image
	cvFlip(img, img, -1);

	// upsample image to use full 16bit range (the image will be very dark otherwise)
	cvConvertScale(img, img, 16, 0);	// conversion factor 12bit -> 16bit = 2^16/2^12 = 2^4 = 16

	return img;
}

IplImage *bufferToImageCam(char *buffer);
IplImage *bufferToImageCam(char *buffer)
{
	IplImage *img;
	int BUFFERSIZE = 1280 * 720 * 3;
	// create new image to hold the loaded data
	CvSize mSize;

	mSize.height = 720;
	mSize.width = 1280;

	img = cvCreateImage(mSize, IPL_DEPTH_8U, 3);

	if (!img) {
		printf("%s", "failed to decode image\n");
		return img;
	}

	memcpy(img->imageData, buffer, BUFFERSIZE);

	return img;
}
