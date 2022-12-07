#include <stdlib.h>
#include "stack.h"

struct stack_element {
    void *item;
    struct stack_element *next;
};

struct stack_head* create_stack()
{
    struct stack_head *stack=malloc(sizeof(struct stack_head));
    if (stack) {
        stack->size=0;
        stack->top=NULL;
    }
    return stack;
}

int stack_push(struct stack_head *stack, void *item)
{
    struct stack_element *selem=malloc(sizeof(struct stack_element));
    if (selem) {
        selem->next=stack->top;
        selem->item=item;

        stack->top=selem;
        stack->size++;

        return 1;
    }
    return 0;
}

void* stack_pop(struct stack_head *stack)
{
    struct stack_element *old_top=stack->top;
    if (old_top) {
        void *item=old_top->item;
        stack->top=old_top->next;
        free(old_top);
        return item;
    }
    return NULL;
}

