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
} t_list;

typedef struct set{
	char fname[20];
	char lname[20];
	char num[20];
} t_set;
t_list* create_list();

int insert_head(t_list* list, void* data);
int insert_tail(t_list* list, void* data);

void* get_first(t_list* list);
void* get_next(t_list*list);
void* get_index(t_list* list, int idx);

void remove_curr(t_list* list);
void delete_list(t_list* list);


void load_data(char* path, t_list* list);
void write_data(char* path, t_list* list);

