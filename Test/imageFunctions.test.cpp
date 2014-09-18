#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "unistd.h"
#include <sys/types.h>
#include <sys/stat.h>

// setup & cleanup
int main( int argc, char* const argv[] )
{
	// global setup...

	int result = Catch::Session().run( argc, argv );

	// global clean-up...

	return result;
}

// code to be tested
#include "../imageFunctions.c"

TEST_CASE( "Image functions" ) {
	SECTION( "rotate180 reverses a string" ) {
		char a[] = { "ab" };
		char b[] = { "ba" };
		REQUIRE( strcmp( rotate180(a), b ) == 0 );
	}

	SECTION("calcCorrelation calculates a correlation between two images"){
		// img 1  img 2  img 3  expected correlation:
		// 000    110    011    corr1/2 < corr1/3 == corr 2/3
		// 011    110    011
		// 011    000    000
		char img1[] = { "000011011" };
		char img2[] = { "110110000" };
		char img3[] = { "011011000" };

		float corr12 = calcCorrelation(img1, img2, 9);
		float corr13 = calcCorrelation(img1, img3, 9);
		float corr23 = calcCorrelation(img2, img3, 9);

		//~ printf("corr12: %f, corr13: %f, corr23: %f\n", corr12, corr13, corr23);

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
		char img1[] = { "000000011100011100011100000000000000" };
		char img2[] = { "011100011100011100000000000000000000" };

		struct disp *displacement = findDisplacement(img1, img2, 6, 6);
		REQUIRE(displacement->x ==  0);
		REQUIRE(displacement->y == -1);
	}

	SECTION("findDisplacement returns the correct displacement vector for an image"){
		//~ FILE *fh = fopen("fixtures/calibrierung_2014_09_15_oben.RBF", "rb");
		//~ fseek(fh, 64, SEEK_SET);
		//~ fread(16)
		//~ fread(imageBuffer, )


		//~ REQUIRE(displacement->x == 1);
		//~ REQUIRE(displacement->y == 1);
	}

	SECTION("displaceImage returnes a displaced copy of an image buffer"){
		// img 1      expected
		// 1 1 1 0    -1-1-1-1
		// 1 1 1 0    -1 1 1 1
		// 1 1 1 0    -1 1 1 1
		// 0 0 0 0    -1 1 1 1
		char img1[] = { "1110111011100000" };
		char expected[] = {  "aaaaa111a111a111" };
		char displaced[] = { "0000000000000000" };
		displaceImage(img1, displaced, 4, 4, 1, 1);
		//~ printf("result: %s\nexpected: %s", displaced, expected);
		REQUIRE( strcmp(displaced, expected) == 0 );
		REQUIRE( strcmp(displaced, img1)     != 0 );
	}
}

