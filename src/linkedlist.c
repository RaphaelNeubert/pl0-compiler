#include <stdlib.h>
#include "linkedlist.h"


t_list* create_list(){
	t_list* ptmp=malloc(sizeof(t_list));
	if (ptmp){
		ptmp->head.next=&(ptmp->head);
		ptmp->head.prev=&(ptmp->head);
		ptmp->head.item=NULL;
		ptmp->curr=NULL;
		ptmp->size=0;
	}
	return ptmp;
}

static int insert_after(t_node* pbef, void* data){
	t_node* ptmp=malloc(sizeof(t_node));
	if (ptmp){
		ptmp->next=pbef->next;
		ptmp->prev=pbef;
		pbef->next=ptmp;
		ptmp->next->prev=ptmp;
		ptmp->item=data;
		return 1;
	}
	return 0;
}

int insert_head(t_list* list, void* data){
	list->size++;
	return insert_after(&(list->head), data);
}

int insert_tail(t_list* list, void* data){
	list->size++;
	return insert_after(list->head.prev, data);
}
int insert_curr(t_list* list, void* data){
	list->size++;
	return insert_after(list->curr, data);
}

void* get_first(t_list* list){
	list->curr=list->head.next;
	return (list->head.next->item);
}

void* get_last(t_list* list){
	list->curr=list->head.prev;
	return (list->head.prev->item);
}

void* get_next(t_list* list){
	list->curr=list->curr->next;
	return (list->curr->item);
}

void* get_prev(t_list* list){
	list->curr=list->curr->prev;
	return (list->curr->item);
}

void* get_index(t_list* list, int idx){
	int i;
	get_first(list);
	for (i=0; i<idx; i++){
		list->curr=list->curr->next;
	}
	return (list->curr->item);
}

int list_size(t_list* list){
	return (list->size);
}

void remove_curr(t_list* list){
	if (list->curr){
		list->curr->prev->next=list->curr->next;
		list->curr->next->prev=list->curr->prev;
		free(list->curr);
		list->curr=NULL;
		list->size--;
	}
}

