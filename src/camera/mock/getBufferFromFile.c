/* HEADER is 64 bytes
 * image size is  1344 * 1024 * 16/8
 */
short *getBufferFromFile(char *filename);
short *getBufferFromFile(char *filename)
{
	short *buffer = NULL;
	int length;
	int read_bytes;

	FILE *f = fopen(filename, "rb");
	if (!f) {
		printf("failed to open file\n");
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	length = ftell(f) - 64;  // substract header
	if(length < 1){
		printf("file to small or unreadable\n");
		fclose(f);
		return NULL;
	}

	fseek(f, 64, SEEK_SET); // 64bit offset for header
	buffer = malloc(length);
	if (!buffer) {
		printf("failed to create buffer\n");
		free(buffer);
		fclose(f);
		return NULL;
	}

	read_bytes = fread(buffer, sizeof(char), length, f);
	if ( length != read_bytes * sizeof(char)) {
		printf("failed to read into buffer\n");
		free(buffer);
		fclose(f);
		return NULL;
	}

	fclose(f);

	return buffer;
}