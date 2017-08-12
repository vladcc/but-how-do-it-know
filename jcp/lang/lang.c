/* lang.c -- a compiler for the lang language; compiles to jcpasm */
/* ver. 1.1 */
 
/* Author: Vladimir Dinev */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "../preproc/preproc.h"
#include "../jcpasm/jcpasm.h"
#include "../mach_code.h"
#include "lexlang.h"

#define LBL_START	'.'
#define LBL_JUMP	1
#define LBL_BREAK	'\0'
#define LITERAL_LEN	5
#define DASH		'-'		// command line arguments begin with -
#define OUTF		'o'		// output file follows
#define VERS		'v'		// print version info
#define HELP		'h'		// print help
#define print_use()	printf("Use:  %s <in file> %c%c <out file>\n", exenm, DASH, OUTF)
#define help_opt()	printf("Help: %s %c%c\n", exenm, DASH, HELP)

typedef struct label_pair {
	char * start;
	char * out;
} label_pair;

char * in_buff;				// input buffer pointer
char exenm[] = "lang";		// internal executable name
char ver[] = "v1.1";		// executable version
int curr_lineno = 0;		// current line number
char * fin, * fout;			// input/output file strings
FILE * input_file, * output_file;

extern char ppexenm[];
extern char ppext[];
extern char jasm_exenm[];
extern char jasm_ext[];

bool g_is_if = false;
token g_curr_compar;

// parser functions
void parse_register(void);
void parse_assign(void);
void parse_if(label_pair * lbls);
void parse_loop(void);
void parse_block(label_pair * lbls);
const char * check_literal(void);
void e_match(token tok);
void print_ln_err(void);
char * make_label(void);

// service functions
bool is_preproc_here(void);
bool is_jasm_here(void);
FILE * efopen(const char * fname, const char * mode);
void * emalloc(size_t nbytes);
void print_help(void);
void quit(void);

int main(int argc, char * argv[])
{	
	if (argc > 1 && DASH == argv[1][0])
	{
		if (HELP == argv[1][1])
			print_help();
		else if (VERS == argv[1][1])
			printf("%s %s\n", exenm, ver);
		else
		{
			fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: unrecognized argument \"%s\"\n", argv[1]);
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
		fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: unrecognized option %s\n", argv[2]);
		quit();
	} 
	
	fin = argv[1];	
	// run the preprocessor
	char * run_pp_asm = emalloc(strlen(ppexenm) + strlen(fin) + 8);
	sprintf(run_pp_asm, "%s %s", ppexenm, fin);
	
	// check if file is available
	input_file = efopen(fin, "r");
	fclose(input_file);
	
	if (is_preproc_here())
	{
		if (system(run_pp_asm) == 0)
		{
			// if the preprocessor did it's job, new input file
			sprintf(run_pp_asm, "%s%s", fin, ppext);
			fin = run_pp_asm;
		}
		else
		{
			free(run_pp_asm);
			run_pp_asm = NULL;
		}
	}
	else
	{
		printf("%s: ", exenm), printf("Warning: %s: The preprocessor should be in the same directory\n", exenm);
		printf("but it's not here. Continuing anyway.\n");
	}
	
	input_file = efopen(fin, "r");
	
	fout = emalloc(strlen(argv[3]) + strlen(jasm_ext) + 1);
	sprintf(fout, "%s%s", argv[3], jasm_ext);
	
	output_file = efopen(fout, "w");
	
	Lexer.SetInput(input_file);
	Lexer.Init();
	
	static char curr_text[SUB_STR_SZ];
	in_buff = curr_text;
	
	token ctok;
	while ((ctok = Lexer.Current()) != EOI)
	{
		switch (ctok)
		{
			case TOK_NEW_LINE:
				e_match(TOK_NEW_LINE);
				fprintf(output_file, "\n");
				break;
			case TOK_INSTR:
				fprintf(output_file, "%s", Lexer.GetSrcLine());
				e_match(TOK_INSTR);
				break;
			case TOK_LABEL:
				fprintf(output_file, "%s", Lexer.GetSrcLine());
				e_match(TOK_LABEL);
				break;
			case TOK_COMMENT:
				fprintf(output_file, "%s", Lexer.GetSrcLine());
				e_match(TOK_COMMENT);
				break;
			case TOK_REGISTER:
				parse_register();
				break;
			case TOK_IF:
				parse_if(NULL);
				break;
			case TOK_JUMP:
				e_match(TOK_JUMP);
				break;
			case TOK_BREAK:
				e_match(TOK_BREAK);
				fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: stray break\n", curr_lineno);
				quit();
				break;
			case TOK_CONTINUE:
				e_match(TOK_CONTINUE);
				fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: stray continue\n", curr_lineno);
				quit();
				break;
			case TOK_LOOP:
				parse_loop();
				break;
			default:
				fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: ", Lexer.LineNo());
				fprintf(stderr, "something not a condition, instruction, number, or register < %s >\n",
						Lexer.GetLexBuff());
				quit();
				break;
		}
	}
	
	fclose(output_file);
	fclose(input_file);
	
	free(run_pp_asm);
	run_pp_asm = emalloc(strlen(jasm_exenm) + strlen(fout) + strlen(" -o ") + strlen(argv[3]) + 2);
	sprintf(run_pp_asm, "%s %s -o %s", jasm_exenm, fout, argv[3]);
	
	if (is_jasm_here())
		system(run_pp_asm);
	else
	{
		printf("%s: ", exenm), printf("Warning: %s: %s should be in the same directory\n", jasm_exenm, exenm);
		printf("but it's not here. Continuing anyway.\n");
	}
	
	free(run_pp_asm);
	free(fout);
	return 0;
}

/* ---------------------------- PARSER FUNCTIONS START ----------------------------  */
void parse_register(void)
{
	/* translate register assignment or begin translation of comparison */
	e_match(TOK_REGISTER);
	
	int reg, reg2;
	if (sscanf(&in_buff[1], "%d", &reg) != 1)
		goto regerr;
	
	if (reg < 0 || reg > 3)
		goto regerr;
	
	token ctok = Lexer.Current();
	
	switch (ctok)
	{
		case TOK_ASSIGN:
			if (g_is_if)
			{
				fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: assignment in if\n", curr_lineno);
				quit();
			}
			
			e_match(TOK_ASSIGN);
			ctok = Lexer.Current();
	
			if (TOK_LITERAL == ctok)
			{
				fprintf(output_file, "%s %s, %s\n", mcode[DATA].name, gregs[reg], Lexer.GetNum());
				e_match(TOK_LITERAL);
			}
			else if (TOK_REGISTER == ctok)
			{
				e_match(TOK_REGISTER);
				if (sscanf(&in_buff[1], "%d", &reg2) != 1)
					goto regerr;
	
				if (reg2 < 0 || reg2 > 3)
					goto regerr;
				
				fprintf(output_file, "%s %s, %s\n", mcode[XOR].name, gregs[reg], gregs[reg]);
				fprintf(output_file, "%s %s, %s\n", mcode[OR].name, gregs[reg2], gregs[reg]);
			}
			else
			{
				fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: literal or register expected but got < %s >\n", 
						curr_lineno, Lexer.GetLexBuff());
				quit();
			}
				
			break;
		
		case TOK_LESS:
		case TOK_EQLESS:
		case TOK_EQ:
		case TOK_GREAT:
		case TOK_EQGREAT:
		case TOK_DIFF:
			if (false == g_is_if)
			{
				fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: logical comparison without and if\n", 
						curr_lineno);
				quit();
			}
			g_curr_compar = ctok;
			e_match(ctok);
			
			e_match(TOK_REGISTER);
			if (sscanf(&in_buff[1], "%d", &reg2) != 1)
				goto regerr;

			if (reg2 < 0 || reg2 > 3)
				goto regerr;
		
			fprintf(output_file, "%s %s, %s\n", mcode[CMP].name, gregs[reg], gregs[reg2]);
			break;
		default:
			break;
	}
	
	return;
	
regerr:
	fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: bad register < %s >\n", 
			curr_lineno, in_buff);
	quit();
}

void parse_if(label_pair * lbls)
{
	/* translate simple if/endif; no else and else ifs */
	e_match(TOK_IF);

	g_is_if = true;
	parse_register();
	g_is_if = false;
	
	char * lbl_code = NULL;
	char * lbl_end = NULL;
	
	switch (g_curr_compar)
	{
		case TOK_LESS:
			lbl_end = make_label();
			
			fprintf(output_file, "JEA %s\n", lbl_end);
			parse_block(lbls);
			fprintf(output_file, "%s:\n", lbl_end);
			break;
		case TOK_EQLESS:
			lbl_end = make_label();
			
			fprintf(output_file, "JA %s\n", lbl_end);
			parse_block(lbls);
			fprintf(output_file, "%s:\n", lbl_end);
			break;
		case TOK_EQ:
			lbl_code = make_label();
			lbl_end = make_label();
			
			fprintf(output_file, "JE %s\n", lbl_code);
			fprintf(output_file, "%s %s\n", mcode[JMP].name, lbl_end);
			fprintf(output_file, "%s:\n", lbl_code);
			parse_block(lbls);
			fprintf(output_file, "%s:\n", lbl_end);
			break;
		case TOK_GREAT:
			lbl_code = make_label();
			lbl_end = make_label();
			
			fprintf(output_file, "JA %s\n", lbl_code);
			fprintf(output_file, "%s %s\n", mcode[JMP].name, lbl_end);
			fprintf(output_file, "%s:\n", lbl_code);
			parse_block(lbls);
			fprintf(output_file, "%s:\n", lbl_end);
			break;
		case TOK_EQGREAT:
			lbl_code = make_label();
			lbl_end = make_label();
			
			fprintf(output_file, "JAE %s\n", lbl_code);
			fprintf(output_file, "%s %s\n", mcode[JMP].name, lbl_end);
			fprintf(output_file, "%s:\n", lbl_code);
			parse_block(lbls);
			fprintf(output_file, "%s:\n", lbl_end);
			break;
		case TOK_DIFF:
			lbl_end = make_label();
			
			fprintf(output_file, "JE %s\n", lbl_end);
			parse_block(lbls);
			fprintf(output_file, "%s:\n", lbl_end);
			break;
		default: 
			break;
	}
	
	e_match(TOK_ENDIF);
	
	free(lbl_code);
	free(lbl_end);
	return;
}

void parse_loop(void)
{
	/* translate loop/break/endloop construct */	
	e_match(TOK_LOOP);
	
	label_pair lbls;
	bool was_break = false;
	
	lbls.start = make_label();
	lbls.out = make_label();
	
	fprintf(output_file, "%s:\n", lbls.start);
	
	parse_block(&lbls);
	
	if (*lbls.out != LBL_BREAK && *lbls.out != LBL_JUMP)
		fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Warning: line %d: loop ends without a break or a jump\n", Lexer.LineNo());
	else
	{
		if (LBL_BREAK == *lbls.out)
		{
			*lbls.out = LBL_START;
			was_break = true;
		}
		else
			*lbls.out = LBL_START;
	}
		
	fprintf(output_file, "%s %s\n", mcode[JMP].name, lbls.start);
	
	if (was_break)
		fprintf(output_file, "%s:\n", lbls.out);
	
	e_match(TOK_ENDLOOP);
	
	free(lbls.start);
	free(lbls.out);
	return;
}

void parse_block(label_pair * lbls)
{
	/* called between if/endif and loop/endloop */
	
	token ctok;
	while ((ctok = Lexer.Current()) != TOK_ENDIF && ctok != TOK_ENDLOOP && ctok != EOI)
	{
		switch (ctok)
		{
			case TOK_NEW_LINE:
				e_match(TOK_NEW_LINE);
				fprintf(output_file, "\n");
				break;
			case TOK_INSTR:
				fprintf(output_file, "%s", Lexer.GetSrcLine());
				e_match(TOK_INSTR);
				break;
			case TOK_LABEL:
				fprintf(output_file, "%s", Lexer.GetSrcLine());
				e_match(TOK_LABEL);
				break;
			case TOK_COMMENT:
				fprintf(output_file, "%s", Lexer.GetSrcLine());
				e_match(TOK_COMMENT);
				break;
			case TOK_REGISTER:
				parse_register();
				break;
			case TOK_IF:
				parse_if(lbls);
				break;
			case TOK_LOOP:
				parse_loop();
				break;
			case TOK_JUMP:
				e_match(TOK_JUMP);
				
				if (lbls)
					*(lbls->out) = LBL_JUMP;
				break;
			case TOK_BREAK:
				e_match(TOK_BREAK);
				
				if (lbls)
				{
					*(lbls->out) = *(lbls->out) ? LBL_START : LBL_BREAK;
					fprintf(output_file, "%s %s\n", mcode[JMP].name, lbls->out);
					*(lbls->out) = LBL_BREAK;
				}	
				else
				{
					fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: stray break\n", curr_lineno);
					quit();
				}
				
				break;
			case TOK_CONTINUE:
				e_match(TOK_CONTINUE);
				fprintf(output_file, "%s %s\n", mcode[JMP].name, lbls->start);
				break;
			default:
				break;
		}
	}
	
	return;
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
		src_ln[end] = NUL;
		
	fprintf(stderr, "%s: ", exenm), fprintf(stderr, "%s\n", src_ln);
	fprintf(stderr, "%s: ", exenm), fprintf(stderr, "%*c\n", Lexer.GetErrPos(), '^');
	
	return;
}

char * make_label(void)
{
	/* get a new label; caller frees memory */
#define LBL_BUFF 8

	static int n = 0;
	char * lbl = emalloc(LBL_BUFF);
	
	sprintf(lbl, ".L%d", n);
	++n;
	
	return lbl;
}
/* ---------------------------- PARSER FUNCTIONS END ----------------------------  */

/* ---------------------------- SERVICE FUNCTIONS START ----------------------------  */
bool is_preproc_here(void)
{
	/* see if the preproc is available */
	FILE * pproc = fopen(ppexenm, "r");
	
	if (NULL == pproc)
		return false;
	else
		fclose(pproc);
		
	return true;
}

bool is_jasm_here(void)
{
	/* see if the jasm is available */
	FILE * jasm = fopen(jasm_exenm, "r");
	
	if (NULL == jasm)
		return false;
	else
		fclose(jasm);
		
	return true;
}

FILE * efopen(const char * fname, const char * mode)
{
	/* open a file or die with an error */
	FILE * fp; 
	
	if ( (fp = fopen(fname, mode)) == NULL)
	{
		fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: could not open file \"%s\"\n", fname);
		exit(EXIT_FAILURE);
	}
	
	return fp;
}

void * emalloc(size_t nbytes)
{
	/* allocate memory or die with an error */
	void * newmem;
	
	if ((newmem = malloc(nbytes)) == NULL)
	{
		fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: memory allocation failed\n");
		exit(EXIT_FAILURE);
	}
	
	return newmem;
}

void print_help(void)
{
	/* show help */
	printf("Compile: %s <input text file> %c%c <output binary file>\n", 
			exenm, DASH, OUTF);
	printf("Version: %s %c%c\n", exenm, DASH, VERS);
	printf("Help:    %s %c%c\n", exenm, DASH, HELP);
	return;
}

void quit(void)
{
	/* hcf */
	puts("Compilation aborted");
	exit(EXIT_FAILURE);
	return;
}
/* ---------------------------- SERVICE FUNCTIONS END ----------------------------  */