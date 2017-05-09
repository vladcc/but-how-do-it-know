/* chtbl.c -- a chained hash table implementation */
/* v1.0 */
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "chtbl.h"

int chtbl_init(CHTbl * htbl, int buckets, 
				int (*h)(const void * key), 
				int (*compar)(const void * key1, const void * key2), 
				void (*destroy)(void * data))
{
	/* initialize the hash table */
	
	// allocate space
	if ( (htbl->table = (List *)malloc(buckets * sizeof(*(htbl->table))) ) == NULL )
		return -1;

	// initialize the buckets
	htbl->buckets = buckets;
	
	int i;
	for (i = 0; i < htbl->buckets; ++i)
		list_init(&htbl->table[i], compar, destroy);
	
	htbl->h = h;
	htbl->compar = compar;
	htbl->destroy = destroy;
	htbl->size = 0;
	
	return 0;
}

void chtbl_destroy(CHTbl * htbl)
{
	/* kill the hash table */
	int i;
	for (i = 0; i < htbl->buckets; ++i)
		list_destroy(&htbl->table[i]);
		
	free(htbl->table);
	
	// zero out memory
	memset(htbl, 0, sizeof(*htbl));
	
	return;
}

int chtbl_insert(CHTbl * htbl, const void * data)
{
	/* add to the hash table */
	int bucket, retval;
	void * temp = (void *)data;
	// do nothing if data is already in the table
	if (chtbl_lookup(htbl, &temp) == 0)
		return CHTBL_ELMT_EXISTS;
	
	// hash the key
	bucket = htbl->h(data) % htbl->buckets;
	
	// insert data
	if ( (retval = list_ins_first(&htbl->table[bucket], data)) == 0)
		htbl->size++;
	
	return retval;
}

int chtbl_remove(CHTbl * htbl, void ** data)
{
	/* remove from the hash table */
	ListElmt * elmt, * prev;
	// hash the data
	int bucket = htbl->h(*data) % htbl->buckets;
	
	prev = NULL;
	// search for the data
	for (elmt = (&htbl->table[bucket])->head; elmt != NULL; 
		elmt = elmt->next)
	{
		if (htbl->compar(*data, elmt->data) == 0)
		{
			if (list_rem_next(&htbl->table[bucket], prev, data) == 0)
			{
				htbl->size--;
				return 0;
			}
			else
				return -1;
		}
		
		prev = elmt;
	}
	
	return -1; // not found
}

int chtbl_lookup(const CHTbl * htbl, void ** data)
{
	/* search the hash table */
	
	// hash the data
	int bucket = htbl->h(*data) % htbl->buckets;
	// lookup
	ListElmt * found = list_find(&htbl->table[bucket], *data);
	
	*data = found;
	
	if (NULL == found)
		return -1;
		
	*data = found;
	
	return 0;
}