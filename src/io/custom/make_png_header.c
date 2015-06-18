/*
 * Create png header string from input string.
 *
 * yes, yes, png doesn't have headers, but ancillary chunks. But that
 * would sound stupid.
 * Every header is 60byte long, which equals 48byte for content
 * (4 bytes for length, 4 bytes for type, 4 bytes for crc).
 * Content has to be in the form:
 * Keyword (character string) + Null + Text string
 *
 * Keywords (copied from http://www.w3.org/TR/PNG/#11keywords):
 *   Title          Short (one line) title or caption for image
 *   Author         Name of image's creator
 *   Description    Description of image (possibly long)
 *   Copyright      Copyright notice
 *   Creation Time  Time of original image creation
 *   Software       Software used to create the image
 *   Disclaimer     Legal disclaimer
 *   Warning        Warning of nature of content
 *   Source         Device used to create the image
 *   Comment        Miscellaneous comment
 */

/* we need zlib for crc calculation*/
#include <zlib.h>

void make_png_header(char *content, int content_length, int *header, int header_length);
void make_png_header(char *content, int content_length, int *header, int header_length)
{
	int i;

	int crc;
	unsigned char *crcbytes;
	Bytef text[header_length - 8];

	/*
	 * Length of data: ONLY data, not type code, length itself or crc
	 * header_length - 4(type code) - 4(crc) - 4(type code) = header_length-12
	 */

	header[0] = 0;
	header[1] = 0;
	header[2] = 0;
	header[3] = header_length - 12;

	/* chunk type code: tEXt = 116 69 88 116 */
	header[4] = (int)'t';
	header[5] = (int)'E';
	header[6] = (int)'X';
	header[7] = (int)'t';

	/* content */
	for (i = 0; i < content_length + 14; i++) {
		header[i + 8] = (int)content[i];
	}
	for (i = content_length + 8; i < header_length; i++) {
		header[i] = (int)' '; /* write spaces into the rest */
	}

	/* CRC-32 of chunk type code and chunk data fields, but not crc itself or length (77-8) */
	for (i = 0; i < header_length - 8; i++)
		text[i] = (int)header[i + 4];
	crc = crc32(0L, Z_NULL, 0);
	crc = crc32(crc, text, header_length - 8);
	crcbytes = (unsigned char *)&crc;

	header[header_length - 4] = (int)crcbytes[3];
	header[header_length - 3] = (int)crcbytes[2];
	header[header_length - 2] = (int)crcbytes[1];
	header[header_length - 1] = (int)crcbytes[0];
}
