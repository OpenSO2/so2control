#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "imageFunctions.h"

//~
//~ Image manipulation functions
//~



// rotates an image by 180 degrees, e.g. turn it upside-down
//  1 2
//  3 4     => 1 2 3 4
//
//  4 3
//  2 1     => 4 3 2 1
int rotateImage(short *Buffer, int length)
{
	short *tempBuffer;
	int i,j;

	tempBuffer = (short *)malloc(length*sizeof(short));
	if (tempBuffer != NULL)
	{
		/* copy Buffer to tempBuffer */
		for (j = 0; j < length; j++)
		{
			tempBuffer[j] = Buffer[j];
		}
		/* rotate image data */
		j = length;
		for (i = 0; i < length; i++)
		{
			j--;
			Buffer[j] = tempBuffer[i];
		}

		free(tempBuffer);
	}
	else
	{
		printf("Allocation failed \n");
		return 1;
	}

	return 0;
}

// calculates the correlation for a set of two images by averaging the squared brightness difference.
// @TODO: check for better algorithms
float calcCorrelation(short *img1, short *img2, int length){
	int i, skipped = 0;
	float correlation = 0.0, diff;

	for(i = 0; i < length; i++){
		if(img1[i] == 5000 || img2[i] == 5000){
			skipped++;
		} else {
			diff = (float)img1[i] - (float)img2[i];
			correlation += diff*diff/100000;
			//~ printf(" [%i/%i] %f", i, length, img1[i]);
			//~ printf(" diff: %f (%i, %i), diffsq %f => corr %f, ", diff, img1[i], img2[i], diff*diff, correlation);
		}
	}

	correlation /= (float)(length - skipped);

	if(length - skipped < 1) return 0.0;

	//~ printf("\ncorrelation %f (%f) for %i values", correlation, 1./correlation, length - skipped);

	correlation = 1/correlation; // invert so that higher values correspond to a better correlation
	return correlation;
}

// displace an image by a fixed displacement vector
// FIXME: height/width
int displaceImage(short *imageBuffer, short *displacedimageBuffer, int width, int height, int x, int y){
	int i, row, col, l = width * height;

	for( i = 0; i < l; i++ ){
		col = i%width;
		row = i/width;

		if(    col - x > width  //
			|| col - y < 0      //
			|| row - x < 0      //
			|| row - y > height //
			//~ || i - x - y*width > 35 // FIXME
			|| i - x - y*width < 0 // FIXME
		){
			//~ printf(" (skipped) ");
			displacedimageBuffer[i] = 5000;
		} else {
			// calc displaced pixel
			displacedimageBuffer[i] = imageBuffer[ i - x - y*width ];
		}
	}
	return 1;
}

// returns the displacement vector between two images
struct disp *findDisplacement(short *img1, short *img2, int height, int width, int max_distance){
	struct disp *displacement;

	int x, y, displacement_x = 0, displacement_y = 0;
	int l = height*width;

	float corr = 0.0, correlation = 0.0;

	short *displacementBuffer;
	displacementBuffer = (short *)malloc( l * sizeof(short) );

#ifdef _PHX_WIN32
	displacement = malloc(sizeof(struct disp));
#else
	displacement = (disp *)malloc(sizeof(struct disp));
#endif

	//~ calculate the correlation for every displacement in n pixel distance
	for (x = -max_distance + 1; x < max_distance; x++){
		for (y = -max_distance + 1; y < max_distance; y++){
			//~ printf("length displacementBuffer %i\n", strlen(displacementBuffer));

			displaceImage(img2, displacementBuffer, width, height, x, y); //~ printf("displacementBuffer was returned with %i", strlen(displacementBuffer) );

			corr = calcCorrelation(img1, displacementBuffer, l);

			//~ printf("\n# calculated correlation for [%i, %i] : %f\n", x, y, corr );
			//~ printf("\n# calculated correlation for [%i, %i] : %f (img1: %s, img2: %s, displ: %s)\n\n", x, y, corr, img1, img2, displacementBuffer );
			if(corr > correlation){
				displacement_x = x;
				displacement_y = y;
				correlation = corr;
			}
		}
	}

	free(displacementBuffer);

	displacement->x = -displacement_x;
	displacement->y = -displacement_y;

	return displacement;
}



//
int getBufferCenter(short *buffer, short *smallBuffer, int height, int width, int smallHeight, int smallWidth){
	int i;
	int l = height * width;
	int start = (height - smallHeight)/2 * width;
	int end = l - start;
	int j = 0;
	int col;

	for(i = start; i < end; i++){ // this cuts off top and bottom
		col = i%width;
		if( // this cuts off left and right
			   col > (width - smallWidth)/2 - 1 // left
			&& col < (width + smallWidth)/2 // right
		){
			smallBuffer[j] = buffer[i];
			j++;
		}
	}
	return 0;
}



int cmp(const void *ptr1, const void *ptr2) {
	if( *(short *)ptr1 < *(short *)ptr2 )
		return -1;
	else if( *(short *)ptr1 > *(short *)ptr2 )
		return 1;
	else
		return 0;
}

// calculates the median
// @FIXME: document
// @FIXME: write unit test
// @TODO: rename to something sensible
int findMedian(short *buffer, int length){
	int i;
	short *copy;
	int firstNonBlack;
	int edgeDiff = 500;
	int edge = 0;

	// copy buffer
	copy = (short *)malloc(length * sizeof(short));
	for(i = 0; i < length; i++){
		copy[i] = buffer[i];
	}

	// sort copied buffer
	qsort(copy, length, sizeof(short), cmp);

	// find brightness edge
	int leap = 10000;
	for(i = leap; i < length; i = i + 10){
		if(copy[i] - copy[i-leap] > edgeDiff){
			edge = i;
			printf("found edge at %i with %i (%i -> %i)\n", edge, copy[edge], copy[i-leap], copy[i]);
			return (copy[i] + copy[i-leap])/2;
		}
	}

	printf("no edge detected, switching to secondary algorithm!\n");

	//find mean value
	for(i = 0; i < length; i++){
		if(copy[i] != 0){
			firstNonBlack = i;
			return copy[ (firstNonBlack + length)/2 ] - 100;
		}
	}
}

// @FIXME: document
// @FIXME: write unit test
int posterize(short *buffer, int length){
	int i;
	short black = 0;
	short white = 4000;
	short median = findMedian(buffer, length);

	printf("calculated median: %i\n", median);

	for(i = 0; i < length; i++){
		buffer[i] = buffer[i] > median ? white : black;
	}
	return 0;
}

