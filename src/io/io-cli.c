#include <stdio.h>
#include <libgen.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "bufferToImage.c"
#include "converter/getBufferFromFile.c"
#include "io.h"
#include "log.h"

#ifdef BENCHMARK
#include <time.h>
#endif

/* prototypes */
void parse_filename_to_timeStruct(char * infile, timeStruct *time);

void parse_filename_to_timeStruct(char * infile, timeStruct *time)
{
	char *inf = basename(infile);
	// infile is of the form testing_2014_09_22-23_43_55_984_cam_bot.raw
	//                       0123456789012345678901234567890123456789012
	// date is of the form YYYY-MM-DDThh:mm:ss.sssZ (ISO 8601)
	//                     012345678901234567890123
	time->year = (inf[ 8] - '0') * 1000 + (inf[ 9] - '0') * 100 + (inf[10] - '0') * 10 + inf[11] - '0';
	time->mon  = (inf[13] - '0') * 10 + inf[14] - '0';
	time->day  = (inf[16] - '0') * 10 + inf[17] - '0';
	time->hour = (inf[19] - '0') * 10 + inf[20] - '0';
	time->min  = (inf[22] - '0') * 10 + inf[23] - '0';
	time->sec  = (inf[25] - '0') * 10 + inf[26] - '0';
	time->milli= (inf[28] - '0') * 100 + (inf[29] - '0') * 10 + inf[30] - '0';
}



int main(int argc, char *argv[])
{
	char * infile = argv[1];
	char * outfolder = argv[2];
	int rawdump = argv[3] || 0;
	char * buffer;
	int status = 1;
	sParameterStruct sSO2Parameters;

	if(log_init()){
		printf("could not start log file, stop.\n");
	}

	#ifdef BENCHMARK
		float startTime;
	#endif

	if (argc < 3 || argc > 4) {
		printf("Usage: %s in.raw outfolder [write_raw_dump]\n", argv[0]);
		return 1;
	}

	#ifdef BENCHMARK
		startTime = (float)clock() / CLOCKS_PER_SEC;
	#endif

	buffer = getBufferFromFile(infile);

	#ifdef BENCHMARK
		log_debug("reading file took %fms \n",
			   ((float)clock() / CLOCKS_PER_SEC - startTime) * 1000);
	#endif

	timeStruct time;
	parse_filename_to_timeStruct(infile, &time);

	sSO2Parameters.dImageCounter = 0;
	sSO2Parameters.dTriggerPulseWidth = 15;
	sSO2Parameters.dBufferlength = 1376256;
	sSO2Parameters.dHistMinInterval = 350;
	sSO2Parameters.dHistPercentage = 5;
	sSO2Parameters.dInterFrameDelay = 10;
	sSO2Parameters.fid = NULL;
	sSO2Parameters.identifier = 'A';
	sSO2Parameters.stBuffer = buffer;
	sSO2Parameters.timestampBefore = &time;
	sSO2Parameters.dFixTime = 0.000000;

	sprintf(sSO2Parameters.cFileNamePrefix, "");
	sprintf(sSO2Parameters.cImagePath, outfolder);

	if(rawdump)
		status = io_writeDump(&sSO2Parameters);
	else
		status = io_writeImage(&sSO2Parameters);

	if(status == 0){
		log_debug("Converted %s to %s", infile, outfolder);
	} else {
		log_error("Something wrong writing to File.");
	}

	return 0;
}
