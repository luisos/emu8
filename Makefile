CC = gcc
CFLAGS := -O3
target := emu8
objects := console.o disasm.o i8080.o ioports.o machine.o main.o memory.o 

.PHONY: all clean

all: $(target)


$(target): $(objects)
	$(CC) $(CFLAGS) -o $@ $(objects)


*.o: console.h disasm.h emu8.h i8080.h ioports.h machine.h memory.h Makefile

install: $(target)
	cp $(target) ~/bin

clean:
	rm -f $(target) $(objects)

clang:
	rm -f $(target) $(objs)
	clang $(CFLAGS) -o $(target).clang $(objects)

gcc:
	rm -f $(target) $(objs)
	gcc $(CFLAGS) -o $(target).gcc $(objects)

pcc:
	rm -f $(target) $(objs)
	pcc $(CFLAGS) -o $(target).pcc $(objects)
