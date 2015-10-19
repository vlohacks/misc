#include "dllist.h"

void dllist_init(dllist_t * list) {
    list->first = list->last = 0;
    list->count = 0;
}

dllist_element_t * dllist_insert (dllist_t * list, dllist_element_t * parent, dllist_element_t * child, const dllist_element_insertpos_t pos) {

	dllist_element_t * tmp;

	switch (pos) {
	case DLLIST_INSERT_BEFORE:		
		tmp = parent->prev;
		child->next = parent;
		child->prev = tmp;
		parent->prev = child;
		if (tmp)
			tmp->next = child;
			
		break;

	case DLLIST_INSERT_AFTER:		
		tmp = parent->next;
		child->prev = parent;
		child->next = tmp;
		parent->next = child;
		if (tmp)
			tmp->prev = child;
		break;

	default:
		return 0;
		break;

	}

	if (child->prev == 0)
		list->first = child;

	if (child->next == 0)
		list->last = child;
        
        list->count++;

	return child;
	
}


dllist_element_t * dllist_remove (dllist_t * list, dllist_element_t * child) {
	if (child->next == 0)
		list->last = child->prev;
	else
		child->next->prev = child->prev;

	if (child->prev == 0)
		list->first = child->next;
	else
		child->prev->next = child->next;

	child->prev = child->next = 0;
        
        list->count--;

	return child;
}

void dllist_push(dllist_t * list, dllist_element_t * child) {

    if (list->count) {
        list->last->next = child;
        child->prev = list->last;
        list->last = child;
    } else { 
        list->last = child;
        list->first = child;
    }
    
    list->count++;
}

dllist_element_t * dllist_pop(dllist_t * list) {
    if (list->count == 0)
        return 0;
    
    dllist_element_t * ret = list->last;
    list->last = ret->prev;
    if (list->last)
        list->last->next = 0;
        
    ret->prev = 0;
    list->count--;
    
    if (list->count == 0)
        list->first = 0;
    
    return ret;
}
