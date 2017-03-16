/* chtbl.h -- public interface for a chained hash table */
/* v1.0 */
#ifndef CHTBL_H
#define CHTBL_H

#include <stdlib.h> // for NULL
#include "list.h"

enum {CHTBL_ELMT_EXISTS = 1};

/* structure for the chained table */
typedef struct CHTbl_ {
	int buckets;
	int (*h)(const void * key);
	int (*compar)(const void * key1, const void * key2);
	void (*destroy)(void * data);
	int size;
	List * table; // pointer to the array of linked lists
} CHTbl;

/* public interface */

int chtbl_init(CHTbl * htbl, int buckets, 
				int (*h)(const void * key), 
				int (*compar)(const void * key1, const void * key2), 
				void (*destroy)(void * data));
/* 
returns: 0 on success, -1 otherwise

description: Initializes the hash table. The number of buckets is specified by buckets, h() specifies
the hash function, compar is a function for comparing two elements, destroy provides a facility to
free the memory for the members of the hash table if the user wishes to do so.

complexity: O(buckets) 
*/

void chtbl_destroy(CHTbl * htbl);
/*
returns: nothing

description: Destroys the hash table specified by htbl.

complexity: O(buckets)
*/

int chtbl_insert(CHTbl * htbl, const void * data);
/*
returns: 0 on success, -1 on failure, CHTBL_ELMT_EXISTS if the element exists in the table

description: Inserts an element in the hash table.

complexity: O(1)
*/

int chtbl_remove(CHTbl * htbl, void ** data);
/*
returns: 0 on success, -1 on failure

description: Removes the element matching data. Upon return data points to the removed element.

complexity: O(1)
*/

int chtbl_lookup(const CHTbl * htbl, void ** data);				
/*
returns: 0 on success, -1 on failure

description: Searches for data in the table. If it's found, upon return data points to that element.

complexity: O(1)
*/

#endif