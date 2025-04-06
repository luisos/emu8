#ifndef MEMORY_H
#define MEMORY_H

#define MEMORY_SIZE  0x10000  // 64K

extern byte memory_data[];

byte memory_read_byte(word addr);
word memory_read_word(word addr);
void memory_write_byte(word addr, byte val);
void memory_write_word(word addr, word val);
bool memory_load_file(char *binfile);
void memory_set_rom(word start, word end);

#endif  // MEMORY_H
