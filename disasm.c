#include <stdio.h>
#include "disasm.h"


char *mnemonics[256] = {
	"nop",  "lxi bc,", "stax bc", "inx bc", "inr b", "dcr b", "mvi b,", "rlc",
	"*nop", "dad bc",  "ldax bc", "dcx bc", "inr c", "dcr c", "mvi c,", "rrc",
	"*nop", "lxi de,", "stax de", "inx de", "inr d", "dcr d", "mvi d,", "ral",
	"*nop", "dad de",  "ldax de", "dcx de", "inr e", "dcr e", "mvi e,", "rar",
	"*nop", "lxi hl,", "shld ",   "inx hl", "inr h", "dcr h", "mvi h,", "daa",
	"*nop", "dad hl",  "lhld ",   "dcx hl", "inr l", "dcr l", "mvi l,", "cma",
	"*nop", "lxi sp,", "sta ",    "inx sp", "inr m", "dcr m", "mvi m,", "stc",
	"*nop", "dad sp",  "lda ",    "dcx sp", "inr a", "dcr a", "mvi a,", "cmc",

	"mov b,b", "mov b,c", "mov b,d", "mov b,e", "mov b,h", "mov b,l", "mov b,m", "mov b,a",
	"mov c,b", "mov c,c", "mov c,d", "mov c,e", "mov c,h", "mov c,l", "mov c,m", "mov c,a",
	"mov d,b", "mov d,c", "mov d,d", "mov d,e", "mov d,h", "mov d,l", "mov d,m", "mov d,a",
	"mov e,b", "mov e,c", "mov e,d", "mov e,e", "mov e,h", "mov e,l", "mov e,m", "mov e,a",
	"mov h,b", "mov h,c", "mov h,d", "mov h,e", "mov h,h", "mov h,l", "mov h,m", "mov h,a",
	"mov l,b", "mov l,c", "mov l,d", "mov l,e", "mov l,h", "mov l,l", "mov l,m", "mov l,a",
	"mov m,b", "mov m,c", "mov m,d", "mov m,e", "mov m,h", "mov m,l", "hlt",     "mov m,a",
	"mov a,b", "mov a,c", "mov a,d", "mov a,e", "mov a,h", "mov a,l", "mov a,m", "mov a,a",

	"add b", "add c", "add d", "add e", "add h", "add l", "add m", "add a",
	"adc b", "adc c", "adc d", "adc e", "adc h", "adc l", "adc m", "adc a",
	"sub b", "sub c", "sub d", "sub e", "sub h", "sub l", "sub m", "sub a",
	"sbb b", "sbb c", "sbb d", "sbb e", "sbb h", "sbb l", "sbb m", "sbb a",
	"ana b", "ana c", "ana d", "ana e", "ana h", "ana l", "ana m", "ana a",
	"xra b", "xra c", "xra d", "xra e", "xra h", "xra l", "xra m", "xra a",
	"ora b", "ora c", "ora d", "ora e", "ora h", "ora l", "ora m", "ora a",
	"cmp b", "cmp c", "cmp d", "cmp e", "cmp h", "cmp l", "cmp m", "cmp a",

	"rnz", "pop bc",  "jnz ", "jmp ",  "cnz ", "push bc",  "adi ", "rst 0",
	"rz",  "ret",     "jz ",  "*jmp ", "cz ",  "call ",    "aci ", "rst 1",
	"rnc", "pop de",  "jnc ", "out ",  "cnc ", "push de",  "sui ", "rst 2",
	"rc",  "*ret",    "jc ",  "in ",   "cc ",  "*call ",   "sbi ", "rst 3",
	"rpo", "pop hl",  "jpo ", "xthl",  "cpo ", "push hl",  "ani ", "rst 4",
	"rpe", "pchl",    "jpe ", "xchg",  "cpe ", "*call ",   "xri ", "rst 5",
	"rp",  "pop psw", "jp ",  "di",    "cp ",  "push psw", "ori ", "rst 6",
	"rm",  "sphl",    "jm ",  "ei",    "cm ",  "*call ",   "cpi ", "rst 7",
};

char opsize[256] = {
	0, 2, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  0, 0, 1, 0,
	0, 2, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  0, 0, 1, 0,
	0, 2, 2, 0,  0, 0, 1, 0,  0, 0, 2, 0,  0, 0, 1, 0,
	0, 2, 2, 0,  0, 0, 1, 0,  0, 0, 2, 0,  0, 0, 1, 0,

	0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
	0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
	0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
	0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,

	0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
	0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
	0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
	0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,

	0, 0, 2, 2,  2, 0, 1, 0,  0, 0, 2, 2,  2, 2, 1, 0,
	0, 0, 2, 1,  2, 0, 1, 0,  0, 0, 2, 1,  2, 2, 1, 0,
	0, 0, 2, 0,  2, 0, 1, 0,  0, 0, 2, 0,  2, 2, 1, 0,
	0, 0, 2, 0,  2, 0, 1, 0,  0, 0, 2, 0,  2, 2, 1, 0,
};


int disasm(unsigned char *mem)
{
	int opcode = *mem++;
	printf("%s", mnemonics[opcode]);
	int size = opsize[opcode];
	if (size == 1)
		printf("%02X", *mem);
	else if (size == 2)
		printf("%04X", *(unsigned short *)mem);
	putchar('\n');
	return size + 1;
}
