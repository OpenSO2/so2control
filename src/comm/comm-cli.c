#include "comm.h"
#include "configurations.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

int get_buffer(char *str, char **buf, int *size);

int main(int argc, char *argv[])
{
	sConfigStruct config;
	int stat = 0;
	int size = 0;
	int toggle = 0;
	int i = 0;
	char *a;
	char *buf;
	char spectrum[2048];
	char line[512];
	FILE *fid;

	if (argc != 2) {
		printf("usage: %s <port> \n", argv[0]);
		return 1;
	}

	buf = (char *)calloc(sizeof(char), 10);

	config.comm_port = atoi(argv[1]);

	stat = comm_init(&config);
	if (stat) {
		printf("failed to init comm\n");
		return -1;
	}

	while (1) {
		sleep(1);
		toggle = !toggle;

		a = toggle ? "fixtures/testing_2016_05_25-12_53_44_537_cam_top.raw.png" : "fixtures/testing_2016_05_25-12_53_59_657_cam_top.raw.png";
		stat = get_buffer(a, &buf, &size);
		if (stat) {
			printf("could not get buffer for file %s\n", a);
			goto fail;
		}
		comm_set_buffer("top", buf, size);

		a = toggle ? "fixtures/testing_2016_05_25-12_53_44_537_cam_bot.raw.png" : "fixtures/testing_2016_05_25-12_53_59_657_cam_bot.raw.png";
		stat = get_buffer(a, &buf, &size);
		if (stat) {
			printf("could not get buffer for file %s\n", a);
			goto fail;
		}
		comm_set_buffer("bot", buf, size);

		a = toggle ? "fixtures/testing_2016_05_25-12_53_44_537_cam_webcam.raw.png" : "fixtures/testing_2016_05_25-12_53_59_658_cam_webcam.raw.png";
		stat = get_buffer(a, &buf, &size);
		if (stat) {
			printf("could not get buffer for file %s\n", a);
			goto fail;
		}
		comm_set_buffer("cam", buf, size);

		a = "fixtures/test_spectrum.dat";
		fid = fopen(a, "r");
		if (!fid) {
			log_error("failed to open file");
			goto fail;
		}

		i = 0;
		while (fgets(line, 512, fid) != NULL) {
			spectrum[i++] = atol(line);
		}
		fclose(fid);

		comm_set_buffer("spc", spectrum, 2048);
	}

 fail:
	free(buf);

	return stat;
}

int get_buffer(char *str, char **buf, int *size)
{
	int stat = 0;

	FILE *fid = fopen(str, "r");
	if (!fid) {
		printf("failed to open file\n");
		return -1;
	}

	/* get filesize */
	fseek(fid, 0, SEEK_END);
	*size = ftell(fid);
	rewind(fid);

	/* allocate buffer */
	*buf = (char *)realloc(*buf, sizeof(char) * (*size));
	if (buf == NULL) {
		perror("error allocating enough memory");
		return -1;
	}

	stat = fread(*buf, 1, *size, fid);
	if (stat != *size) {
		perror("couldn't read mock file");
		return -1;
	}
	fclose(fid);

	return 0;
}
