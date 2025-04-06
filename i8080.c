#include "emu8.h"

static word add16(word w1, word w2);
static byte add8(byte b1, byte b2);
static void check_flags(byte val);
static void daa();
static byte dec8(byte val);
static word get_psw();
static byte inc8(byte val);
static void rotate_left(bool fromcarry);
static void rotate_right(bool fromcarry);
static void set_psw(word psw);
static byte sub8(byte b1, byte);

#define read8  memory_read_byte
#define read16  memory_read_word
#define write8  memory_write_byte
#define write16  memory_write_word
#define fetch8()  read8(PC++)
#define fetch16()  (fetch8() | (fetch8()<<8))

struct i8080 i8080_cpu;

#define A  i8080_cpu.a
#define B  i8080_cpu.b
#define C  i8080_cpu.c
#define D  i8080_cpu.d
#define E  i8080_cpu.e
#define H  i8080_cpu.h
#define L  i8080_cpu.l
#define M  read8(HL)
#define BC  i8080_cpu.bc
#define DE  i8080_cpu.de
#define HL  i8080_cpu.hl
#define PC  i8080_cpu.pc
#define SP  i8080_cpu.sp
#define AUXCARRY  i8080_cpu.auxcarry
#define CARRY  i8080_cpu.carry
#define INTR  i8080_cpu.intr
#define PARITY  i8080_cpu.parity
#define SIGN  i8080_cpu.sign
#define ZERO  i8080_cpu.zero



void i8080_reset()
{
	i8080_cpu = (struct i8080){0};
}

#define ACI()  ADC(fetch8())
#define ADC(r8)  ADD(r8 + CARRY)
#define ADD(r8)  A = add8(A, (r8))
#define ADI()  ADD(fetch8())
#define ANA(r8)  A &= r8; check_flags(A); CARRY = false
#define ANI()  ANA(fetch8())
#define CALL()  PUSH(PC + 2); PC = fetch16() 
#define CALL_IF(cond)  if (cond) {CALL();} else {PC += 2; ncycles -= 6;}
#define CC()  CALL_IF(CARRY)
#define CM()  CALL_IF(SIGN)
#define CMA()  A = ~A
#define CMC()  CARRY=!CARRY
#define CMP(r8)  sub8(A, (r8))
#define CNC()  CALL_IF(!CARRY)
#define CNZ()  CALL_IF(!ZERO)
#define CP()  CALL_IF(!SIGN)
#define CPE()  CALL_IF(PARITY)
#define CPI()  CMP(fetch8())
#define CPO()  CALL_IF(!PARITY)
#define CZ()  CALL_IF(ZERO)
#define DAA()  daa()
#define DAD(r16)  HL = add16(HL, r16)
#define DCR(r8)  r8 = dec8(r8)
#define DCR_M()  write8(HL, dec8(M))
#define DCX(r16)  r16--
#define DI()  INTR = false
#define EI()  INTR = true
#define HLT()  return 0
#define IN()  A = ioports_read(fetch8())
#define INR(r8)  r8 = inc8(r8)
#define INR_M()  write8(HL, inc8(M))
#define INX(r16)  r16++
#define JC()  JMP_IF(CARRY)
#define JM()  JMP_IF(SIGN)
#define JMP()  PC = fetch16()
#define JMP_IF(cond)  if (cond) {JMP();} else {PC += 2; ncycles -=6;}
#define JNC()  JMP_IF(!CARRY)
#define JNZ()  JMP_IF(!ZERO)
#define JP()  JMP_IF(!SIGN)
#define JPE()  JMP_IF(PARITY)
#define JPO()  JMP_IF(!PARITY)
#define JZ()  JMP_IF(ZERO)
#define LDA()  A = read8(fetch16())
#define LDAX(r16)  A = read8(r16)
#define LHLD()  HL = read16(fetch16())
#define LXI(r16)  r16 = fetch16()
#define MOV(rd, rs)  rd = rs
#define MOV_M(r8)  write8(HL, r8)
#define MVI(r8)  r8 = fetch8()
#define MVI_M()  write8(HL, fetch8())
#define NOP()
#define ORA(r8)  A |= r8; check_flags(A); CARRY = false
#define ORI()  ORA(fetch8())
#define OUT()  ioports_write(fetch8(), A)
#define PCHL()  PC = HL
#define POP(r16)  r16 = read16(SP); SP += 2
#define POP_PSW()  set_psw(read16(SP)); SP += 2
#define PUSH(r16)  SP -= 2; write16(SP, r16)
#define PUSH_PSW()  PUSH(get_psw())
#define RAL()  rotate_left(true)
#define RAR()  rotate_right(true)
#define RET()  POP(PC)
#define RET_IF(cond)  if (cond) {RET();} else {ncycles -= 6;}
#define RLC()  rotate_left(false)
#define RM()  RET_IF(SIGN)
#define RNC()  RET_IF(!CARRY)
#define RNZ()  RET_IF(!ZERO)
#define RPE()  RET_IF(PARITY)
#define RP()  RET_IF(!SIGN)
#define RPO()  RET_IF(!PARITY)
#define RRC()  rotate_right(false)
#define RST(n)  PUSH(PC); PC = (n << 3)
#define RC()  RET_IF(CARRY)
#define RZ()  RET_IF(ZERO)
#define SBB(r8)  SUB(r8 + CARRY)
#define SBI()  SBB(fetch8())
#define SHLD()  write16(fetch16(), HL)
#define SPHL()  SP = HL
#define STA()  write8(fetch16(), A)
#define STAX(r16)  write8(r16, A)
#define STC()  CARRY = true
#define SUB(r8)  A = sub8(A, (r8))
#define SUI()  SUB(fetch8())
#define XRA(r8)  A ^= r8; check_flags(A); CARRY = false
#define XRI()  XRA(fetch8())
#define XCHG()  { word w = DE; DE = HL; HL = w; }
#define XTHL()  { word w = read16(SP); write16(SP, HL); HL = w; }

byte opcode_cycles[256] = {
	 4, 10,  7,  5,  5,  5,  7,  4,  4, 10,  7,  5,  5,  5,  7,  4,
	 4, 10,  7,  5,  5,  5,  7,  4,  4, 10,  7,  5,  5,  5,  7,  4,
	 4, 10, 16,  5,  5,  5,  7,  4,  4, 10, 16,  5,  5,  5,  7,  4,
	 4, 10, 13,  5, 10, 10, 10,  4,  4, 10, 13,  5,  5,  5,  7,  4,

	 5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,
	 5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,
	 5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,
	 7,  7,  7,  7,  7,  7,  7,  7,  5,  5,  5,  5,  5,  5,  7,  5,

	 4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
	 4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
	 4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
	 4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,

	11, 10, 10, 10,  17, 11, 7, 11, 11, 10, 10, 10, 17, 17,  7, 11,
	11, 10, 10, 10,  17, 11, 7, 11, 11, 10, 10, 10, 17, 17,  7, 11,
	11, 10, 10, 18,  17, 11, 7, 11, 11,  5, 10,  5, 17, 17,  7, 11,
	11, 10, 10,  4,  17, 11, 7, 11, 11,  5, 10,  4, 17, 17,  7, 11,
	
};

int i8080_step() {
	int opcode = fetch8();
	int ncycles = opcode_cycles[opcode];

	switch (opcode) {
	case 0x00: NOP(); break;
	case 0x01: LXI(BC); break;
	case 0x02: STAX(BC); break;
	case 0x03: INX(BC); break;
	case 0x04: INR(B); break;
	case 0x05: DCR(B); break;
	case 0x06: MVI(B); break;
	case 0x07: RLC(); break;
	case 0x08: NOP(); break;
	case 0x09: DAD(BC); break;
	case 0x0a: LDAX(BC); break;
	case 0x0b: DCX(BC); break;
	case 0x0c: INR(C); break;
	case 0x0d: DCR(C); break;
	case 0x0e: MVI(C); break;
	case 0x0f: RRC(); break;

	case 0x10: NOP(); break;
	case 0x11: LXI(DE); break;
	case 0x12: STAX(DE); break;
	case 0x13: INX(DE); break;
	case 0x14: INR(D); break;
	case 0x15: DCR(D); break;
	case 0x16: MVI(D); break;
	case 0x17: RAL(); break;
	case 0x18: NOP(); break;
	case 0x19: DAD(DE); break;
	case 0x1a: LDAX(DE); break;
	case 0x1b: DCX(DE); break;
	case 0x1c: INR(E); break;
	case 0x1d: DCR(E); break;
	case 0x1e: MVI(E); break;
	case 0x1f: RAR(); break;

	case 0x20: NOP(); break;
	case 0x21: LXI(HL); break;
	case 0x22: SHLD(); break;
	case 0x23: INX(HL); break;
	case 0x24: INR(H); break;
	case 0x25: DCR(H); break;
	case 0x26: MVI(H); break;
	case 0x27: DAA(); break;
	case 0x28: NOP(); break;
	case 0x29: DAD(HL); break;
	case 0x2a: LHLD(); break;
	case 0x2b: DCX(HL); break;
	case 0x2c: INR(L); break;
	case 0x2d: DCR(L); break;
	case 0x2e: MVI(L); break;
	case 0x2f: CMA(); break;
	
	case 0x30: NOP(); break;
	case 0x31: LXI(SP); break;
	case 0x32: STA(); break;
	case 0x33: INX(SP); break;
	case 0x34: INR_M(); break;
	case 0x35: DCR_M(); break;
	case 0x36: MVI_M(); break;
	case 0x37: STC(); break;
	case 0x38: NOP(); break;
	case 0x39: DAD(SP); break;
	case 0x3a: LDA(); break;
	case 0x3b: DCX(SP); break;
	case 0x3c: INR(A); break;
	case 0x3d: DCR(A); break;
	case 0x3e: MVI(A); break;
	case 0x3f: CMC(); break;

	case 0x40: MOV(B,B); break;
	case 0x41: MOV(B,C); break;
	case 0x42: MOV(B,D); break;
	case 0x43: MOV(B,E); break;
	case 0x44: MOV(B,H); break;
	case 0x45: MOV(B,L); break;
	case 0x46: MOV(B,M); break;
	case 0x47: MOV(B,A); break;
	case 0x48: MOV(C,B); break;
	case 0x49: MOV(C,C); break;
	case 0x4a: MOV(C,D); break;
	case 0x4b: MOV(C,E); break;
	case 0x4c: MOV(C,H); break;
	case 0x4d: MOV(C,L); break;
	case 0x4e: MOV(C,M); break;
	case 0x4f: MOV(C,A); break;

	case 0x50: MOV(D,B); break;
	case 0x51: MOV(D,C); break;
	case 0x52: MOV(D,D); break;
	case 0x53: MOV(D,E); break;
	case 0x54: MOV(D,H); break;
	case 0x55: MOV(D,L); break;
	case 0x56: MOV(D,M); break;
	case 0x57: MOV(D,A); break;
	case 0x58: MOV(E,B); break;
	case 0x59: MOV(E,C); break;
	case 0x5a: MOV(E,D); break;
	case 0x5b: MOV(E,E); break;
	case 0x5c: MOV(E,H); break;
	case 0x5d: MOV(E,L); break;
	case 0x5e: MOV(E,M); break;
	case 0x5f: MOV(E,A); break;

	case 0x60: MOV(H,B); break;
	case 0x61: MOV(H,C); break;
	case 0x62: MOV(H,D); break;
	case 0x63: MOV(H,E); break;
	case 0x64: MOV(H,H); break;
	case 0x65: MOV(H,L); break;
	case 0x66: MOV(H,M); break;
	case 0x67: MOV(H,A); break;
	case 0x68: MOV(L,B); break;
	case 0x69: MOV(L,C); break;
	case 0x6a: MOV(L,D); break;
	case 0x6b: MOV(L,E); break;
	case 0x6c: MOV(L,H); break;
	case 0x6d: MOV(L,L); break;
	case 0x6e: MOV(L,M); break;
	case 0x6f: MOV(L,A); break;

	case 0x70: MOV_M(B); break;
	case 0x71: MOV_M(C); break;
	case 0x72: MOV_M(D); break;
	case 0x73: MOV_M(E); break;
	case 0x74: MOV_M(H); break;
	case 0x75: MOV_M(L); break;
	case 0x76: HLT(); break;
	case 0x77: MOV_M(A); break;

	case 0x78: MOV(A,B); break;
	case 0x79: MOV(A,C); break;
	case 0x7a: MOV(A,D); break;
	case 0x7b: MOV(A,E); break;
	case 0x7c: MOV(A,H); break;
	case 0x7d: MOV(A,L); break;
	case 0x7e: MOV(A,M); break;
	case 0x7f: MOV(A,A); break;

	case 0x80: ADD(B); break;
	case 0x81: ADD(C); break;
	case 0x82: ADD(D); break;
	case 0x83: ADD(E); break;
	case 0x84: ADD(H); break;
	case 0x85: ADD(L); break;
	case 0x86: ADD(M); break;
	case 0x87: ADD(A); break;

	case 0x88: ADC(B); break;
	case 0x89: ADC(C); break;
	case 0x8a: ADC(D); break;
	case 0x8b: ADC(E); break;
	case 0x8c: ADC(H); break;
	case 0x8d: ADC(L); break;
	case 0x8e: ADC(M); break;
	case 0x8f: ADC(A); break;

	case 0x90: SUB(B); break;
	case 0x91: SUB(C); break;
	case 0x92: SUB(D); break;
	case 0x93: SUB(E); break;
	case 0x94: SUB(H); break;
	case 0x95: SUB(L); break;
	case 0x96: SUB(M); break;
	case 0x97: SUB(A); break;
	case 0x98: SBB(B); break;
	case 0x99: SBB(C); break;
	case 0x9a: SBB(D); break;
	case 0x9b: SBB(E); break;
	case 0x9c: SBB(H); break;
	case 0x9d: SBB(L); break;
	case 0x9e: SBB(M); break;
	case 0x9f: SBB(A); break;

	case 0xa0: ANA(B); break;
	case 0xa1: ANA(C); break;
	case 0xa2: ANA(D); break;
	case 0xa3: ANA(E); break;
	case 0xa4: ANA(H); break;
	case 0xa5: ANA(L); break;
	case 0xa6: ANA(M); break;
	case 0xa7: ANA(A); break;
	case 0xa8: XRA(B); break;
	case 0xa9: XRA(C); break;
	case 0xaa: XRA(D); break;
	case 0xab: XRA(E); break;
	case 0xac: XRA(H); break;
	case 0xad: XRA(L); break;
	case 0xae: XRA(M); break;
	case 0xaf: XRA(A); break;

	case 0xb0: ORA(B); break;
	case 0xb1: ORA(C); break;
	case 0xb2: ORA(D); break;
	case 0xb3: ORA(E); break;
	case 0xb4: ORA(H); break;
	case 0xb5: ORA(L); break;
	case 0xb6: ORA(M); break;
	case 0xb7: ORA(A); break;
	case 0xb8: CMP(B); break;
	case 0xb9: CMP(C); break;
	case 0xba: CMP(D); break;
	case 0xbb: CMP(E); break;
	case 0xbc: CMP(H); break;
	case 0xbd: CMP(L); break;
	case 0xbe: CMP(M); break;
	case 0xbf: CMP(A); break;

	case 0xc0: RNZ(); break;
	case 0xc1: POP(BC); break;
	case 0xc2: JNZ(); break;
	case 0xc3: JMP(); break;
	case 0xc4: CNZ(); break;
	case 0xc5: PUSH(BC); break;
	case 0xc6: ADI(); break;
	case 0xc7: RST(0); break;
	case 0xc8: RZ(); break;
	case 0xc9: RET(); break;
	case 0xca: JZ(); break;
	case 0xcb: JMP(); break;
	case 0xcc: CZ(); break;
	case 0xcd: CALL(); break;
	case 0xce: ACI(); break;
	case 0xcf: RST(1); break;

	case 0xd0: RPO(); break;
	case 0xd1: POP(DE); break;
	case 0xd2: JNC(); break;
	case 0xd3: OUT(); break;
	case 0xd4: CNC(); break;
	case 0xd5: PUSH(DE); break;
	case 0xd6: SUI(); break;
	case 0xd7: RST(2); break;
	case 0xd8: RC(); break;
	case 0xd9: RET(); break;
	case 0xda: JC(); break;
	case 0xdb: IN(); break;
	case 0xdc: CC(); break;
	case 0xdd: CALL(); break;
	case 0xde: SBI(); break;
	case 0xdf: RST(3); break;

	case 0xe0: RNC(); break;
	case 0xe1: POP(HL); break;
	case 0xe2: JPO(); break;
	case 0xe3: XTHL(); break;
	case 0xe4: CPO(); break;
	case 0xe5: PUSH(HL); break;
	case 0xe6: ANI(); break;
	case 0xe7: RST(4); break;
	case 0xe8: RPE(); break;
	case 0xe9: PCHL(); break;
	case 0xea: JPE(); break;
	case 0xeb: XCHG(); break;
	case 0xec: CPE(); break;
	case 0xed: CALL(); break;
	case 0xee: XRI(); break;
	case 0xef: RST(5); break;

	case 0xf0: RP(); break;
	case 0xf1: POP_PSW(); break;
	case 0xf2: JP(); break;
	case 0xf3: DI(); break;
	case 0xf4: CP(); break;
	case 0xf5: PUSH_PSW(); break;
	case 0xf6: ORI(); break;
	case 0xf7: RST(6); break;
	case 0xf8: RM(); break;
	case 0xf9: SPHL(); break;
	case 0xfa: JM(); break;
	case 0xfb: EI(); break;
	case 0xfc: CM(); break;
	case 0xfd: CALL(); break;
	case 0xfe: CPI(); break;
	case 0xff: RST(7); break;
	}
	return ncycles;
}


static void check_flags(byte val)
{
	ZERO = !val;
	SIGN = val >> 7;

	val ^= (val >> 1);
	val ^= (val >> 2);
	val ^= (val >> 4);
	PARITY = !(val & 1);
}


static byte add8(byte b1, byte b2)
{
	int r = b1 + b2;
	CARRY = r > 0xff;
	AUXCARRY = ((b1 & 0xf) + (b2 & 0xf)) > 0x0f;
	check_flags(r);
	return r;
}


static byte sub8(byte b1, byte b2)
{
	int r = b1 - b2;
	CARRY = r < 0;
	AUXCARRY = (b1 & 0xf) < (b2 & 0xf);
	check_flags(r);
	return r;
}


static word add16(word w1, word w2)
{
	dword sum = w1 + w2;
	CARRY = sum >> 16;
	return sum;
}

static void rotate_left(bool fromcarry)
{
	int c = fromcarry ? CARRY : (A >> 7);
	CARRY = A >> 7;
	A = (A << 1) | c;
}


static void rotate_right(bool fromcarry)
{
	int c = fromcarry ? CARRY : (A & 1);
	CARRY = A & 1;
	A = (A >> 1) | (c << 7);
}


static byte inc8(byte val)
{
	check_flags(++val);
	return val;
}


static byte dec8(byte val)
{
	check_flags(--val);
	return val;
}


static void daa()
{
	if (AUXCARRY || (A & 0xf) > 9) {
		bool c = CARRY;
		A = add8(A, 6);
		CARRY = c;
	}
	if (CARRY || (A >> 4) > 9)
		A = add8(A, 0x60);
}


static word get_psw() {
	return 0x02
		| CARRY
		| (PARITY << 2)
		| (AUXCARRY << 4)
		| (ZERO << 6)
		| (SIGN << 7)
		| (A << 8);
}

static void set_psw(word psw) {
	A = psw >> 8;
	CARRY = psw & 1;
	PARITY = psw & 4;
	AUXCARRY = psw & 0x10;
	ZERO = psw & 0x40;
	SIGN = psw & 0x80;
}

