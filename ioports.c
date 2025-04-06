#include "emu8.h"

struct rw_handler ioports_map[IOPORTS_NUM];


void ioports_handle(byte port, readbyte_fn reader, writebyte_fn writer)
{
	ioports_map[port] = (struct rw_handler){reader, writer};
}


byte ioports_read(byte port)
{
	readbyte_fn reader = ioports_map[port].reader;
	return reader ? reader() : 0;
}


void ioports_write(byte port, byte data)
{
	writebyte_fn writer = ioports_map[port].writer;
	if (writer)
		writer(data);
}
