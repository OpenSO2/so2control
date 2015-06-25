#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "getBufferFromFile.c"
#include "io.h"
#include "log.h"

#ifdef BENCHMARK
#include <time.h>
#endif

/* prototypes */
void parse_filename_to_timeStruct(char * infile, timeStruct *time);

void parse_filename_to_timeStruct(char * inf, timeStruct *time)
{
	int offset = strlen(inf) - 43;
	// infile is of the form testing_2014_09_22-23_43_55_984_cam_bot.raw
	//                       0         1         2         3         4
	//                       0123456789012345678901234567890123456789012
	time->year = (inf[offset +  8] - '0') * 1000 + (inf[offset +  9] - '0') * 100 + (inf[offset + 10] - '0') * 10 + inf[offset + 11] - '0';
	time->mon  = (inf[offset + 13] - '0') * 10 + inf[offset + 14] - '0';
	time->day  = (inf[offset + 16] - '0') * 10 + inf[offset + 17] - '0';
	time->hour = (inf[offset + 19] - '0') * 10 + inf[offset + 20] - '0';
	time->min  = (inf[offset + 22] - '0') * 10 + inf[offset + 23] - '0';
	time->sec  = (inf[offset + 25] - '0') * 10 + inf[offset + 26] - '0';
	time->milli= (inf[offset + 28] - '0') * 100 + (inf[offset + 29] - '0') * 10 + inf[offset + 30] - '0';
}

int main(int argc, char *argv[])
{
	char * infile = argv[1];
	char * outfolder = argv[2];
	int rawdump = argv[3] ? 1 : 2;
	short * buffer;
	timeStruct time;
	int status = 1;
	sParameterStruct sSO2Parameters;
	sConfigStruct config;

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

	sprintf(sSO2Parameters.cFileNamePrefix, "%s", "image");
	sprintf(sSO2Parameters.cImagePath, "%s", outfolder);

	config.processing = rawdump;

	io_init(&config);

	status = io_write(&sSO2Parameters);
	if(status == 0){
		log_debug("Converted %s to %s", infile, outfolder);
	} else {
		log_error("Something wrong writing to File.");
	}

	io_uninit(&config);

	return 0;
}
