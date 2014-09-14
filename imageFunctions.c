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
