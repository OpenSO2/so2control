#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//~
//~ Image manipulation functions
//~




//~ rotate an image by 180 degrees, e.g. turn it upside-down
//~  1 2
//~  3 4     => 1 2 3 4
//~
//~  4 3
//~  2 1     => 4 3 2 1
char *rotate180(char *string){
	int i;
	int l = (int)strlen(string);
	char *tmpstr = (char *)malloc( l * sizeof(char) + 1 ); // (char *) to please c++ compiler...

	strcpy(tmpstr, string);

	for(i = 0; i < l; i++){
		string[i] = tmpstr[ l - i - 1 ];
	}

	free(tmpstr);

	return string;
}

// calculates the correlation for a set of two images by averaging the squared brightness difference.
// @TODO: check for better algorithms
float calcCorrelation(char *img1, char *img2, int length){
	int i, skipped = 0;
	float correlation = 0.0, diff;
	for(i = 0; i < length; i++){
		if(img1[i] == 'a' || img2[i] == 'a'){
			//~ printf(" skipped ");
			skipped++;
		} else {
			diff = (float)img1[i] - (float)img2[i];
			//~ printf("%f", diff);
			correlation += diff*diff;
			//~ printf(" [%i/%i] ", i, length);
			//~ printf(" diff: %f (%c, %c), diffsq %f => corr %f, ", diff, img1[i], img2[i], diff*diff, correlation);
		}
	}
	//~ printf("correlation %f for %s and %s and %i values\n", correlation, img1, img2, length);
	correlation /= length - skipped;
	correlation = 1/correlation; // invert so that higher values correspond to a better correlation
	return correlation;
}

// displace an image by a fixed displacement vector
int displaceImage(char *imageBuffer, char *displacedimageBuffer, int width, int height, int x, int y){
	int i, row, col, l = strlen(imageBuffer);
	//~ printf("\n%i\n", l);
	//~ if(strlen(imageBuffer) != strlen(displacedimageBuffer) ){
		//~ printf("Error: imagebuffer and displacedimagebuffer are not of the same length: %i != %i \n", strlen(imageBuffer), strlen(displacedimageBuffer));
	//~ }
	for( i = 0; i < l; i++ ){
		col = i%width;
		row = i/width;

		if(    col - x > width  //
			|| col - y < 0      //
			|| row - x < 0      //
			|| row - y > height //
			|| i - x - y*width > 35 // FIXME
			|| i - x - y*width < 0 // FIXME
		){
			//~ printf(" (skipped) ");
			displacedimageBuffer[i] = 'a';
		} else {
			// calc displaced pixel
			displacedimageBuffer[i] = imageBuffer[ i - x - y*width ];
		}

		//~ printf("%i>%i|%i%c\n", i, i - x - y*width, displacedimageBuffer[i], displacedimageBuffer[i]);

		//~ if(displacedimageBuffer[i] == 0){
			//~ displacedimageBuffer[i] = 'a';
			//~ printf("EOW %c", displacedimageBuffer[i]);
		//~ }

	}
	return 1;
}

//~ TODO: incorporate into sSO2Parameters?
struct disp{
	int x;
	int y;
};

// returns the displacement vector between two images
struct disp *findDisplacement(char *img1, char *img2, int height, int width){
	struct disp *displacement;
	displacement = (disp *)malloc(sizeof(struct disp));

	int x, y, displacement_x = 0, displacement_y = 0;
	int l = strlen(img1);

	float corr = 0.0, correlation = 0.0;

	char *displacementBuffer;
	displacementBuffer = (char *)malloc( l * sizeof(char) );

	int max_distance = 2;

	//~ calculate the correlation for every displacement in n pixel distance
	for (x = -max_distance + 1; x < max_distance; x++){
		for (y = -max_distance + 1; y < max_distance; y++){
			//~ printf("length displacementBuffer %i\n", strlen(displacementBuffer));
			displaceImage(img2, displacementBuffer, width, height, x, y); //~ printf("displacementBuffer was returned with %i", strlen(displacementBuffer) );
			corr = calcCorrelation(img1, displacementBuffer, l);
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



