/*
 * See http://www.undertec.de/blog/2009/05/kbhit-und-getch-fur-linux.html
 */

#if defined(WIN)
#include<conio.h>
#else
#include<termios.h>
#include<stdio.h>
#include<string.h>
#include "kbhit.h"

int kbhit(void)
{
	struct termios term, oterm;
	int fd = 0;
	int c = 0;
	tcgetattr(fd, &oterm);
	memcpy(&term, &oterm, sizeof(term));
	term.c_lflag &= !ICANON;
	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 1;
	tcsetattr(fd, TCSANOW, &term);
	c = getchar();
	tcsetattr(fd, TCSANOW, &oterm);
	if (c != -1)
		ungetc(c, stdin);
	return ((c != -1) ? 1 : 0);
}

#endif
