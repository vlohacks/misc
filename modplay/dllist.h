#ifndef _DLLIST_H
#define _DLLIST_H


struct dllist_element_s {
	struct dllist_element_s * next;
	struct dllist_element_s * prev;
	void * data;
};

typedef struct dllist_element_s dllist_element_t;

typedef struct {
	dllist_element_t * first;
	dllist_element_t * last;
        int count;
} dllist_t;

typedef enum {
	DLLIST_INSERT_BEFORE,
	DLLIST_INSERT_AFTER
} dllist_element_insertpos_t;

dllist_element_t * dllist_insert(dllist_t * list, dllist_element_t * parent, dllist_element_t * child, const dllist_element_insertpos_t pos);
dllist_element_t * dllist_remove (dllist_t * list, dllist_element_t * child);
void dllist_push(dllist_t * list, dllist_element_t * child);
dllist_element_t * dllist_pop(dllist_t * list);

#endif
