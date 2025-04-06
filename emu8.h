#ifndef EMU8_H
#define EMU8_H

#define DEBUG printf


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;

typedef byte (*readbyte_fn)();
typedef void (*writebyte_fn)(byte);

struct rw_handler {
	readbyte_fn reader;
	writebyte_fn writer;
};

#include "console.h"
#include "disasm.h"
#include "i8080.h"
#include "ioports.h"
#include "machine.h"
#include "memory.h"

#endif  // EMU8_H
