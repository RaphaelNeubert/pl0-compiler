/*
Raphael Neubert

20/041/61
*/

struct list_node {
	struct list_node *next;
	struct list_node *prev;
	void *item;
};

struct list_head {
	struct list_node head;
	struct list_node *curr;
	int size;
};

struct list_head* create_list();

int list_insert_head(struct list_head* list, void* data);
int list_insert_tail(struct list_head* list, void* data);
int list_insert_curr(struct list_head* list, void* data);

void* list_get_first(struct list_head* list);
void* list_get_last(struct list_head* list);
void* list_get_next(struct list_head* list);
void* list_get_prev(struct list_head* list);
void* list_get_index(struct list_head* list, int idx);

int list_size(struct list_head* list);

void list_remove_curr(struct list_head* list);
