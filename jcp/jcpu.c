/* jcpu.c -- emulator for the John Clark Scott's computer from "But How Do It Know?" */
/* ver.1.01 */

/* This is an emulator of the computer from the book "But How Do It Know?"
 * by John Clark Scott. Internally airthmetic and logic is done with the C 
 * operators rather than by simulating the whole system and the flags are represented 
 * as separate registers. Externally, it behaves as you would expect from the book. */

/* Author: Vladimir Dinev */
#include "jcpu.h"
#include "mach_code.h"

#define RA			0x0C	// & 0x0C for reg a
#define RB			0x03	// & 0x03 for reg b
#define BYTE_MAX	0xFF	// max byte value
#define get_ra_ir() (GREG_OFF + ((regs[IR] & RA) >> 2))	// get the index of reg a from IR
#define get_rb_ir() (GREG_OFF + (regs[IR] & RB))		// get the index of reg b from IR
#define get_instr()	(regs[IR] >> 4)						// get instruction nibble
#define set_zf()	(regs[ZF] = (regs[rb] == 0))		// set the zero flag

byte ram[RAM_S] = {0x00};		// the ram
byte regs[NUM_REGS] = {0x00};	// the registers
fpvv_t * func_arr; 				// pointer to a function pointer

static void load(void);
static void store(void);
static void data(void);
static void jmpr(void);
static void jmp(void);
static void jcond(void);
static void clearf(void);
static void dummy(void);
static void add(void);
static void shr(void);
static void shl(void);
static void not(void);
static void and(void);
static void or(void);
static void xor(void);
static void cmp(void);

void jcpu_load(const byte * code, int csize)
{
	/* initialize the function array
	 * load the code in ram */

	// an array of void function pointers
	static fpvv_t fa[INSTR_COUNT] = {	
		load, store,
		data,
		jmpr, jmp, jcond,
		clearf,
		dummy,
		add, shr, shl, not, and, or, xor, cmp
	};
		  
	func_arr = fa;

	int i;
	for (i = 0; i <= BYTE_MAX && i < csize; ++i)
		ram[i] = code[i];
	
	return;
}

void jcpu_step(void)
{
	/* CPU cycle */
	/* 1. move IAR to MAR
	 * 2. set IR to the value at the MAR address 
	 * 3. add one to IAR 
	 * 4, 5, 6 execute instruction */		
	regs[MAR] = regs[IAR];
	regs[IR] = ram[regs[MAR]];
	++regs[IAR];
	func_arr[get_instr()]();
	return;
}

static void load(void)
{
	/* LD RA, RB - loads RB from RAM address in RA
	 * 4. place RA in MAR 
	 * 5. place value at address in MAR in RB */
	regs[MAR] = regs[get_ra_ir()];
	regs[get_rb_ir()] = ram[regs[MAR]];
	return;
}

static void store(void)
{
	/* ST RA, RB - stores RB to RAM address in RA 
	 * 4. place RA in MAR
	 * 5. place RB at address in MAR */
	 regs[MAR] = regs[get_ra_ir()];
	 ram[regs[MAR]] = regs[get_rb_ir()];
	 return;
}

static void data(void)
{
	/* DATA RB - loads next byte as data in RB
	 * 4. send IAR to MAR
	 * 5. set the register to the value at MAR
	 * 6. add one to IAR */
	regs[MAR] = regs[IAR];
	regs[get_rb_ir()] = ram[regs[MAR]];
	++regs[IAR];
	return;
}

static void jmpr(void)
{
	/* JMPR RB - jumps to address in RB
	 * 4. set IAR to RB */
	regs[IAR] = regs[get_rb_ir()];
	return;
}

static void jmp(void)
{
	/* JMP addr - jumps to the address in the next byte
	 * 4. send IAR to MAR
	 * 5. move value at address in MAR to IAR */
	regs[MAR] = regs[IAR];
	regs[IAR] = ram[regs[MAR]];
	return;
}

static void jcond(void)
{
	/* J<flag(s)> addr - jumps to the address in the next byte when
	 * any of the requested flag bits is set
	 * 4. move IAR to MAR
	 * 5. add one to IAR
	 * 6. move the address from RAM to IAR if any of the requested flags is set */
	 byte flags = 0;
	 
	 regs[MAR] = regs[IAR];
	 ++regs[IAR];
	 
	 flags |= (regs[CF] << 3) | (regs[AF] << 2) | (regs[EF] << 1) | regs[ZF];
	 
	 if (regs[IR] & flags)
		regs[IAR] = ram[regs[MAR]];
	 
	return;
}

static void clearf(void)
{
	/* clear the flags */
	*((int *)&regs[CF]) = 0;
	return;
}

static void dummy(void)
{
	/* for array padding */
	return;
}

static void add(void)
{
	/* ADD RA, RB - adds the value in RA to the value in RB in RB
	 * modifies: CF, ZF
	 * step 0: get RA and RB
	 * step 1: add RA and RB in tmp
	 * step 2: add in the carry flag to tmp
	 * step 3: set the carry flag 
	 * step 4: move tmp to RB 
	 * step 5: set ZF */
	int ra = get_ra_ir();
	int rb = get_rb_ir();
	byte tmp = regs[ra] + regs[rb];
	
	tmp += regs[CF];
	regs[CF] = (regs[ra] + regs[rb]) > BYTE_MAX;
	regs[rb] = tmp;
	set_zf();
	return;
}

static void shr(void)
{
	/* SHR RA, RB - shifts RA one to the right into RB
	 * modifies: CF, ZF
	 * step 0: get RA and RB
	 * step 1: SHR RA in RB and | with CF << 7
	 * step 2: set CF  
	 * step 3: set ZF */
	int ra = get_ra_ir();
	int rb = get_rb_ir();
	unsigned int tmp = regs[ra];
	
	regs[rb] = (tmp >> 1) | (regs[CF] << 7);
	regs[CF] = ((tmp & 0x01) > 0);
	set_zf();
	return;
}

static void shl(void)
{
	/* SHL RA, RB - shifts RA one to the left into RB
	 * modifies: CF, ZF
	 * step 0: get RA and RB
	 * step 1: SHL RA in RB and | with CF
	 * step 2: set CF  
	 * step 3: set ZF */
	int ra = get_ra_ir();
	int rb = get_rb_ir();
	unsigned int tmp = regs[ra];

	regs[rb] = (tmp << 1) | regs[CF];
	regs[CF] = ( (tmp << 1) > BYTE_MAX );
	set_zf();
	return;
}

static void not(void)
{
	/* NOT RA, RB - sets RB to the reverse bits value of RA
	 * modifies: ZF
	 * step 0: get RA and RB 
	 * step 1: NOT RA in RB
	 * step 2: set ZF */
	int ra = get_ra_ir();
	int rb = get_rb_ir();
	
	regs[rb] = ~regs[ra];
	set_zf();
	return;
}

static void and(void)
{
	/* AND RA, RB - & RA and RB in RB
	 * modifies: ZF
	 * step 0: get RA and RB 
	 * step 1: and RA in RB
	 * step 2: set ZF */
	int ra = get_ra_ir();
	int rb = get_rb_ir();
	
	regs[rb] &= regs[ra];
	set_zf();
	return;
}

static void or(void)
{
	/* OR RA, RB - | RA and RB in RB
	 * modifies: ZF
	 * step 0: get RA and RB 
	 * step 1: or RA in RB
	 * step 2: set ZF */
	int ra = get_ra_ir();
	int rb = get_rb_ir();
	
	regs[rb] |= regs[ra];
	set_zf();
	return;
}

static void xor(void)
{
	/* XOR RA, RB - ^ RA and RB in RB
	 * modifies: ZF
	 * step 0: get RA and RB 
	 * step 1: xor RA in RB
	 * step 2: set ZF */
	int ra = get_ra_ir();
	int rb = get_rb_ir();
	
	regs[rb] ^= regs[ra];
	set_zf();
	return;
}

static void cmp(void)
{
	/* CMP RA, RB - compares RA and RB
	 * modifies: AF, EF
	 * step 0: get RA and RB 
	 * step 1: compare RA and RB
	 * step 2: set AF and EF */
	int ra = get_ra_ir();
	int rb = get_rb_ir();
	
	regs[AF] = regs[ra] > regs[rb];
	regs[EF] = regs[ra] == regs[rb];
	return; 
}
