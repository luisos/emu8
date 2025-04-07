#include "emu8.h"
#include "console.h"
#include <ctype.h>


uint32_t machine_cycles;


uint32_t machine_run(word start)
{
	i8080_reset();
	i8080_cpu.pc = start;
	machine_cycles = 0;

	for (;;) {
		int c = i8080_step();
		if (c == 0)
			break;
		machine_cycles += c;
	}
	return machine_cycles;
}


static char* skip_spaces(const char *s)
{
	char *p = (char*)s;
	while (isspace(*p))
		p++;
	return p;
}


static void clear_buffer()
{
	while (getchar() != '\n')
		;
}


static void dump_mem(int offset)
{
	offset &= 0xffff;
	int addr = offset - offset % 16;
	for (int i = 0; i < 16; i++) {
		printf("%04x: ", addr);
		for (int j = 0; j < 16; j++) {
			printf(" %02hhx", memory_data[addr + j]);
		}
		printf("  ");
		for (int j = 0; j < 16; j++) {
			char c = memory_data[addr + j];
			putchar(c >= ' ' ? c : '.');
		}
		putchar('\n');
		addr += 16;
	}

}


static void debug_help()
{
	puts("Debug keys:");
	puts("c - continue");
	puts("d - dump <address>");
	puts("g - goto <address>");
	puts("n - next");
	puts("q - quit");
	puts("r - show registers\n");
}


void machine_debug(word start)
{
	i8080_reset();
	i8080_cpu.pc = start;
	machine_cycles = 0;
	unsigned addr = 0;
	size_t n;
	char cmd[128];
	bool stop = false;
	int ncycles;

	debug_help();
	machine_dump();

	while (!stop) {
		console_restore();
		putchar(':');
		//ssize_t r = getline(&cmd, &cmdsize, stdin);
		int c = getchar();
		console_init();
		char *p = skip_spaces(cmd);

		switch (tolower(c)) {
		case 'q':
			stop = true;
			break;
		case 'c': 
			do {
				ncycles = i8080_step();
				machine_cycles += ncycles;
			} while (ncycles != 0);
			machine_dump();
			stop = true;
			break;

		case 'r':
			machine_dump();
			break;
		case 'g':
			addr = strtol(skip_spaces(p + 1), NULL, 16);
			while (i8080_cpu.pc != addr) {
				ncycles = i8080_step();
				machine_cycles += ncycles;
				if (ncycles == 0) {
					stop = 0;
					break;
				}
			}
			machine_dump();
			break;
		case '\r':
		case 'n':
			putchar('\r');
			ncycles = i8080_step();
			machine_cycles += ncycles;
			machine_dump();
			if (ncycles == 0)
				stop = 0;
			break;
		case 'd':
			console_restore();
			printf("d ");
			//addr = strtol(skip_spaces(p + 1), NULL, 16);
			n = scanf("%x", &addr);
			dump_mem(addr);
			clear_buffer();
			console_init();
			break;
		default:
			putchar(c);
			puts("Bad command");
		}
	}
	printf("Total cycles: %u\n", machine_cycles);
}


void machine_dump()
{
	putchar(i8080_cpu.carry ? 'c' : '.');
	putchar(i8080_cpu.zero ? 'z' : '.');
	putchar(i8080_cpu.sign ? 's' : '.');
	putchar(i8080_cpu.parity ? 'p' : '.');
	putchar(i8080_cpu.auxcarry ? 'a' : '.');

	printf(" A=%02x BC=%04x DE=%04x HL=%04x SP=%04x PC=%04x  ",
	       i8080_cpu.a, i8080_cpu.bc, i8080_cpu.de,
	       i8080_cpu.hl, i8080_cpu.sp, i8080_cpu.pc);

	// printf(" cycles=%-8lu ", machine_cycles);
	disasm(memory_data + i8080_cpu.pc);
}


bool machine_load_image(char *binfile, word start)
{
	FILE *fp = fopen(binfile, "r");
	if (!fp)
		return false;
	size_t n = fread(memory_data + start, 1, MEMORY_SIZE - start, fp);
	fclose(fp);
	return true;
}


bool machine_load_rom(char *binfile, word start, word end)
{
	FILE *fp = fopen(binfile, "r");
	if (!fp)
		return false;

	if (start > end) {
		word t = start;
		start = end;
		end = t;
	}
	size_t n = fread(memory_data + start, 1, end - start + 1, fp);
	fclose(fp);
	memory_set_rom(start, end);
	return true;
}
