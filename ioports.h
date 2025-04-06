#ifndef IOPORTS_H
#define IOPORTS_H


#define IOPORTS_NUM 256


extern struct rw_handler ioports_map[];


byte ioports_read(byte port);
void ioports_write(byte port, byte data);
void ioports_handle(byte port, readbyte_fn reader, writebyte_fn writer);

#endif  // IOPORTS_H
