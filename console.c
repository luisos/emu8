#include "emu8.h"

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

struct termios saved_term;


void console_init()
{
	tcgetattr(STDIN_FILENO, &saved_term);
	struct termios term = saved_term;
	term.c_iflag &= ~(ICRNL);
	term.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
}


void console_restore()
{
	tcsetattr(STDIN_FILENO, TCSANOW, &saved_term);
}


void console_write(byte c)
{
	putchar(c);
	fflush(stdout);
}


byte console_read()
{
	return getchar();
}


byte console_status()
{
	int nbytes;
	ioctl(0, FIONREAD, &nbytes);
	return nbytes > 0 ? 0xff : 0;
}

