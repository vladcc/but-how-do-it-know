/* jcpu.h -- public interface for jcpu.c */
/* ver. 1.02 */
#ifndef JCPU_H
#define JCPU_H

/* fpvv_t = function pointer void void type
 * a function pointer to a void function */
typedef void (*fpvv_t)(void);
typedef unsigned char byte;

#define RAM_S 		256	// size of ram
#define GREG_OFF	7	// offset to r0 in the registers array
enum {MAR, IAR, IR, CF, AF, EF, ZF, R0, R1, R2, R3, NUM_REGS};

extern byte ram[RAM_S];
extern byte regs[NUM_REGS];

void jcpu_load(const byte * code, int csize);
/* returns: Nothing.
 * 
 * description: Loads the code array in the jcpu ram. */

void jcpu_step(void);
/* returns: Nothing.
 * 
 * description: Executes a single cpu instruction. */
#endif
