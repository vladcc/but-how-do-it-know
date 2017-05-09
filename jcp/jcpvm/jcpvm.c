/* jcpvm.c -- a virtual machine for the jcpu */
/* ver. 1.02 */

/* Implements the user interface. */

/* Author: Vladimir Dinev */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../display.h"
#include "../jcpu.h"

#define MAX_CODE 		256		// maximum code for ram
#define IN_BUFF_SZ		128		// input buffer size
#define NUL				'\0'	// ascii null
#define DECIMAL			'd'		// print frame in decimal
#define QUIT			'q'		// quit the emulation
#define JUMP			'j'		// jump n instructions in the future
#define RESET			'r'		// reset the emulation
#define HELP			'h'		// print help
#define VERS			'v'		// print version info
#define DASH			'-'		// cmd line argument prefix
#define press_enter()	printf("Press enter to continue"), getchar()
#define prompt()		printf("%*s\rcmd: ", FRAME_ROWS, " ")
#define reset_cur_pos()	disp_move_cursor_xy(0, 0)
#define mv_cur_bottom()	disp_move_cursor_xy(24, 0)
#define reset_cpu()		memset(regs, 0, NUM_REGS)
#define print_ver()		printf("%s %s\n", exenm, ver)

char exenm[] = "jcpvm";	// executable name
char ver[] = "v1.02";	// executable version
int last_inst = 0;		// the previous executed instruction address

FILE * efopen(const char * fname);
int fsize(FILE * fp);
void new_screen(void);
void print_help(bool interactive);

int main(int argc, char * argv[])
{
	/* parse command line options
	 * loop through the emulation */
	static byte incode[MAX_CODE] = {0};
	static char cmdbuff[IN_BUFF_SZ] = {NUL};
	
	if (argc > 1 && DASH == argv[1][0])
	{
		if (HELP == argv[1][1])
			print_help(false);
		else if(VERS == argv[1][1])
			print_ver();
		else
		{
			fprintf(stderr, "Err: unrecognized argument \"%s\"\n", argv[1]);
			printf("Use: %s <file name> or %s %c%c for help\n", 
				exenm, exenm, DASH, HELP);
		}
		
		return -1;
	}
	
	if (argc != 2)
	{
		printf("Use: %s <file name> or %s %c%c for help\n", 
				exenm, exenm, DASH, HELP);
		return -1;
	}
	
	FILE * infile = efopen(argv[1]);
	int f_sz = fsize(infile);
	size_t read_c = fread(incode, f_sz, 1, infile);
	
	fclose(infile);
	
	if (read_c > 0)
		jcpu_load(incode, f_sz);
	else
	{
		fprintf(stderr, 
				"Err: \"%s\" is either empty or a reading error has occured\n", 
				argv[1]);
		return -1;
	}
		
	disp_init_frame();
	disp_clear();
	
	char * ch;
	last_inst = regs[IAR];
	int j_steps = 0;
	
	// main loop
	while (true)
	{
		new_screen();
		fgets(cmdbuff, IN_BUFF_SZ, stdin);
		ch = cmdbuff;
		
		while (isspace(*ch))
			++ch;
		
		switch (*ch)
		{
			case DECIMAL:
				reset_cur_pos();
				disp_print(DEC_DSP, last_inst);
				mv_cur_bottom();
				press_enter();
				continue;
				break;
			case JUMP:
				++ch;
				if (sscanf(ch, "%d", &j_steps) != 1)
					j_steps = 0;
				break;
			case RESET:
				reset_cpu();
				jcpu_load(incode, f_sz);
				continue;
				break;
			case HELP:
				print_help(true);
				press_enter();
				continue;
				break;
			case QUIT:
				goto gohome;
				break;
			default:
				break;
		}
		
		if (j_steps > 0 && j_steps < MAX_CODE)
		{
			while (j_steps-- > 0)
			{
				last_inst = regs[IAR];
				jcpu_step();
			}
		}
		else
		{
			last_inst = regs[IAR];
			jcpu_step();
		}
	}
	
gohome:
	return 0;
}

FILE * efopen(const char * fname)
{
	/* try to open a file
	 * die on failure */
	FILE * fp; 
	
	if ( (fp = fopen(fname, "rb")) == NULL)
	{
		fprintf(stderr, "Err: could not open file \"%s\"\n", fname);
		exit(EXIT_FAILURE);
	}
	
	return fp;
}

int fsize(FILE * fp)
{
	/* return the file size for fp */
	int size;
	
	if (fseek(fp, 0L, SEEK_END) != 0)
		return -1;
		
	size = ftell(fp);
	rewind(fp);
	
	return size;
}

void new_screen(void)
{
	/* print a new fram
	 * Note: last_inst is global for this file */
	reset_cur_pos();
	disp_print(HEX_DSP, last_inst);
	mv_cur_bottom();
	prompt();
	return;
}

void print_help(bool interactive)
{
	/* show help */
	if (interactive)
	{
		disp_clear();
		reset_cur_pos();
	}
	else
	{
		printf("Start the vm: %s <file name>\n", exenm);
		printf("<file name> should be the name of a file compiled for the jcpu\n");
		printf("Version:      %s %c%c\n" ,exenm, DASH, VERS);
		printf("Help:         %s %c%c\n", exenm, DASH, HELP);
	}
	
	printf("\nInteractive options:\n");
	printf("Execute a single instruction - enter\n");
	printf("Jump n instructions ahead    - %c <n> + enter\n", JUMP);
	printf("Note: jumping executes n instructions, it does not skip over\n");
	printf("n instructions from the code\n");
	printf("Reset the cpu                - %c + enter\n", RESET);
	printf("Print screen in decimal      - %c + enter\n", DECIMAL);
	printf("Print help in vm             - %c + enter\n", HELP);
	printf("Quit                         - %c + enter\n", QUIT);
	return;
}
