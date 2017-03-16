/* list.h -- public interface for a singly linked list */
/* v1.0 */
/* Description: singly linked list accepting generic data via void pointers.
 * Supports inserting, removing, sorting, searching, traversing, and destroying.
 * Allocating memory for the data is the responsiblity of the user. 
 * When a single element is removed from the list, the memory for the ListElmt is deallocated
 * and a pointer to the data of the node is returned. The user may provide a destroy function
 * to deallocate the data memory. This function is applied only when list_destroy() 
 * is called. If none is provided, only the list is destroyed. */
 
#ifndef LIST_H_
#define LIST_H_

#include <stdlib.h> // for NULL

#define LIST_SORT_UP 	1
#define LIST_SORT_DOWN -1

/* structure for the elements */
typedef struct ListElmt_ {
	void * data;
	struct ListElmt_ * next;
} ListElmt;

/* structure for the list */
typedef struct List_ {
	int size;
	int (*compar)(const void * key1, const void * key2); // for sorting and searching
	void (*destroy)(void * data);
	ListElmt * head;
	ListElmt * tail;
} List;

/* public interface */
void list_init(List * list, int (*compar)(const void * key1, const void * key2), 
			   void (*destroy)(void * data));
/* 
returns: nothing

description: Initializes the linked list specified by list. Must be called before list can be used. 
destroy provides a user defined way to free memory when list_destroy is called. destroy can be NULL.

complexity: O(1) 
*/

void list_destroy(List * list);
/*
returns: nothing

description: Destroys list by calling destroy from list_init, provided it's not NULL. 
No other operations are permitted after calling list_destroy.

complexity: O(n)
*/

int list_apply_elmt(List * list, ListElmt * element, 
					void(*fun)(ListElmt * l_element, void * args), void * fun_args);
/*
returns: 0 on success, -1 on failure

description: Applies fun() to element; if element is NULL, fun() is applied from head to tail.

complexity: O(1) or O(n)
*/

#define list_apply_all(list, fun, args) list_apply_elmt((list), NULL, (fun), (args))
/*
returns: 0 on success, -1 on failure

description: Macro for list_apply_elmt(); applies fun() to all elements of list.

complexity: O(n)
*/

int list_ins_next(List * list, ListElmt * element, const void * data);
/*
returns: 0 on success, -1 on failure

description: Inserts an element after element in list. If element is NULL, the new one is inserted at the head of the list.
The new element contains a pointer to data, so the memory should remain valid as long as the element is in the list.
That's done by the caller.

complexity: O(1)
*/

#define list_ins_first(list, data) list_ins_next((list), NULL, (data))
/*
returns: 0 on success, -1 on failure

description: Macro for list_ins_next(); inserts at the head of the list

complexity: O(1)
*/

int list_rem_next(List * list, ListElmt * element, void ** data);
/*
returns: 0 on success, -1 on failure

description: Removes the element after element from list. If element is NULL, the element at the head is removed.
Upon return, data points to the data stored in the element that was removed. The caller manages memory.

complexity: O(1)
*/

#define list_rem_first(list, data) list_rem_next((list), NULL, (data))
/*
returns: 0 on success, -1 on failure

description: Macro for list_rem_next(); removes form the head of the list

complexity: O(1)
*/

#define list_is_head(list, element) ((element) == (list)->head ? 1 : 0)
/*
int list_is_head(const ListElmt * element);
returns: 1 if element is head of the list, 0 if not

description: Macro that determines whether the element is the head of a linked list.

complexity: O(1)
*/

#define list_is_tail(list, element) ((element)->next == NULL ? 1 : 0)
/*
int list_is_tail(const ListElmt * element);
returns: 1 if the element is tail, 0 if not

description: Macro that determines if element is tail.

complexity: O(1)
*/

int list_sort(List * list, int sort_up_down);
/*
returns: 0 on success, -1 of failure

description: Linked list sort function (bubble sort in this case). 1 sorts up, -1 sorts down.
Use LIST_SORT_UP and LIST_SORT_DOWN.

complexity: O(n*n)
*/

ListElmt * list_find_from(List * list, ListElmt * start, void * key);
/*
returns: pointer to the element containing the key, NULL if not found or on error

description: Linear search through list looking for key. If start is NULL,
searching starts from the head.

complexity: O(n)
*/

#define list_find(list, key) list_find_from((list), NULL, (key))
/*
returns: pointer to the element containing the key, NULL if not found or list is bad

description: Linear search through list looking for key.

complexity: O(n)
*/
#endif