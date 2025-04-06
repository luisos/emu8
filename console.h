#ifndef CONSOLE_H
#define CONSOLE_H

void console_init();
void console_restore();
void console_write(byte c);
byte console_read();
byte console_status();

#endif  // CONSOLE_H
