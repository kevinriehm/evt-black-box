/*
 * A set of utility macros to deal with generic linked lists, where each
 * element is represented by a pointer to a structure containing two pointers
 * (to the same structure type) named "prev" and "next".
 *
 * e.g.,
 *  typedef struct linked_list_struct {
 *      struct linked_list_struct *prev;
 *      struct linked_list_struct *next;
 *  } *linked_list_element_t;
 */

// Find the first element in the list and store it in head
#define LL_FIND_HEAD(head, element) { \
	if((head) = (element)) \
		while((head)->prev && (head)->prev != (element)) \
			(head) = (head)->prev; \
}

// Append tail to body
#define LL_APPEND(body, tail) { \
	if(body) body->next = tail; \
	if(tail) tail->prev = body; \
}

