/* lexlang.c -- lexer implementation for the lang compiler */
/* ver. 1.1 */

/* Reads the input source file and returns a token of what
 * was read along with it's textual representation if any. */

/* Author: Vladimir Dinev */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lexlang.h"
#include "../mach_code.h"

#define NEW_LINE	'\n'
#define JMP_INSTR	'J'
#define BUFF_SZ 	256	// the input buffer size
#define COMMENT		'#'	// comments in the source start with #
#define LBL_START	'.'	// labels begin with a '.'
#define read_line()	(fgets(input_buff, BUFF_SZ, fp_in) != NULL)
#define eat_line()	while (*curr_ch) ++curr_ch

/* so we can describe token values in text 
 * like token_strs[tok] */
static const char * token_strs[] = {
	"=",
	"<", "=<", "==", ">=", ">", "!=",
	"IF", "ENDIF",
	"LOOP", "ENDLOOP", "BREAK", "CONTINUE",
	"jump", "instruction",
	"register",
	"label",
	"address/number",
	"comment", "new line",
	"eof"
};
	
static FILE * fp_in;						// input file pointer
static int line_no = 0;						// current line in the source
static char input_buff[BUFF_SZ] = {NUL};	// the input buffer; lines are read in here
static char * curr_ch = input_buff;			// the current character in the input buffer
static char sub_str[SUB_STR_SZ] = {NUL};	// holds a processed substring from the source line
static int curr_tok = -1;					// the value of the current token
static char * lexm_start;					// remembers the start of the lexeme

static int get_lexm(void);
static int get_literal(void);
static int next_lexm(void);

/* -------------------- PUBLIC INTERFACE START -------------------- */
// defines the Lexer struct
lexr_class Lexer = {
	set_input, 
	init,
	get_lex_buff,
	curr_line, 
	match, 
	curr_tkn,
	get_num,
	get_src_line,
	get_err_pos,
	token_str
};

void set_input(FILE * fp)
{
	/* point to the input file */
	fp_in = fp;
	return;
}

void init(void)
{
	/* start reading */
	static bool is_init = false;
	
	if (is_init == false)
	{
		curr_tok = next_lexm();
		is_init = true;
	}
	
	return;
}

const char * get_lex_buff(void)
{
	/* return the current lexeme buffer */
	return sub_str;
}

int curr_line(void)
{
	/* return the line number */
	return line_no;
}

bool match(token tok)
{
	/* match a token against the current token */
	if (curr_tok != tok)
	{
		fprintf(stderr, "Err: line %d: %s expected\n", line_no, token_strs[tok]);
		return false;
	}
	
	curr_tok = next_lexm();
	
	return true;
}

token curr_tkn(void)
{
	/* return the current token */
	return curr_tok;
}

char * get_num(void)
{
	return sub_str;
}

char * get_src_line(void)
{
	/* return the source line buffer */
	return input_buff;
}

int get_err_pos(void)
{
	/* return the position at which the error occured */
	return lexm_start - input_buff;
}

const char * token_str(token tok)
{
	/* print a token as text */
	return token_strs[tok];
}
/* -------------------- PUBLIC INTERFACE END -------------------- */

static int next_lexm(void)
{
	/* get next lexeme/token */
	int i, ch_int;
	
	while (true)
	{
		if (NUL == *curr_ch)
		{
			if (read_line() == false)
				return EOI;
			else
			{
				++line_no;
				curr_ch = input_buff;
				while (isspace(*curr_ch))
				{
					// return new lines to keep symmetry
					if (NEW_LINE == *curr_ch)
						return TOK_NEW_LINE;
					
					++curr_ch;
				}
		
				// make everything uppercase and replace commas
				for (i = 0; input_buff[i] != NUL; ++i)
				{
					input_buff[i] = toupper(input_buff[i]);
					if (',' == input_buff[i])
						input_buff[i] = ' ';
				}
					
				curr_ch = input_buff;
			}
		}
		else
			++curr_ch;
		
		// eat spaces
		while (isspace(*curr_ch))
			++curr_ch;
		
		// skip empty lines
		if (NUL == *curr_ch)
			continue;
		
		lexm_start = curr_ch+1;
	
		char next;
		ch_int = *curr_ch;
		switch (ch_int)
		{
			case '=':
				next = *(curr_ch+1);
				
				if ('=' == next)
				{
					++curr_ch;
					return TOK_EQ;
				}
					
				return TOK_ASSIGN;
				break;
			case '!': 
				if ('=' == *(curr_ch+1))
				{
					++curr_ch;
					return TOK_DIFF;
				}
				else
				{
					fprintf(stderr, "Err: line %d: < ! > without < = >\n", line_no);
					fprintf(stderr, "Parsing aborted\n");
					exit(EXIT_FAILURE);
				}
				break;
			case '<':
				next = *(curr_ch+1);
				
				if ('=' == next)
				{
					++curr_ch;
					return TOK_EQLESS;
				}
				
				return TOK_LESS;	
				break;
			case '>':	
				next = *(curr_ch+1);
				
				if ('=' == next)
				{
					++curr_ch;
					return TOK_EQGREAT;
				}
					
				return TOK_GREAT;	
				break;
			case 'R':	get_lexm();		return TOK_REGISTER;	break;
			case LBL_START:	eat_line(); return TOK_LABEL;	break;
			case COMMENT:	eat_line(); return TOK_COMMENT;	break;
			case EOF:	return EOI;	break;
			default:
					// check for asm instructions and keywords
					if (isalpha(*curr_ch))
					{
						int kw = get_lexm();
						
						if (TOK_INSTR == kw)
							eat_line();
						
						return kw;
					}
					else if (isdigit(*curr_ch))
						return get_literal();
					else
						return TOK_ERR;
				break;
		}
	}
}

static int get_literal(void)
{
	/* read a literal value */
#define LITERAL_LEN	4
	char * plit = sub_str;
	int lit_len = 0;
	
	// skip prefix
	if ('0' == *curr_ch)
		*plit++ = *curr_ch++, ++lit_len;
	if ('X' == *curr_ch)
		*plit++ = *curr_ch++, ++lit_len;
	
	while (*curr_ch && !isspace(*curr_ch))
	{
		if (!isxdigit(*curr_ch))
		{
			*plit = *curr_ch, *(plit+1) = NUL;
			fprintf(stderr, "Err: line %d: bad literal < %s >\n", line_no, sub_str);
			fprintf(stderr, "Parsing aborted\n");
			exit(EXIT_FAILURE);
		}
		
		++lit_len;
		if (lit_len > LITERAL_LEN)
		{
			*plit = *curr_ch, *(plit+1) = NUL;
			fprintf(stderr, "Err: line %d: literal < %s > too long\n", line_no, sub_str);
			fprintf(stderr, "Parsing aborted\n");
			exit(EXIT_FAILURE);
		}
		
		*plit++ = *curr_ch++;
	}
	
	*plit = NUL;
	return TOK_LITERAL;
}

static int get_lexm(void)
{
	/* read a word and see what it is */
	char * kword_pos = sub_str;
	
	// get characters inbetween spaces
	while (*curr_ch && !isspace(*curr_ch))
	{
		*kword_pos = *curr_ch;
		++kword_pos, ++curr_ch;
	}
	
	// terminate the string
	*kword_pos = NUL;
	
	if (JMP_INSTR == *sub_str)
		return TOK_JUMP;
	
	// look for keyword
	int i;
	for (i = TOK_IF; i <= TOK_CONTINUE; ++i)
	{
		if (strcmp(sub_str, token_strs[i]) == 0)
			return i;
	}
	
	// if not keyword, assume instruction
	return TOK_INSTR;
}