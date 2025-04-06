#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "emu8.h"

#define streq(s1, s2)  (!strcmp((str1), (str2)))


void die(const char *msg)
{
	perror(msg);
	exit(1);
}


void usage()
{
	puts("emu8 [-h] [-d] [-s] <binfile>");
	puts("\t-h show help");
	puts("\t-s show statistics");
	puts("\t-d debug mode");
}


int main(int argc, char **argv)
{
	bool badopt = false;
	bool opt_debug = false;
	bool opt_help = false;
	bool opt_statistics = false;

	char *binfile = NULL;

	if (argc == 1)
		badopt = true;

	while (--argc) {
		char *arg = *++argv;

		if (arg[0] != '-')
			binfile = arg;
		else if (!strcmp(arg, "-h"))
			opt_help = true;
		else if (!strcmp(arg, "-d"))
			opt_debug = true;
		else if (!strcmp(arg, "-s"))
			opt_statistics = true;
		else 
			badopt = true;
	}

	if (badopt || opt_help) {
		usage();
		return !badopt;
	}

	console_init();
	atexit(console_restore);
	signal(SIGTERM, console_restore);

	if (!binfile)
		die("No binfile");

	if (!machine_load_image(binfile, 0))
		die("Cant't open binary file");

	ioports_handle(0, console_status, NULL);
	ioports_handle(1, console_read, console_write);

	if (opt_debug) {
		machine_debug(0);
		return 0;
	}

	uint32_t cycles = machine_run(0);
	if (opt_statistics)
		printf("\nCYCLES %u\n\n", cycles);

	return 0;
}

