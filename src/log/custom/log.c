#include<stdio.h>
#include<stdarg.h>
#include<time.h>
#include<sys/stat.h>
#include "log.h"
#include "timehelpers.h"

#ifdef POSIX
#include "errno.h"
#endif

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define LOG_BUFFER_SIZE 512

static sConfigStruct *conf = NULL;
static FILE *logfile = NULL;
static timeStruct t;

int logg(char *type, char *message, va_list args);

int log_init(sConfigStruct * config)
{
	struct stat st = { 0 };
	char * dir;
	char * filename;
	int filenamelength;
	char * subpath = "logs";
	conf = config;

	if(strcmp("-", config->cImagePath)){
		// check for and remove trailing "/" to avoid ugly "//" in imagePath
		if (config->cImagePath[(strlen(config->cImagePath) - 1)] == '/') {
			config->cImagePath[(strlen(config->cImagePath) - 1)] = '\0';
		}

		dir = malloc(strlen(config->cImagePath) + 1 + strlen(subpath) + 1);
		dir[0] = '\0';

		strcat(dir, config->cImagePath);
		strcat(dir, "/");
		strcat(dir, subpath);

		/* check if log folder exists and create if not */
		if (stat(dir, &st) == -1) {
			mkdir(dir, 0700);
		}

		/*
		 * 0        1         2
		 * 1234567890123456789012345
		 * /log_2017_05_26_23_47.txt
		 */
		filenamelength = strlen(dir) + 25 + 1;
		filename = malloc(filenamelength);
		getTime(&t);
		snprintf(filename, filenamelength, "%s/log_%04d_%02d_%02d_%02d_%02d.txt",
			dir, t.year, t.mon, t.day, t.hour, t.min);

		free(dir);

		logfile = fopen(filename, "a");
		if (NULL == logfile) {
			log_error("failed to open logfile %s", filename);
			free(filename);
			return 1;
		}

		log_message("logfile will be written to %s", filename);

		free(filename);
	} else {
		log_message("no log is written to disk");
	}

	log_message("*******************************************");
	log_message("*                                         *");
	log_message("*       SO2-Camera Control Software       *");
	log_message("*                                         *");
	log_message("*******************************************");
	log_message("*                                         *");
	log_message("* A control program for SO2 measuring     *");
	log_message("* camera based on two Hamamatsu           *");
	log_message("* C8484-16 CCD Cameras.                   *");
	log_message("*                                         *");
	log_message("* written by Johann Jacobson              *");
	log_message("* and Morten Harms                        *");
	log_message("*                                         *");
	log_message("* johann.jacobson(at)zmaw.de              *");
	log_message("* morten.harms(at)zmaw.de                 *");
	log_message("*                                         *");
	log_message("*******************************************");
	log_message("Today is %02d.%02d.%04d %02d:%02d",
		t.day, t.mon, t.year, t.hour, t.min);

	return 0;
}

int log_message(char *message, ...)
{
	int state;
	va_list args;
	va_start(args, message);
	state = logg("INFO ", message, args);
	va_end(args);
	return state;
}

int log_error(char *message, ...)
{
	int state;
	va_list args;
	va_start(args, message);
#ifdef POSIX
	if( errno != 0 ){
		char errstr[512];
		char * new_message;
		char * glue = ". Error returned from OS was: ";

		strerror_r(errno, errstr, 512);

		if((new_message = malloc(strlen(message) + strlen(errstr) + strlen(glue) + 1)) != NULL){
			new_message[0] = '\0';
			strcat(new_message, message);
			strcat(new_message, glue);
			strcat(new_message, errstr);
			message = new_message;
		}
	}
#endif
	state = logg("ERROR", message, args);

#ifdef POSIX
	if( errno != 0 )
		free(message);
#endif

	va_end(args);

	return state;
}

int log_debug(char *message, ...)
{
	int state;
	va_list args;

	if (!conf || conf->debug == 0)
		return 0;

	va_start(args, message);
	state = logg("DEBUG", message, args);
	va_end(args);

	return state;
}

int logg(char *type, char *message, va_list args)
{
	static char buffer[LOG_BUFFER_SIZE];
	static char buffer2[LOG_BUFFER_SIZE];

	getTime(&t);

	/* resolve placeholders message */
	vsnprintf(buffer, LOG_BUFFER_SIZE, message, args);

	/* generate final log string */
	snprintf(buffer2, LOG_BUFFER_SIZE, "%02d:%02d:%02d.%03d | %s | %s\n",
		t.hour, t.min, t.sec, t.milli, type, buffer);

	if (strcmp(type, "ERROR"))
		fprintf(stdout, "%s", buffer2);
	else
		fprintf(stderr, ANSI_COLOR_RED "%s" ANSI_COLOR_RESET, buffer2);

	if (logfile) {
		fprintf(logfile, "%s", buffer2);
	}

	return 0;
}

int log_uninit(void)
{
	va_list args;
	logg("EXIT ", "The program exited this log file ends here", args);
	if (logfile) {
		fclose(logfile);
	}
	return 0;
}
