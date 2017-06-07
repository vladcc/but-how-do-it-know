/* preproc.c -- generic preprocessor implementation */
/* ver. 1.02 */

/* Performs textual substituion. */

/* Author: Vladimir Dinev */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "../adt/chtbl.h"
#include "../adt/list.h"
#include "preproc.h"

#define BUFF_SZ 	256		// generic buffer size
#define BUCKETS 	128		// hash table buckets
#define DRV_START	'%'		// '%' marks a directive
#define NUL			'\0'	// ascii null
#define NEW_LINE	'\n'	// new line
#define COMMENT		'#'		// '#' begins a comment
#define DASH		'-'		// cmd line args begin with '-'
#define HELP		'h'		// help cmd line options
#define VERS		'v'		// version cmd line options
// remove a tailing '\n'
#define eat_nl(str)	{int endc;\
					 if ('\n' == str[(endc = strlen(str)-1)]) str[endc] = '\0';}
#define print_use()	printf("Use:  %s <input file>\n", ppexenm)
#define help_opt()	printf("Help: %s %c%c\n", ppexenm, DASH, HELP)
#define print_ver()	printf("%s %s\n", ppexenm, ver)

// the directive structure
typedef struct directive_ {
	int type;		// directive type
	char * dv;		// directive string
	char * aux;		// auxiliary informaion
} directive;

// supported directives
enum {DEFINE, NUM_DTVS};
char * directives[NUM_DTVS] = {
	"%define "						// %define works just like #define in C
};

char exenm[] = "preproc";			// internal executable name
char ver[] = "v1.02";				// executable version
int lineno = 0;						// curret line number
char input_buff[BUFF_SZ];			// the input buffer; lines are read in here
char * inbuff_ptr;					// points to the current character in input_buff
CHTbl * drvs_htbl;					// a pointer to the hash table holding the directive strings
FILE * infile, * outfile;			// input and output file pointers

// preprocessor functions
void get_directives(void);
void drv_define(void);

// hash table functions
int hash_drv(const void * key);
int compar_drv(const void * key1, const void * key2);
directive * make_drv_node(int type, char * drvline);
void free_drvs(void * drv);

// service functions
char * read_line(void);
char * read_word(void);
void eat_sep(void);
void eat_comnt(void);
FILE * efopen(const char * fname, const char * mode);
void * emalloc(size_t nbytes);
void print_help(void);
void quit(void);

/* ------------------------- MAIN FUNCTION START ------------------------- */
int main(int argc, char * argv[])
{
	/* parse command line
	 * set up hash table
	 * process the input file */
	 
	if (argc > 1)
	{
		if (DASH == argv[1][0])
		{
			if (HELP == argv[1][1])
				print_help();
			else if (VERS == argv[1][1])
				print_ver();
			else
			{
				fprintf(stderr, "Err: unrecognized option \"%s\"\n", argv[1]);
				print_use();
				help_opt();
			}
			
			return -2;
		}
	}
	 
	if (argc != 2)
	{
		print_use();
		help_opt();
		return -2;
	}
	
	CHTbl drvs_htbl_;
	drvs_htbl = &drvs_htbl_;
	
	if (chtbl_init(drvs_htbl, BUCKETS, hash_drv, compar_drv, free_drvs) < 0)
	{
		fprintf(stderr, "Err: hash table initializatoin failed\n");
		return -1;
	}
	
	infile = efopen(argv[1], "r");
	char * outfname = emalloc(strlen(argv[1]) + strlen(ppext) + 1);
	strcpy(outfname, argv[1]);
	strcat(outfname, ppext);
	outfile = efopen(outfname, "w");
	
	read_line();
	// no legal directives
	if (DRV_START != *inbuff_ptr)
	{			
		fclose(infile);
		fclose(outfile);
		chtbl_destroy(drvs_htbl);
		remove(outfname);
		free(outfname);
		return -1;
	}
	
	get_directives();
	drv_define();
	
	fclose(outfile);
	fclose(infile);
	free(outfname);
	chtbl_destroy(drvs_htbl);
	return 0;
}
/* ------------------------- MAIN FUNCTION END ------------------------- */

/* ------------------------- PREPROCESSOR FUNCTIONS START ------------------------- */
void get_directives(void)
{
	/* read and save directives */
	directive * dins;
	do
	{
		if (DRV_START == *inbuff_ptr)
		{
			// see if we have a legal directive on the current line
			if (strstr(input_buff, directives[DEFINE]) != NULL)
			{
				dins = make_drv_node(DEFINE, inbuff_ptr);
				if (chtbl_insert(drvs_htbl, dins) == CHTBL_ELMT_EXISTS)
				{
					fprintf(stderr, 
							"Warning: line %d: duplicate %sfor < %s %s > ignored\n",
							lineno, directives[DEFINE], dins->dv, dins->aux);
					free(dins);
				}
				// output as comment to preserve the number of lines
				fprintf(outfile, "#%s", input_buff);
			}	
			else
			{
				eat_nl(input_buff);
				fprintf(stderr, "Err: line %d: unknown directive < %s >\n",
						lineno, input_buff);
				quit();
			}
		}
		else
			break;
			
		input_buff[0] = NUL;
	} while (read_line() != NULL);
	
	if (NUL == input_buff[0])
	{
		fprintf(stderr, "Err: nothing after directives\n");
		quit();
	}
	
	return;
}

void drv_define(void)
{
	/* perform literal text substitution */
	char * wrd;
	directive dummy_, * dum;
	
	do
	{
		while ((wrd = read_word()) != NULL)
		{
			dum = &dummy_;
			dum->dv = wrd;
			
			// look for the substring in the hash table
			if (chtbl_lookup(drvs_htbl, (void **)&dum) == 0)
			{
				// output substituted text
				dum = *(directive **)dum;
				fprintf(outfile, "%s", dum->aux);
			}
			else
			{
				// output original substring
				fprintf(outfile, "%s", wrd);
			}
			// process comment
			eat_comnt();
		}
	} while (read_line() != NULL);
	
	return;
}
/* ------------------------- PREPROCESSOR FUNCTIONS END ------------------------- */

/* ------------------------- HASH TABLE FUNCTIONS START ------------------------- */
int hash_drv(const void * key)
{
	/* hash directives in alphabetical order */
	return tolower(((directive *)key)->dv[0]);
}

int compar_drv(const void * key1, const void * key2)
{
	/* for sorting and searching directive */
	return strcmp(&(((directive *)key1)->dv[0]), &(((directive *)key2)->dv[0]));
}

directive * make_drv_node(int type, char * drvline)
{
	/* allocate memory for a new directive structure
	 * save type
	 * save associated strings */
	static char dv[BUFF_SZ] = {NUL};
	static char aux[BUFF_SZ] = {NUL};
	
	directive * newdrv = emalloc(sizeof(*newdrv));
	
	if (DEFINE == type)
	{
		if (sscanf(drvline, "%*s %s %s", dv, aux) != 2)
		{
			eat_nl(drvline);
			fprintf(stderr, "Err: line %d: incomplete directive < %s >\n",
					lineno, drvline);
			quit();
		}
		newdrv->type = DEFINE;
		newdrv->dv = emalloc(strlen(dv)+1);
		newdrv->aux = emalloc(strlen(aux)+1);
		strcpy(newdrv->dv, dv);
		strcpy(newdrv->aux, aux);
	}
	
	return newdrv;
}

void free_drvs(void * drv)
{
	/* free the allocated string and
	 * directive memory */
	directive * dp = (directive *)drv;
	free(dp->dv);
	
	if (DEFINE == dp->type)
		free(dp->aux);
		
	free(dp);
	return;
}
/* ------------------------- HASH TABLE FUNCTIONS END ------------------------- */

/* ------------------------- SERVICE FUNCTIONS START ------------------------- */
char * read_line(void)
{
	/* read a line from the input file */
	char * retp;
	while ((retp = fgets(input_buff, BUFF_SZ, infile)) != NULL)
	{
		++lineno;
		inbuff_ptr = input_buff;
		
		// process comment
		eat_comnt();
		
		if (NUL != *inbuff_ptr)
			break;
	}
	
	return retp;
}

char * read_word(void)
{
	/* read a word from the input buffer */
	static char str_wrd[BUFF_SZ];
	
	if (NUL == *inbuff_ptr)
		return NULL;
	
	// process leading separators
	eat_sep();
	
	// get word
	int i = 0;
	while (!isspace(*inbuff_ptr) && 
		   ',' != *inbuff_ptr && 
		   COMMENT != *inbuff_ptr && 
		   NUL != *inbuff_ptr)
		str_wrd[i++] = *inbuff_ptr++;
	
	str_wrd[i] = NUL;
	
	return str_wrd;
}

void eat_sep(void)
{
	/* output/skip separators */
	while (isspace(*inbuff_ptr) || ',' == *inbuff_ptr)
	{
		fputc(*inbuff_ptr, outfile);
		++inbuff_ptr;
	}
		
	return;
}

void eat_comnt(void)
{
	/* output/skip comments */
	
	// process leading separators
	eat_sep();
	
	// output comment untill the end of the string
	if (COMMENT == *inbuff_ptr)
	{
		while (NUL != *inbuff_ptr)
		{
			fputc(*inbuff_ptr, outfile);
			++inbuff_ptr;
		}
	}
	
	return;
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

void * emalloc(size_t nbytes)
{
	/* allocate memory or die with an error */
	void * newmem;
	
	if ((newmem = malloc(nbytes)) == NULL)
	{
		fprintf(stderr, "Err: memory allocation failed\n");
		exit(EXIT_FAILURE);
	}
	
	return newmem;
}

void print_help(void)
{
	/* show help */
	printf("%s -- text file preprocessor\n", ppexenm);
	print_use();
	help_opt();
	printf("Ver.: %s %c%c\n", ppexenm, DASH, VERS);
	printf("\nDescription\n");
	printf("%s processes a text file according to a set of directives.\n", ppexenm);
	printf("If %s finds a directive inside the input file, it\n", ppexenm);
	printf("processes the file, leaving the result in another file named\n");
	printf("<input file>%s, returning 0 to the OS.\n", ppext);
	printf("If %s does not find any directives, it returns non-zero and exits.\n", ppexenm);
	printf("If an error occurs, %s prints the error message and quits with\n", ppexenm);
	printf("EXIT_FAILURE.\n");
	printf("Regardless of outcome, the input file is never modified.\n");
	printf("All directives begin with a '%%' immediately followed by the\n");
	printf("directive name. The directives must be gathered at the beginning of\n");
	printf("the file, placed at the start of the line, one directive per line.\n");
	printf("They can only be preceded by comment lines beginning with a '%c'\n", COMMENT);
	printf("The maximum supported line length is %d characters.\n", BUFF_SZ-1);
	printf("\nSupported directives\n");
	printf("1. %s\n", directives[DEFINE]);
	printf("Syntax: %s<a> <b>\n", directives[DEFINE]);
	printf("Substitutes <a> with <b>\n");
	printf("<a> and <b> are defined as any sequence of printable characters\n");
	printf("delimited by a space or a comma.\n");
	return;
}

void quit(void)
{
	/* hcf */
	puts("Preprocessing aborted");
	fclose(infile);
	fclose(outfile);
	exit(EXIT_FAILURE);
	return;
}
/* ------------------------- SERVICE FUNCTIONS END ------------------------- */
