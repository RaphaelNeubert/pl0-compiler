
struct stack_head {
    int size;
    struct stack_element *top;
};

struct stack_head* create_stack();
int stack_push(struct stack_head *stack, void *item);
void* stack_pop(struct stack_head *stack);
