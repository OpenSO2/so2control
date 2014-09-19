#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "unistd.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

// setup & cleanup
int main( int argc, char* const argv[] )
{
	// global setup...

	int result = Catch::Session().run( argc, argv );

	// global clean-up...

	return result;
}

// helper
short *readImage(const char *inputFilename){
	FILE *inputFID, *outputFID;
	int i, j, xlength, ylength, depth, numElements;
	char *headerBuffer;
	short *inputBuffer;

	inputFID = fopen(inputFilename,"rb");
	if ( inputFID != NULL)
	{
		/* later: read var from header */
		xlength = 1344;
		ylength = 1024;
		depth = sizeof(short);

		/* allocate space for header Buffer */
		headerBuffer = (char *)malloc(64*sizeof(char));

		/* allocate space for input Buffer */
		inputBuffer = (short *)malloc(xlength * ylength * sizeof(short));

		if ( inputBuffer != NULL)
		{
			/* read header from file */
			fread(headerBuffer, sizeof(char), 64, inputFID);
			/* read image data from file */
			numElements = fread(inputBuffer, depth, xlength * ylength, inputFID);

			if (numElements != xlength*ylength)
			{
				printf("not enough elements read from file\n");
				printf(" %i elements read from %s\n", numElements, inputFilename);
			}

			free(headerBuffer);
		}

		else
		{
			printf("close file %s failed \n",inputFilename);
		}
	}
	else
	{
		printf("open file %s failed.\n",inputFilename);
	}
	return inputBuffer;
}


// code to be tested
#include "../imageFunctions.c"

TEST_CASE( "Image functions" ) {
	SECTION( "rotateImage reverses an array" ) {
		short a[] = { 1, 0 };
		short b[] = { 0, 1 };
		int l = 2;
		rotateImage(a, l);
		for(int i = 0; i < l; i++){
			REQUIRE( a[i] == b[i] );
		}
	}

	SECTION("rotateImage rotates images"){
		short *inFile1, *inFile2;
		int i, l = 1344*1024;
		inFile1 = readImage("Test/fixtures/unten.raw");
		inFile2 = readImage("Test/fixtures/unten_turned.raw");
		rotateImage(inFile1, l);
		for(int i=1; i < l; i++){
			if(inFile1[i-1] != inFile2[i]) printf("%i %i\n", inFile1[i-1], inFile2[i]);
			//~ REQUIRE( inFile1[i] == inFile2[i+1] );
		}
	}

	SECTION("calcCorrelation calculates a correlation between two images"){
		// img 1  img 2  img 3  expected correlation:
		// 000    110    011    corr1/2 < corr1/3 == corr2/3
		// 011    110    011
		// 011    000    000
		short img1[] = {
			0,0,0,
			0,1,1,
			0,1,1
		};
		short img2[] = {
			1,1,0,
			1,1,0,
			0,0,0
		};
		short img3[] = {
			0,1,1,
			0,1,1,
			0,0,0
		};

		float corr12 = calcCorrelation(img1, img2, 9);
		float corr13 = calcCorrelation(img1, img3, 9);
		float corr23 = calcCorrelation(img2, img3, 9);

		REQUIRE(corr12 < corr13);
		REQUIRE(corr13 == corr23);
	}

	SECTION("findDisplacement returns the correct displacement vector"){
		// img 1     img 2     expected displacement:
		// 000000    011100    0, -1
		// 011100    011100
		// 011100    011100
		// 011100    000000
		// 000000    000000
		// 000000    000000
		short img1[] = {
			0,0,0,0,0,0,
			0,1,1,1,0,0,
			0,1,1,1,0,0,
			0,1,1,1,0,0,
			0,0,0,0,0,0,
			0,0,0,0,0,0
		};
		short img2[] = {
			0,1,1,1,0,0,
			0,1,1,1,0,0,
			0,1,1,1,0,0,
			0,0,0,0,0,0,
			0,0,0,0,0,0,
			0,0,0,0,0,0
		};

		struct disp *displacement = findDisplacement(img1, img2, 6, 6);
		REQUIRE(displacement->x ==  0);
		REQUIRE(displacement->y == -1);
	}

	SECTION("findDisplacement returns the correct displacement vector for an image"){
		short *inFile1, *inFile2, *outFile;
		inFile1 = readImage("Test/fixtures/oben.raw");
		inFile2 = readImage("Test/fixtures/unten_turned.raw");

		struct disp *displacement;
		displacement = findDisplacement(inFile1, inFile2, 1024, 1344);

		REQUIRE(displacement->x == 9); // FIXME: verify
		REQUIRE(displacement->y == -9); // FIXME: verify
	}

	SECTION("displaceImage returnes a displaced copy of an image buffer"){
		// img 1      expected
		// 1 1 1 0    -1-1-1-1
		// 1 1 1 0    -1 1 1 1
		// 1 1 1 0    -1 1 1 1
		// 0 0 0 0    -1 1 1 1
		short img1[] = {
			1,1,1,0,
			1,1,1,0,
			1,1,1,0,
			0,0,0,0
		};
		short expected[] = {
			5000,5000,5000,5000,
			5000,1,1,1,
			5000,1,1,1,
			5000,1,1,1
		};
		short displaced[] = {
			0,0,0,0,
			0,0,0,0,
			0,0,0,0,
			0,0,0,0
		};
		displaceImage(img1, displaced, 4, 4, 1, 1);

		for(int i = 0; i < 16; i++){
			REQUIRE( displaced[i] == expected[i] );
		}
	}
}

