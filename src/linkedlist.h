/*
Raphael Neubert

20/041/61
*/

typedef struct node{
	struct node *next;
	struct node *prev;
	void *item;
} t_node;

typedef struct list{
	t_node head;
	t_node *curr;
	int size;
} t_list;

t_list* create_list();

int insert_head(t_list* list, void* data);
int insert_tail(t_list* list, void* data);
int insert_curr(t_list* list, void* data);

void* get_first(t_list* list);
void* get_last(t_list* list);
void* get_next(t_list*list);
void* get_prev(t_list* list);
void* get_index(t_list* list, int idx);

int list_size(t_list* list);

void remove_curr(t_list* list);
