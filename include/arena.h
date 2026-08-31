#ifndef ARENA_H
#define ARENA_H

/*
Header comment:
    File:       arena.h
    Project:    lang
    Created:    2026-08-19

    Copyright (C) 2026 Adrian Long
    Licensed under the MIT License.

Description:
    A generic, dynamically growing, single-type bump allocator.

    An Arena manages a collection of memory chunks where each chunk is a
    sequential allocation of a fixed number of a single type of element.
    Elements remain valid until the arena is popped or freed.

    The Arena grows automatically when a index is requested and the chunk that
    would contain the memory for that index is not allready allocated.
    Individual allocations cannot be freed, however elements can be accesed in a
    last in first out order using Arena_push, and Arena_pop respectivly.

    Arena is type-generic through the use of implicit casts to and from void
    pointers.

Usage:
    typedef Arena IntArena;

    int main() {
        IntArena buffer = {0};

        // Initialize the IntArena using the default number of chunk slots
        if (Arena_init(int, &arena, 0))) return 1;

        int x = 1;

        // Push 1 to the Arena
        if (Arena_push(&arena, &x)) goto error;
        

        // Push 2 to the Arena
        x = 2;
        if (Arena_push(&arena, &x)) goto error;

        // set the first element of the arena to 3
        int *p = Arena_get(&arena, 0);
        if (!p) goto error;
        printf("%d", *p); // prints 1
        *p = 3;

        Arena_pop(&arena, &x);
        printf("%d", x); // prints 2

        Arena_pop(&arena, &x);
        printf("%d", x); // prints 3
                                     
        return 0;
    error://
        Arena_free(arena);
        return 1;
    }
*/

// Includes:
#include "status.h"

// Constants:
#define MIN_ARENA_CAPACITY 1024
#define CHUNK_CAPACITY 1024

// Data structures:

typedef struct {
    void **chunks;
    size_t capacity;
    size_t elementCount;
    size_t elementSize;
} Arena;

// Functions:
#define Arena_init(type, arena, capacity)\
GenericArena_init(arena, sizeof(type), capacity)
Status GenericArena_init(Arena *arena, size_t elementSize, size_t capacity) {

void *Arena_get(Arena *arena, size_t index);
Status Arena_push(Arena *arena, void *element);
Status Arena_pop(Arena *arena, void *memory);
Status Arena_free(Arena *arena);

#endif
