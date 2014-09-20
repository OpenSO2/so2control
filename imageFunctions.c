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
		
		/*	length = 2752512
			programm bricht bei j= 1378288 ab
		 */

		for (j=0; j < length-1; j++)
		{
			tempBuffer[j] = Buffer[j];
		}
		/* rotate image data */
		j = length-1;
		for (i=0; i<length; i++)
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

#ifdef WIN32
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

