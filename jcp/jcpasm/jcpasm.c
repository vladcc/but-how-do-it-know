/* jcpasm.c -- assembler for the jcpu */
/* ver. 1.123 */

/* Reads an assembly text file and outputs
 * the respective binary instructions for the jcpu. */
 
/* Author: Vladimir Dinev */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "../adt/list.h"
#include "../adt/chtbl.h"
#include "../mach_code.h"
#include "../preproc/preproc.h"
#include "../jcpu.h"
#include "lexjcpa.h"

#define BUCKETS 	128		// hash table buckets
#define MAX_CODE	256		// no more than 256 bytes can compile
#define DASH		'-'		// command line arguments begin with -
#define OUTF		'o'		// output file follows
#define VERS		'v'		// print version info
#define HELP		'h'		// print help
#define LBL_ADDR	':'		// if a label ends with ':', parse it as address mark
#define LBL_JUMP	'j'		// if a label doesn't end with ':', parse it as a jump destination
#define DEC_SEP		' '		// separates the label name and decoration number
#define NUL			'\0'	// ascii null
#define print_use()	printf("Use:  %s <in file> %c%c <out file>\n", exenm, DASH, OUTF)
#define help_opt()	printf("Help: %s %c%c\n", exenm, DASH, HELP)

typedef struct label_ {
	int address;			// the address at which the label was found
	int lineno;				// the line number at which the label was found
	char * lbl_str;			// points to the label string
	char context;			// specifies address of target jump context
	bool visited;			// specifies if it has been used or not
} label;

char * in_buff;				// input buffer pointer
static byte binary[RAM_S];	// compiled code buffer
static int all_size = 0;	// code buffer pointer
CHTbl * instr_htbl;			// instruction hash table pointer
CHTbl * lbls_htbl;			// label hash table pointer
char exenm[] = "jcpasm";	// executable name
char ver[] = "v1.123";		// executable version
int curr_lineno = 0;		// current line number
char * fin, * fout;			// input/output file strings

extern char ppexenm[];
extern char ppext[];

// parser functions
void parse_instr(void);
int parse_register(void);
void parse_label(char context);
void parse_address(void);
void check_literal(void);
void eval_labels(void);
void e_match(token tok);
void print_ln_err(void);

// hash table functions
int hash_inst(const void * key);
int compar_inst(const void * key1, const void * key2);
int hash_lbl(const void * key);
int compar_lbl(const void * key1, const void * key2);
label * make_lbl_node(char * lbl);
void free_labels(void * lbl);
void resolve_lbl_addr(ListElmt * l_element, void * args);
void look_lone_lbls(ListElmt * l_element, void * args);

// service functions
bool is_preproc_here(void);
FILE * efopen(const char * fname, const char * mode);
void * emalloc(size_t nbytes);
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
	
	static char curr_text[SUB_STR_SZ];
	CHTbl instr_htbl_, lbls_htbl_;
	
	instr_htbl = &instr_htbl_;
	lbls_htbl = &lbls_htbl_;
	
	if ( (chtbl_init(instr_htbl, BUCKETS, hash_inst, compar_inst, NULL) < 0) ||
		 (chtbl_init(lbls_htbl, BUCKETS, hash_lbl, compar_lbl, free_labels) < 0) )
	{
		fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: hash table initializatoin failed\n");
		quit();
	}
	
	fin = argv[1];
	fout = argv[3];
	
	// run the preprocessor
	char * run_ppstr = emalloc(strlen(ppexenm) + strlen(fin) + 8);
	sprintf(run_ppstr, "%s %s", ppexenm, fin);
	
	// check file
	FILE * input_file = efopen(fin, "r");
	fclose(input_file);
	
	if (is_preproc_here())
	{
		if (system(run_ppstr) == 0)
		{
			// if the preprocessor did it's job, new input file
			sprintf(run_ppstr, "%s%s", fin, ppext);
			fin = run_ppstr;
		}
		else
		{
			free(run_ppstr);
			run_ppstr = NULL;
		}
	}
	else
	{
		printf("%s: ", exenm), printf("Warning: %s: The preprocessor should be in the same directory\n", exenm);
		printf("but it's not here. Continuing anyway.\n");
	}
	
	int i;
	for (i = 0; i < INSTR_COUNT; ++i)
		chtbl_insert(instr_htbl, &mcode[i]);
	
	input_file = efopen(fin, "r");
	Lexer.SetInput(input_file);
	Lexer.Init();
	in_buff = curr_text;
	
	token ctok;
	while ((ctok = Lexer.Current()) != EOI)
	{
		if (all_size > MAX_CODE)
			break;
	
		switch (ctok)
		{
			case TOK_INSTR:
				parse_instr();
				break;
			case TOK_LABEL:
				/* called from here it parses labels outside of any instruction only
				 * if a label is after a jump, parse_label() is called from parse_instr() */
				parse_label(LBL_ADDR);
				break;
			default:
				fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: ", curr_lineno);
				fprintf(stderr, "something not an instruction, address, or register < %s >\n",
						in_buff);
				quit();
				break;
		}
	}
	
	FILE * output_file = efopen(fout, "wb");
	if (all_size > MAX_CODE)
	{
		printf("%s: ", exenm), printf("Warning: resulting code is bigger than the maximum of %d bytes\n",
				MAX_CODE);
		printf("Only the first %d bytes will be saved in the binary\n", MAX_CODE);
				
		all_size = MAX_CODE;
	}
	
	eval_labels();
	
	if (fwrite(binary , all_size, 1, output_file) != 1)
	{
		fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: %s: output file was not written properly\n", exenm);
		quit();
	}

	puts("Compilation successful");

	free(run_ppstr);
	fclose(output_file);
	fclose(input_file);
	chtbl_destroy(lbls_htbl);
	chtbl_destroy(instr_htbl);
	return 0;
}

/* ---------------------------- PARSER FUNCTIONS START ----------------------------  */
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
					fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: invalid instruction < %s >\n", 
							curr_lineno, in_buff);
					quit();
					break;
			}
		}
		++all_size;
		
		if (Lexer.Current() == TOK_LITERAL)
			parse_address();
		else
			parse_label(LBL_JUMP);
		
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
				if (Lexer.Current() == TOK_LITERAL)
					parse_address();
				else
					parse_label(LBL_JUMP);
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
		fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: invalid instruction < %s >\n", 
				curr_lineno, in_buff);
		quit();
	}
					
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
	fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: bad register < %s >\n", 
			curr_lineno, in_buff);
	quit();
	return -1; // we never come here
}

void parse_label(char context)
{
	/* validate and process a label */
	e_match(TOK_LABEL);
	
	if (strlen(in_buff) < 3)
	{
		fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: bad label < %s >\n",
				curr_lineno, in_buff);
		quit();
	}
	
	label * curr_lbl = make_lbl_node(in_buff);
	if (curr_lbl->context != context)
	{
		if (LBL_ADDR == curr_lbl->context)
			fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: label < %s > should not end with '%c'\n", 
					curr_lineno, in_buff ,LBL_ADDR);
		else
			fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: label < %s > should end with '%c'\n", 
					curr_lineno, in_buff, LBL_ADDR);
		quit();
	}
	
	label * lblfound = curr_lbl;
	if (LBL_ADDR == curr_lbl->context && 
		chtbl_lookup(lbls_htbl, (void **)&lblfound) == 0)
	{
		lblfound = *(label **)lblfound;
		
		fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: duplicate labels < %s > on lines %d and %d\n",
			curr_lineno, in_buff, lblfound->lineno, curr_lbl->lineno);
		quit();
	}
	else
		chtbl_insert(lbls_htbl, curr_lbl);
	
	return;
}

void parse_address(void)
{
	/* reads a literal decimal or hex number */
	e_match(TOK_LITERAL);
	
	check_literal();
	
	int addr_state;
	unsigned int num;
	if ('X' == in_buff[1])
		addr_state = sscanf(in_buff, "%x", &num);
	else
	{
		if (strpbrk(in_buff, "ABCDEF") != NULL)
		{
			fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: < %s > ",
					curr_lineno, in_buff);
			fprintf(stderr, "Hex numbers should be prefixed with \"0x\"\n");
			quit();
		}
	
		addr_state = sscanf(in_buff, "%d", &num);
	}
		
	if (addr_state != 1 || num > 0xFF)
	{
		fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: invalid address < %s >\n", 
				curr_lineno, in_buff);
		quit();
	}
	
	binary[all_size] = num;
	return;
}

void check_literal(void)
{
	/* check if the literal number is valid */
	char * curr_ch;
	curr_ch = in_buff;
	
	// skip prefix
	if ('0' == *curr_ch)
		++curr_ch;
	if ('X' == *curr_ch)
		++curr_ch;
	
	while (*curr_ch)
	{
		if (!isxdigit(*curr_ch))
		{
			fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: invalid literal < %s >\n",
					curr_lineno, in_buff);
			quit();
		}
		++curr_ch;
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
/* ---------------------------- PARSER FUNCTIONS END ----------------------------  */

/* ---------------------------- HASH TABLE FUNCTIONS START ----------------------------  */
int hash_inst(const void * key)
{
	/* hash instructions in alphabetical order */
	return tolower(((instr *)key)->name[0]);
}

int compar_inst(const void * key1, const void * key2)
{
	/* for sorting and searching instructions */
	return strcmp((const char *)((instr *)key1)->name, ((instr *)key2)->name);
}

int hash_lbl(const void * key)
{
	/* hash labels in alphabetical order */
	return tolower(((label *)key)->lbl_str[1]);
}

int compar_lbl(const void * key1, const void * key2)
{
	/* for sorting and searching labels */
	return strcmp(&(((label *)key1)->lbl_str[1]), &(((label *)key2)->lbl_str[1]));
}

label * make_lbl_node(char * lbl)
{
	/* allocate memory for a new label structure
	 * find out the context of the label
	 * populate the structure */
	int endl = strlen(lbl);
	static int dec_lbl = 0;
	static const int xtra_space = 10;
	
	label * newlbl = emalloc(sizeof(*newlbl));

	newlbl->lbl_str = emalloc(endl + xtra_space);
	strcpy(newlbl->lbl_str, lbl);
	
	--endl;
	// check last character
	if (LBL_ADDR != newlbl->lbl_str[endl])
	{
		newlbl->context = LBL_JUMP;

		// decorate label with a number so we can remember them all
		sprintf(newlbl->lbl_str, "%s%c%d", newlbl->lbl_str, DEC_SEP, dec_lbl);
		++dec_lbl;
	}
	else
		newlbl->context = LBL_ADDR;
		
	newlbl->address = all_size;
	newlbl->lineno = curr_lineno;
	newlbl->visited = false;
	
	return newlbl;
}

void free_labels(void * lbl)
{
	/* free the allocated string and
	 * label memory */
	free(((label *)lbl)->lbl_str);
	free(lbl);
	return;
}

void eval_labels(void)
{
	/* resolve every recorded label
	 * check if any have been unused */
	int i;
	for (i = 0; i < BUCKETS; ++i)
	{
		if (lbls_htbl->table[i].head != NULL)
			list_apply_all(&lbls_htbl->table[i], resolve_lbl_addr, NULL);
	}
	
	// check for lone labels
	for (i = 0; i < BUCKETS; ++i)
	{
		if (lbls_htbl->table[i].head != NULL)
			list_apply_all(&lbls_htbl->table[i], look_lone_lbls, NULL);
	}
	
	return;
}

void resolve_lbl_addr(ListElmt * l_element, void * args)
{
	/* go through all jump destination labels
	 * remove decoration
	 * search for corresponding address label */
	label * lbl = (label *)l_element->data;
	
	if (LBL_ADDR == lbl->context)
		return;
	
	label dummy_, * dum, * searchlbl;
	dum = &dummy_;
	dum->lbl_str = emalloc(strlen(lbl->lbl_str)+1);
	
	strcpy(dum->lbl_str, lbl->lbl_str);
	
	// find decoration separator
	int i = 1;
	while (dum->lbl_str[i] != DEC_SEP)
		++i;
	
	// make jump label into address label
	dum->lbl_str[i++] = LBL_ADDR;
	dum->lbl_str[i] = NUL;
	
	searchlbl = dum;
	// search for address label
	if (chtbl_lookup(lbls_htbl, (void **)&searchlbl) == 0)
	{
		searchlbl = *(label **)searchlbl;
		
		// resolve the address
		binary[lbl->address] = searchlbl->address;
	}
	else
	{
		fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Err: line %d: target label < %s > doesn't exist\n",
				lbl->lineno, dum->lbl_str);
		quit();
	}
	lbl->visited = true;
	searchlbl->visited = true;
	
	free(dum->lbl_str);
	return;
}

void look_lone_lbls(ListElmt * l_element, void * args)
{
	/* let the user know if a label is left unused */
	label * lbl = (label *)l_element->data;
	
	if (false == lbl->visited)
		fprintf(stderr, "%s: ", exenm), fprintf(stderr, "Warning: line %d: unused label < %s >\n",
				lbl->lineno, lbl->lbl_str);
	
	return;
}
/* ---------------------------- HASH TABLE FUNCTIONS END ----------------------------  */

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
