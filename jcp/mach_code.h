/* mach_code.h -- machine code values */
/* ver. 1.0 */
#ifndef MACH_CODE_H
#define MACH_CODE_H

#define INSTR_STR 	8 	// max instruction size
#define GREGS		4	// 4 general registers
#define FLAGSN		9 	// 4 flags + 5 padding indices

enum {	LOAD, STORE,
		DATA,
		JMPR, JMP, JCOND,
		CLF,
		PAD, // padding 
		ADD, SHR, SHL, NOT, AND, OR, XOR, CMP,
		INSTR_COUNT};

// instruction information
typedef struct instr_ {
	int size;
	int code;
	char * name;//[INSTR_STR];
} instr;

extern instr mcode[INSTR_COUNT];
extern char * gregs[GREGS];
extern char * flags[FLAGSN];
#endif
