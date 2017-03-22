/* jcpdis.c -- disassembler for the jcpu */
/* ver. 1.01 */

/* Reads a binary file and disassembles it
 * to jcpu assembly language. */
 
/* Author: Vladimir Dinev */
#include <stdio.h>
#include <stdlib.h>
#include "../disasm.h"

#define MAX_CODE	256		// no more than 256 bytes can be decompiled
#define DASH		'-'		// command line arguments begin with -
#define OUTF		'o'		// output file follows
#define VERS		'v'		// version info flag
#define HELP		'h'		// help flag
#define SKIP_ADDR	12
#define print_use()	printf("Use:  %s <in file> %c%c <out file>\n", exenm, DASH, OUTF)
#define help_opt()	printf("Help: %s %c%c\n", exenm, DASH, HELP)

char exenm[] = "jcpdis";	// executable name
char ver[] = "v1.01";		// executable version

FILE * efopen(const char * fname, const char * mode);
int fsize(FILE *fp);
void print_help(void);

int main(int argc, char * argv[])
{
	/* parse command line
	 * open files
	 * call into disasm
	 * save disassembly as text */
	if (argc > 1 && DASH == argv[1][0])
	{
		if (HELP == argv[1][1])
			print_help();
		else if(VERS == argv[1][1])
			printf("%s %s\n", exenm, ver);
		else
		{
			fprintf(stderr, "Err: unrecognized argument \"%s\"\n", argv[1]);
			print_use();
			help_opt();
		}
		
		return -1;
	}
	
	if (argc != 4)
	{
		print_use();
		help_opt();
		return -1;
	}
	
	if (DASH != argv[2][0] || OUTF != argv[2][1])
	{
		fprintf(stderr, "Err: unrecognized argument \"%s\"\n", argv[2]);
		return -1;
	}
	
	static byte code[MAX_CODE] = {0};
	char * fin, * fout;
	fin = argv[1];
	fout = argv[3];
	
	FILE * file_input = efopen(fin, "rb");
	int file_size = fsize(file_input);
	
	if (-1 == file_size)
		goto readerr;
		
	if (file_size > MAX_CODE)
	{
		printf("Warning: resulting code is bigger than the maximum of %d bytes\n",
				MAX_CODE);
		printf("Only the first %d bytes will be disassembled\n", MAX_CODE);
				
		file_size = MAX_CODE;
	}
	
	if (fread(code, file_size, 1, file_input) != 1)
		goto readerr;
	
	char ** disstr = disasm_dis(code, file_size, PREF_HEX);
	FILE * file_output = efopen(fout, "w");
	
	int i;
	for (i = 0; i < file_size; ++i)
	{
		if (disstr[i] != NULL)
			fprintf(file_output, "%s\n", &disstr[i][SKIP_ADDR]);
	}
	
	fclose(file_output);
	fclose(file_input);
	puts("Disassembling complete");
	return 0;
	
readerr:
	fprintf(stderr, "Err: a reading error has occured\n");
	fclose(file_input);
	return -1;
}

FILE * efopen(const char * fname, const char * mode)
{
	/* open a file or die with an error */
	FILE * fp; 
	
	if ( (fp = fopen(fname, mode)) == NULL)
	{
		fprintf(stderr, "Err: could not open file \"%s\"\n", fname);
		exit(EXIT_FAILURE);
	}
	
	return fp;
}

int fsize(FILE * fp)
{
	/* get file size for opened file */
	int size;
	
	if (fseek(fp, 0L, SEEK_END) != 0)
		return -1;
		
	size = ftell(fp);
	rewind(fp);
	
	return size;
}

void print_help(void)
{
	/* show help */
	printf("Disassemble: %s <input binary file> %c%c <output text file>\n", 
			exenm, DASH, OUTF);
	printf("<input binary file> should be the name of a file compiled for the jcpu\n");
	printf("Version:     %s %c%c\n", exenm, DASH, VERS);
	printf("Help:        %s %c%c\n", exenm, DASH, HELP);
	return;
}
