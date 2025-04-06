#ifndef MACHINE_H
#define MACHINE_H


uint32_t machine_run(word start);
void machine_debug(word start);
void machine_dump();
bool machine_load_image(char *binfile, word start);
bool machine_load_rom(char *binfile, word start, word end);


#endif  // MACHINE_H

