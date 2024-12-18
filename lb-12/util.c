#include "util.h"

void *array_at(const array_t *arr, int idx)
{
    assert(arr != NULL);
    assert(idx >= 0);
    assert(idx < arr->len);
    return (char*)arr->items + idx * arr->item_size;
}

void push(stack_t **stack, void *value)
{
    stack_t *new_stack = malloc(sizeof(stack_t));
    new_stack->value = value;
    new_stack->next = *stack;
    *stack = new_stack;
}

void *pop(stack_t **stack)
{
    assert(stack != NULL && "Stack underflow");
    void *value = (*stack)->value;
    stack_t *tmp = *stack;
    *stack = (*stack)->next;
    free(tmp);
    return value;
}
