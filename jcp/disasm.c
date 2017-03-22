/* disasm.c -- the disassembler engine */
/* ver. 1.01 */

/* Reads binary, outputs jcpu assembly language */

/* Author: Vladimir Dinev */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disasm.h"
#include "mach_code.h"

#define ROWS 			(256 + 4)	// four more for the screen
#define COLS 			32			// max instruction string width
#define INST_NBL		4			// >> 4 for the instruction nibble of a byte
#define FLAGS_NBL		0x0F		// & 0x0F for the flags nibble of a conditional jump
#define FLAG_C			0x08		// & 0x08 for the carry flag
#define FLAG_A			0x04		// & 0x04 for the a greater flag
#define FLAG_E			0x02		// & 0x02 for the equal flag
#define FLAG_Z			0x01		// & 0x01 for the zero flag
#define REG_A			0x0C		// & 0x0C to get reg a
#define REG_B			0x03		// & 0x03 to get reg b
#define get_next_byte()	sprintf(str_instr, pref_nopref, str_instr, code[*offset+1])

const char * pref_nopref;

// disassemble the next instruction
static char * diasm_get_instr(byte * code, int * offset);

char ** disasm_dis(byte * code, int size, int prefhex)
{
	/* place the disassembled result in disasm_code
	 * and return it's address */
	static char dcode[ROWS][COLS];
	static char * disasm_code[ROWS] = {NULL};
	static char * hexpr[] = {"%s %02X", "%s %#02X"};
	
	if (prefhex < NO_PREF || prefhex > PREF_HEX)
	{
		fprintf(stderr, "Err: module disasm: Invalid value for argument ");
		fprintf(stderr, "prefhex in function disasm_dis()\n");
		exit(EXIT_FAILURE);
	}
	
	pref_nopref = hexpr[prefhex];
	
	// mark end of code for the display module
	disasm_code[ROWS] = "--- --      -- --  --";
	disasm_code[ROWS+1] = "--- --      -- --  --";
	disasm_code[ROWS+2] = "--- --      -- --  --";
	disasm_code[ROWS+3] = "--- --      -- --  --";
	
	int i, ins_addr;
	for (i = 0; i < size;)
	{
		// Note: i gets incremented in diasm_get_instr()
		ins_addr = i;
		sprintf(dcode[ins_addr], "%02X> %s", ins_addr, diasm_get_instr(code, &i));
		disasm_code[ins_addr] = dcode[ins_addr];
	}
	
	return disasm_code;
}

static char * diasm_get_instr(byte * code, int * offset)
{
	/* build a string mnemonic */
	static char str_instr[COLS];
	
	// get higher nibble
	int inst_code = code[*offset] >> INST_NBL;
	// for reg a and reg b from lower nibble
	int rega = 0, regb = 0;
	int flgs;
	
	// get instruction code as string
	switch (mcode[inst_code].size)
	{
		case 1:
			sprintf(str_instr, "%02X     ", code[*offset]);
			break;
		case 2:
			sprintf(str_instr, "%02X  %02X ", code[*offset], code[*offset+1]);
			break;
		default:
			break;
	}
	
	// get instriction mnemonic in the string
	sprintf(str_instr, "%s %s", str_instr, mcode[inst_code].name);
	
	switch (inst_code)
	{
		case LOAD:
		case STORE:
		case ADD:
		case SHL:
		case SHR:
		case NOT:
		case AND:
		case OR:
		case XOR:
		case CMP:
			// get source - dest registers
			rega = code[*offset] & REG_A;
			regb = code[*offset] & REG_B;
			// get register strings
			sprintf(str_instr, "%s %s, %s", str_instr, gregs[rega >> 2], gregs[regb]);
			break;
		case DATA:
			// get dest register
			regb = code[*offset] & REG_B;
			sprintf(str_instr, "%s %s,", str_instr, gregs[regb]);
			// get load source
			get_next_byte();
			break;
		case JMP:
			// get jump address
			get_next_byte();
			break;
		case JMPR:
			// get dest register
			regb = code[*offset] & REG_B;
			sprintf(str_instr, "%s %s", str_instr, gregs[regb]);
			break;
		case JCOND:
			// get flags
			flgs = code[*offset] & FLAGS_NBL;
			strcat(str_instr, flags[flgs & FLAG_C]);
			strcat(str_instr, flags[flgs & FLAG_A]);
			strcat(str_instr, flags[flgs & FLAG_E]);
			strcat(str_instr, flags[flgs & FLAG_Z]);
			// get jump address
			get_next_byte();
			break;
		default:
			break;
	}

	*offset += mcode[inst_code].size;
	
	return str_instr;
}
