/* list.c -- an implementation of a singly linked list */
/* See list.h for description. */
/* v1.0 */
#include <stdlib.h> // for NULL
#include <string.h> // for memset()
#include <stdio.h> 	// debug
#include <stdbool.h>
#include "list.h"

void list_init(List * list, int (*compar)(const void * key1, const void * key2),
			   void (*destroy)(void * data))
{
	/* initialize the list */
	list->size = 0;
	list->compar = compar;
	list->destroy = destroy;
	list->head = NULL;
	list->tail = NULL;
	
	return;
}

void list_destroy(List * list)
{
	/* remove each element */
	void * data;
	
	while (list->size > 0)
	{
		if ( (list_rem_next(list, NULL, (void **)&data) == 0) && (list->destroy != NULL) )
			list->destroy(data);
	}

	// no other operations are allowed - clear the structure
	memset(list, 0, sizeof(*list));
	
	return;
}

int list_ins_next(List * list, ListElmt * element, const void * data)
{
	/* insert as next element */
	ListElmt * new_element;
	
	// allocate storage
	if ( (new_element = (ListElmt *)malloc(sizeof(*new_element))) == NULL )
		return -1;
	
	new_element->data = (void *)data;
	
	// insert at the head
	if (NULL == element)
	{
		if (0 == list->size)
			list->tail = new_element;
		
		new_element->next = list->head;
		list->head = new_element;
	}
	else
	{
	// insert elsewhere than the head
		if (NULL == element->next)
			list->tail = new_element;
		
		new_element->next = element->next;
		element->next = new_element;
	}
	
	// adjust the size
	list->size++;
	
	return 0;
}

int list_rem_next(List * list, ListElmt * element, void ** data)
{
	/* remove next element */
	ListElmt * old_element;
	
	// don't remove from an empty list
	if (0 == list->size)
		return -1;
	
	// remove from the head
	if (NULL == element)
	{
		*data = list->head->data;
		old_element = list->head;
		list->head = list->head->next;
		
		if (1 == list->size)
			list->tail = NULL;
	}
	else
	{
	// remove from elsewhere
		if (NULL == element->next)
			return -1;
		
		*data = element->next->data;
		old_element = element->next;
		element->next = element->next->next;
		
		if (NULL == element->next)
			list->tail = element;
	}
	// free element memory
	free(old_element);
	
	// adjust size
	list->size--;
	
	return 0;
}

int list_apply_elmt(List * list, ListElmt * element, 
					void(*fun)(ListElmt * l_element, void * args), void * fun_args)
{
	/* applies fun() to element; if NULL == element, applies fun() to whole list */
	
	// go home if fun() is bad
	if (NULL == fun)
		return -1;
	
	if (NULL == element)
	{
		// apply to all elements
		ListElmt * tmp_element = list->head;
		while (tmp_element != NULL)
		{
			fun(tmp_element, fun_args);
			tmp_element = tmp_element->next;
		}
	}
	else // apply for current element
		fun(element, fun_args);
	
	return 0;
}

int list_sort(List * list, int sort_up_down)
{
	/* bubble sort */
	if ( (list->size < 2) || (list == NULL) || (NULL == list->compar))
		return -1;
	
	if ( (sort_up_down != LIST_SORT_UP) && (sort_up_down != LIST_SORT_DOWN) )
		return -1;
	
	ListElmt * el;
	void * tmp;
	bool swapped = true;
	int i;
	
	while (swapped)
	{
		swapped = false;
		for (i = 1, el = list->head; i < list->size; el = el->next, ++i)
		{
			if (list->compar(el->data, el->next->data) == sort_up_down)
			{
				// swap data pointers
				tmp = el->data;
				el->data = el->next->data;
				el->next->data = tmp;
				swapped = true;
			}
		}
	}
	
	return 0;
}

ListElmt * list_find_from(List * list, ListElmt * start, void * key)
{
	/* linear search for key through list */
	if ((list->size < 1) || (NULL == list) || (NULL == list->compar))
		return NULL;
	
	if (NULL == start)
		start = list->head;
	
	ListElmt * el;
	
	el = start;
	while (el != NULL)
	{
		if (list->compar(el->data, key) == 0)
			return el;
		
		el = el->next;
	}
	
	return NULL;
}