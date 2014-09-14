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

}

