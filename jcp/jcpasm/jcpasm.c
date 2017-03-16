/* jcpasm.c -- assembler for the jcpu */
/* ver. 1.0 */

/* Reads an assembly text file and outputs
 * the respective binary instructions for the jcpu. */
 
/* Author: Vladimir Dinev */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "list.h"
#include "chtbl.h"
#include "lexjcpa.h"
#include "../mach_code.h"
#include "../jcpu.h"

#define BUCKETS 	26 		// letters of the alphabet
#define MAX_CODE	256		// no more than 256 bytes can compile
#define DASH		'-'		// command line arguments begin with -
#define OUTF		'o'		// output file follows
#define VERS		'v'		// print version info
#define HELP		'h'		// print help
#define print_use()	printf("Use:  %s <in file> %c%c <out file>\n", exenm, DASH, OUTF)
#define help_opt()	printf("Help: %s %c%c\n", exenm, DASH, HELP)

const char * in_buff;		// input buffer pointer
static byte binary[RAM_S];	// compiled code buffer
static int all_size = 0;	// code buffer pointer
CHTbl * instr_htbl;			// instruction hash table pointer
char exenm[] = "jcpasm";	// executable name
char ver[] = "v1.0";		// executable version
int curr_lineno = 0;

// parser functions
void parse_instr(void);
void parse_address(void);
int parse_register(void);
void e_match(token tok);
void print_ln_err(void);

// hash table functions
int compar(const void * key1, const void * key2);
int hash(const void * key);

// application functions
FILE * efopen(const char * fname, const char * mode);
void print_help(void);
void quit(void);

int main(int argc, char * argv[])
{	/* parse input options, 
	 * place machine code info in the hash table,
	 * initiate the lexer,
	 * transfer control to the parser,
	 * save the output code */
	 
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
		fprintf(stderr, "Err: unrecognized option %s\n", argv[2]);
		return -1;
	} 
	
	char * fin, * fout;
	static char curr_text[SUB_STR_SZ];
	CHTbl instr_htbl_;
	instr_htbl = &instr_htbl_;
	
	if (chtbl_init(instr_htbl, BUCKETS, hash, compar, NULL) < 0)
	{
		fprintf(stderr, "Err: initializatoin failed\n");
		return -1;
	}
	
	fin = argv[1];
	fout = argv[3];
		
	int i;
	for (i = 0; i < INSTR_COUNT; ++i)
		chtbl_insert(instr_htbl, &mcode[i]);
	
	FILE * input_file = efopen(fin, "r");
	Lexer.SetInput(input_file);
	Lexer.Init();
	in_buff = curr_text;
	
	token ctok;
	while ((ctok = Lexer.Current()) != EOI)
	{
		if (all_size > MAX_CODE)
			break;
	
		if (ctok != ERR)
			parse_instr();
		else
		{
			fprintf(stderr, "Err: line %d: ", curr_lineno);
			fprintf(stderr, "something not an instruction, address, or register < %s >\n",
					in_buff);
			quit();
		}
	}
	
	FILE * output_file = efopen(fout, "wb");
	if (all_size > MAX_CODE)
	{
		printf("Warning: resulting code is bigger than the maximum of %d bytes\n",
				MAX_CODE);
		printf("Only the first %d bytes will be saved in the binary\n", MAX_CODE);
				
		all_size = MAX_CODE;
	}
	
	if (fwrite(binary , all_size, 1, output_file) != 1)
		fprintf(stderr, "Err: output was not written properly\n");
	
	fclose(output_file);
	fclose(input_file);
	chtbl_destroy(instr_htbl);
	return 0;
}

void parse_instr(void)
{
	/* translate instruction strings into binary code,
	 * update the code pointer */
	e_match(TOK_INSTR);
	// special case of conditional jump
	if ('J' == in_buff[0] && in_buff[1] != 'M')
	{
		binary[all_size] = JCOND << 4;
		
		int i;
		for (i = 1; in_buff[i] != NUL; ++i)
		{
			switch (in_buff[i])
			{
				case 'C': binary[all_size] |= 1 << 3; break;
				case 'A': binary[all_size] |= 1 << 2; break;
				case 'E': binary[all_size] |= 1 << 1; break;
				case 'Z': binary[all_size] |= 1; break;
				default:
					fprintf(stderr, "Err: line %d: invalid instruction < %s >\n", 
							curr_lineno, in_buff);
					quit();
					break;
			}
		}
		++all_size;
		parse_address();
		++all_size;
		return;
	}
	
	instr curr_instr, * tip;
	curr_instr.name = (char *)in_buff;
	
	// handle constant string instructions
	tip = &curr_instr;
	if (chtbl_lookup(instr_htbl, (void **)&tip) == 0)
	{
		tip = *(instr **)tip;
		binary[all_size] = (tip->code) << 4;
		
		switch (tip->code)
		{
			case CLF:
				break;
			case JMP:
				++all_size;
				parse_address();
				break;
			case JMPR:
				// get RB
				binary[all_size] |= parse_register();
				break;
			case DATA:
				// get RB
				binary[all_size] |= parse_register();
				++all_size;
				parse_address();
				break;
			default:
				// get RA
				binary[all_size] |= (parse_register() << 2);
				// get RB
				binary[all_size] |= parse_register();
				break;
		}
		++all_size;
	}
	else
	{
		fprintf(stderr, "Err: line %d: invalid instruction < %s >\n", 
				curr_lineno, in_buff);
		quit();
	}
					
	return;
}

void parse_address(void)
{
	/* reads a literal decimal or hex number */
	e_match(TOK_LITERAL);
	
	int addr_state;
	unsigned int num;
	if ('X' == in_buff[1])
		addr_state = sscanf(in_buff, "%x", &num);
	else
		addr_state = sscanf(in_buff, "%d", &num);
	
	if (addr_state != 1 || num > 0xFF)
	{
		fprintf(stderr, "Err: line %d: invalid address < %s >\n", 
				curr_lineno, in_buff);
		quit();
	}
	
	binary[all_size] = num;
	return;
}

int parse_register(void)
{
	/* translates register mnemonics into binary */
	e_match(TOK_REGISTER);

	int reg;
	if ('R' != in_buff[0])
		goto regerr;
	
	if (sscanf(&in_buff[1], "%d", &reg) != 1)
		goto regerr;
	
	if (reg < 0 || reg > 3)
		goto regerr;
	
	return reg;

regerr:
	fprintf(stderr, "Err: line %d: bad register < %s >\n", 
			curr_lineno, in_buff);
	quit();
	return -1; // we never come here
}

int hash(const void * key)
{
	/* hash for alphabetical order */
	return tolower(((instr *)key)->name[0]) - 'a';
}

int compar(const void * key1, const void * key2)
{
	/* for sorting and searching strings */
	return strcmp((const char *)((instr *)key1)->name, ((instr *)key2)->name);
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

void e_match(token tok)
{
	/* get the current lexeme and see if it's what we expect
	 * if it's something else, the source is wrong */
	strcpy((char *)in_buff, Lexer.GetLexBuff());
	curr_lineno = Lexer.LineNo();
	
	if (!Lexer.Match(tok))
	{
		print_ln_err();
		quit();
	}
	
	return;
}

void print_ln_err(void)
{
	/* print source code error info */
	char * src_ln = Lexer.GetSrcLine();
	
	while (isspace(*src_ln))
		++src_ln;
	
	int end = strlen(src_ln) - 1;
	
	if ('\n' == src_ln[end])
		src_ln[end] = '\0';
		
	fprintf(stderr, "%s\n", src_ln);
	fprintf(stderr, "%*c\n", Lexer.GetErrPos(), '^');
	
	return;
}

void print_help(void)
{
	/* print help */
	printf("Compile: %s <input text file> %c%c <output binary file>\n", 
			exenm, DASH, OUTF);
	printf("Version: %s %c%c\n", exenm, DASH, VERS);
	printf("Help:    %s %c%c\n", exenm, DASH, HELP);
	return;
}

void quit(void)
{
	/* go home */
	puts("Compilation aborted");
	exit(EXIT_FAILURE);
	return;
}
