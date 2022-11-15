#include <stdlib.h>
#include "linkedlist.h"


struct list_head* create_list(){
	struct list_head* list=malloc(sizeof(struct list_head));
	if (list){
		list->head.next=&(list->head);
		list->head.prev=&(list->head);
		list->head.item=NULL;
		list->curr=NULL;
		list->size=0;
	}
	return list;
}

static int list_insert_after(struct list_node* pprev, void* data){
	struct list_node* ptmp=malloc(sizeof(struct list_node));
	if (ptmp){
		ptmp->next=pprev->next;
		ptmp->prev=pprev;
		pprev->next=ptmp;
		ptmp->next->prev=ptmp;
		ptmp->item=data;
		return 1;
	}
	return 0;
}

int list_insert_head(struct list_head* list, void* data){
	list->size++;
	return list_insert_after(&(list->head), data);
}

int list_insert_tail(struct list_head* list, void* data){
	list->size++;
	return list_insert_after(list->head.prev, data);
}
int list_insert_curr(struct list_head* list, void* data){
	list->size++;
	return list_insert_after(list->curr, data);
}

void* list_get_first(struct list_head* list){
	list->curr=list->head.next;
	return (list->head.next->item);
}

void* list_get_last(struct list_head* list){
	list->curr=list->head.prev;
	return (list->head.prev->item);
}

void* list_get_next(struct list_head* list){
	list->curr=list->curr->next;
	return (list->curr->item);
}

void* list_get_prev(struct list_head* list){
	list->curr=list->curr->prev;
	return (list->curr->item);
}

void* list_get_index(struct list_head* list, int idx){
	int i;
	list_get_first(list);
	for (i=0; i<idx; i++){
		list->curr=list->curr->next;
	}
	return (list->curr->item);
}

int list_size(struct list_head* list){
	return (list->size);
}

void list_remove_curr(struct list_head* list){
	if (list->curr){
		list->curr->prev->next=list->curr->next;
		list->curr->next->prev=list->curr->prev;
		free(list->curr);
		list->curr=NULL;
		list->size--;
	}
}

