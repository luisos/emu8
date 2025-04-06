#include "emu8.h"

byte memory_data[MEMORY_SIZE];
word rom_start;
word rom_end;


byte memory_read_byte(word addr)
{
	return memory_data[addr];
}


word memory_read_word(word addr)
{
	return memory_read_byte(addr) | (memory_read_byte(addr + 1) << 8);
}


void memory_write_byte(word addr, byte val) {
	if (rom_end == 0 || addr < rom_start || addr > rom_end)
		memory_data[addr] = val;
}


void memory_write_word(word addr, word val)
{
	memory_write_byte(addr, val & 0xff);
	memory_write_byte(addr + 1, val >> 8);
}


bool memory_load_file(char *binfile)
{
	FILE *fp = fopen(binfile, "r");
	if (!fp)
		return false;
	size_t n = fread(memory_data, 1, MEMORY_SIZE, fp);
	fclose(fp);
	return true;
}

void memory_set_rom(word start, word end)
{
	if (end > start) {
		rom_start = start;
		rom_end = end;
	} else {
		rom_start = end;
		rom_end = start;
	}
}
