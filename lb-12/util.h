#ifndef UTIL_H
#define UTIL_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#define error(...)                     \
    do {                               \
        fwprintf(stderr, __VA_ARGS__); \
        exit(1);                       \
    } while (0)

typedef struct { int x, y, z; } vec3;

typedef struct { int x, y; } vec2;

typedef struct {
    void *items;
    int item_size;
    int len;
} array_t;

void *array_at(const array_t *arr, int idx);

typedef struct stack_s {
    void *value;
    struct stack_s *next;
} stack_t;

void push(stack_t **stack, void *value);
void *pop(stack_t **stack);

#endif // UTIL_H
