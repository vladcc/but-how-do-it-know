/* lexjcpa.h -- header file for the jcp assembler lexer */
/* ver. 1.11 */

/* Defines used constants and declares public
 * functions. Provides a class-like interface. */
 
/* Author: Vladimir Dinev */
#ifndef LEXJCPA_H_
#define LEXJCPA_H_

#include <stdbool.h>

#define SUB_STR_SZ	64
#define NUL			'\0'

// tokens
typedef enum token_ {	
		TOK_INSTR,
		TOK_REGISTER,
		TOK_LABEL,
		TOK_LITERAL,
		EOI,
		ERR
} token; 

// public functions
void set_input(FILE * fp);
/* 
 * returns: Nothing.
 * 
 * description: Points the lexer to the proper input file. */

void init(void);
/* 
 * returns: Nothing.
 * 
 * description: Kickstarts the lexer into parsing input. */

const char * get_lex_buff(void);
/*
 * returns: A pointer to the buffer containing the current lexeme.
 * 
 * description: Points you to the last parsed substring. */

int curr_line(void);
/*
 * returns: The current line in the source file.
 * 
 * description: Gets you the current source line number. */

bool match(token tok);
/*
 * returns: True if tok matches the current lexeme, false otherwise.
 * 
 * description: Sees if the requested token matches the current one,
 * displays an error if it doesn't. */

token curr_tkn(void);
/* 
 * returns: The current token so far in the source.
 * 
 * description: Provides you with the current token in the lexer. */

char * get_src_line(void);
/* 
 * returns: A pointer to the buffer containing the current source code line.
 * 
 * description: Provides you with the current source code line;
 * if the last character is not '\n', a '\n' is appended. */

int get_err_pos(void);
/*
 * returns: An int representing the error point in the source code string.
 * 
 * description: Gets the position of the first character of the lexeme
 * on which the error occured; sometimes more accurate than others. */

const char * token_str(token tok);
/* 
 * returns: The string representation of tok.
 * 
 * description: Gets a string describing the token value. */

// class-like interface
typedef struct lexr_class_ {
	void (*SetInput)(FILE * fp);
	void (*Init)(void);
	const char * (*GetLexBuff)(void);
	int (*LineNo)(void);
	bool (*Match)(token tok);
	token (*Current)(void);
	char * (*GetSrcLine)(void);
	int (*GetErrPos)(void);
	const char * (*TokenStr)(token tok);
} lexr_class;

/* a struct with function pointers for convenience defined in lexjcpa.c */
extern lexr_class Lexer;
#endif
