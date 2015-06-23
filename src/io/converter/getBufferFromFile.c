
// HEADER is 64 bytes
// image size is  1344 * 1024 * 16/8
short * getBufferFromFile(char *filename);
short * getBufferFromFile(char *filename)
{
	short * buffer = 0;
	long length;

	FILE *f = fopen(filename, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		length = ftell(f) - 64;	// substract header
		fseek(f, 64, SEEK_SET);	// 64bit offset for header
		buffer = malloc(length);
		if (buffer) {
			fread(buffer, 64, length, f);
		} else {
			printf("failed to read into buffer\n");
		}
		fclose(f);
	} else {
		printf("failed to read file\n");
	}

	return buffer;
}
