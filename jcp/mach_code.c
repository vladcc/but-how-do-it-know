/* mach_code.c -- maps the jcpu machine code to text mnemonics */
/* ver. 1.0 */
 
/* Instructions are mapped by their left nibble. 
 * The condition for the conditional jump instruction JCOND is 
 * specified by it's right nibble and the rest of the name is
 * added to the lone "J" at run time. */
 
 /* Author: Vladimir Dinev */
#include "mach_code.h"

// Note: the array is ordered by .code value
instr mcode[INSTR_COUNT] = {
		{.size = 1, .code = LOAD, .name = "LD"},
		{1, STORE, 	"ST"},
		{2, DATA, 	"DATA"}, 	// second byte is next byte in memory
		{1, JMPR, 	"JMPR"},
		{2, JMP, 	"JMP"},		// second byte is next byte in memory
		{2, JCOND,	"J"},		// second byte is next byte in memory
		{1, CLF, 	"CLF"},
		{1, PAD, 	"PAD"},		// dummy instruction for array padding
		{1, ADD, 	"ADD"},
		{1, SHR, 	"SHR"},
		{1, SHL, 	"SHL"},
		{1, NOT, 	"NOT"},
		{1, AND, 	"AND"},
		{1, OR, 	"OR"},
		{1, XOR, 	"XOR"},
		{1, CMP, 	"CMP"},
};

// register mnemonics
char * gregs[GREGS] = {"R0", "R1", "R2", "R3"};
char * flags[FLAGSN] = {"", "Z", "E", "", "A", "", "", "", "C"};
