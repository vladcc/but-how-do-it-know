/* display.c -- provides display functionality for the jcpvm */
/* ver. 1.01 */

/* Creates a frame buffer and fills it with what
 * represents the current machine state of the jcpu. 
 * A new frame gets created after every interactive step. */

/* Author: Vladimir Dinev */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "os_def.h"
#include "display.h"
#include "disasm.h"
#include "jcpu.h"
#include "mach_code.h"

#define BYTE_CELL	4		// cells conatining the ram info are 4 chars wide
#define RAM_LINE	2		// the ram begins at row index 2 of the frame buffer
#define REG_LINE	2		// registers begin at the same line as the ram
#define CODE_LINE	20		// disassembled code begins at line index 20
#define INSTR_NUM	4		// the number of disassembled instructions minus the last executed
#define INSTR_MAX	256		// maximum number of instructions to disassemble
#define MARK_IAR	'@'		// '@' marks the address pointed to by IAR
#define MARK_MAR	'*'		// '*' marks the address pointed to by MAR

char frame[FRAME_ROWS][FRAME_COLS];	// the frame buffer
char ** disasm_str;					// a pointer to an array of strings; holds the disasm text

static void make_frame(int hex_dec, int last_instr);
static void do_ram(int hex_dec);
static void do_code(int last_instr);
static void do_regs(int hex_dec);

void disp_clear(void)
{
	/* clear the terminal
	 * 1st of 2 system specific functions */
#ifdef WINDOWS
	system("cls");
#else
	system("clear");
#endif
	return;
}

void disp_init_frame(void)
{
	/* put constant strings in the frame */
	int i;
	
	sprintf(&frame[0][0],  
	"    00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F");
	sprintf(&frame[1][0],
	"    ______________________________________________________________");
	
	for (i = 0; i <= 0x0F; ++i)
		sprintf(&frame[2+i][0], "%02X|%63s  ", i << 4, " ");
	
	// disassemble the whole ram
	disasm_str = disasm_dis(ram, sizeof(ram));
	
	return;
}

void disp_print(int hex_dec, int last_instr)
{
	/* print the frame */
	int row;
	
	make_frame(hex_dec, last_instr);
	
	for (row = 0; row < FRAME_ROWS; ++row)
		printf("%s\n", frame[row]);

	return;
}

void disp_move_cursor_xy(int row, int col)
{
	/* move the console cursor to row, col 
	 * 2nd of 2 system specific functions */
#ifdef WINDOWS
	HANDLE h;
	COORD c;
	
	h = GetStdHandle(STD_OUTPUT_HANDLE);
	c.X = col;
	c.Y = row;
	SetConsoleCursorPosition(h, c);
#else
	printf("\e[%d;%dH", row, col);
#endif
}

static void make_frame(int hex_dec, int last_instr)
{
	/* assemble the frame */
	do_ram(hex_dec);
	do_code(last_instr);
	do_regs(hex_dec);
	
	int row = regs[MAR] >> 4;
	int col = regs[MAR] & 0x0F;
	
	// mark current ram address
	frame[2 + row][2 + (4 * col) + 1] = MARK_MAR;
	
	row = regs[IAR] >> 4;
	col = regs[IAR] & 0x0F;
	
	// mark current instruction address
	frame[2 + row][2 + (4 * col) + 1] = MARK_IAR;
	
	return;
}

static void do_ram(int hex_dec)
{
	/* place ram values in the frame */
	int i;
	char * pf;
	static char * ram_base[] = {" %02X ", " %-3d"};
	
	for (i = 0; i < RAM_S; ++i, pf += BYTE_CELL)
	{
		if ((i % 16) == 0)
			pf = &frame[i/16+RAM_LINE][BYTE_CELL-1];
		
		sprintf(pf, ram_base[hex_dec], ram[i]);
	}
		
	return;
}

static void do_code(int last_instr)
{
	/* print the last executed instruction
	 * place INSTR_NUM instructions in the frame */
	static char * last = NULL;
	
	if (last != NULL && last_instr != regs[IAR])
	{
		last[strchr(last, '<') - last] = '\0';
		sprintf(&frame[CODE_LINE - 1][0], "%s %8s", disasm_str[last_instr], " ");
	}
		
	int row, nuls;
	for (row = nuls = 0; row < INSTR_NUM; ++row)
	{	
		if (disasm_str[regs[IAR] + row + nuls] != NULL)
		{
			sprintf(&frame[CODE_LINE + row][0], "%-23s %s %8s", 
					disasm_str[regs[IAR] + row + nuls], (row == 0) ? "<--" : " ", " ");
		}
		else
		{
			--row;
			++nuls;
		}
	}
			
	last = &frame[CODE_LINE][0];
	
	return;
}

static void do_regs(int hex_dec)
{
	/* place the register values in the frame */
	static char * regs_str[] = 	{
		"MAR", "IAR", "IR", " ", 
		"C", "A", "E", "Z", " ", 
		"R0", "R1", "R2", "R3"	
	};
		
	static char * reg_base[] =  {
		"%s  %-3s %02X ", 
		"%s  %-3s %-3d"
	};
	
	int i, j;
	char * cp;
	
	// NUM_REGS + 2 empty lines
	for (i = j = 0; i < NUM_REGS + 2; ++i)
	{
		switch (i)
		{
			// skip the empty lines
			case 3:
			case 8:
				break;
			default:
				cp = &frame[REG_LINE+i][0];
				sprintf(cp, reg_base[hex_dec], cp, regs_str[i], regs[j++]);
				break;
		}
	}
	
	return;
}
