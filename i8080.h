#ifndef I8080_H
#define I8080_H


struct i8080 {
	byte a;
	union { word bc; struct {byte c, b;}; };
	union { word de; struct {byte e, d;}; };
	union { word hl; struct {byte l, h;}; };
	word pc;
	word sp;

	bool auxcarry;
	bool carry;
	bool intr;
	bool parity;
	bool sign;
	bool zero;
};

extern struct i8080 i8080_cpu;

void i8080_reset();
int i8080_step();

#endif  // I8080_H
