/* lexjcpa.c -- lexer implementation for the jcpasm */
/* ver. 1.10 */

/* Reads the input source file and returns a token of what
 * was read, along with it's textual representation if any. */

/* Author: Vladimir Dinev */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lexjcpa.h"
#include "../mach_code.h"

#define BUFF_SZ 	256	// the input buffer size
#define COMMENT		'#'	// comments in the source start with #
#define LBL_START	'.'	// labels begin with a '.'
#define read_line()	(fgets(input_buff, BUFF_SZ, fp_in) != NULL)

/* so we can describe token values in text 
 * like token_strs[tok] */
static const char * token_strs[] = {
	"instruction",
	"register",
	"label",
	"address/number",
	"eof"
};

// TODO: DO THE LABELS
	
static FILE * fp_in;						// input file pointer
static int line_no = 0;						// current line in the source
static char input_buff[BUFF_SZ] = {NUL};	// the input buffer; lines are read in here
static char * curr_ch = input_buff;			// the current character in the input buffer
static char sub_str[SUB_STR_SZ] = {NUL};	// holds a processed substring from the source line
static int curr_tok = -1;					// the value of the current token
static char * lexm_start;					// remembers the start of the lexeme

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
		fprintf(stderr, "Err: line %d: %s expected but got < %s >\n",
		line_no, token_strs[tok], sub_str);
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
	int i;
	
	while (true)
	{
		if (NUL == *curr_ch)
		{
			if (read_line() == false)
				return EOI;
			else
			{
				// make everything uppercase and replace commas
				for (i = 0; input_buff[i] != NUL; ++i)
				{
					input_buff[i] = toupper(input_buff[i]);
					if (',' == input_buff[i])
						input_buff[i] = ' ';
				}
					
				curr_ch = input_buff;
				++line_no;
			}
		}
		
		// eat spaces
		while (isspace(*curr_ch))
			++curr_ch;
		
		// skip empty lines
		if (NUL == *curr_ch)
			continue;
		
		lexm_start = curr_ch+1;
		
		if (LBL_START == *curr_ch)
		{
			// get label
			i = 0;
			sub_str[i] = *curr_ch;
			for (++i, ++curr_ch; i < SUB_STR_SZ && isalnum(*curr_ch); ++i, ++curr_ch)
				sub_str[i] = *curr_ch;
			
			if (':' == *curr_ch)
				sub_str[i++] = *curr_ch++;
		}
		else
		{
			// get substring
			for (i = 0; i < SUB_STR_SZ && isalnum(*curr_ch); ++i, ++curr_ch)
				sub_str[i] = *curr_ch;
		}
		sub_str[i] = NUL;
		
		
		// eat comments
		if (COMMENT == *curr_ch)
		{
			while (*curr_ch)
				++curr_ch;
		}
		
		// everything legal except spaces and the NUL should be eaten by now
		if (!isspace(*curr_ch) && *curr_ch != NUL)
		{
			fprintf(stderr, "Err: line %d: misplaced character '%c'\n", 
					line_no, *curr_ch);
			fprintf(stderr, "Parsing aborted\n");
			exit(EXIT_FAILURE);
		}
		
		// don't go past the end of the string because of a comment
		if (NUL != *curr_ch)
			++curr_ch;
		
		if ('R' == sub_str[0]) 				return TOK_REGISTER;
		else if (LBL_START == sub_str[0])	return TOK_LABEL;
		else if (isalpha(sub_str[0])) 		return TOK_INSTR;
		else if (isdigit(sub_str[0]))		return TOK_LITERAL;
		else if (sub_str[0] != NUL)			return ERR;
	}
}
