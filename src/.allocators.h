#ifndef ALLOCATORS_H
#define ALLOCATORS_H

/*
Allocators:
    does the allocator deal with one type or many?
    how is the backing memory for the allocator layed out?
    how is memory freed back to the allocator?

Arena allocator:
    resizeable, when it runs out of space a new chunk is provided
    many types
    free all at once
Linear allocator:
    non resizable
    many types
    free all at once
Stack allocator:
    non resiable
    many type
    free first in first out
Pool allocator
    resizable, when space runs out more chunks can be provided
    one type
    return slot to a free list
Slab allocator
    resizable, allocate more slabs
    one type per slab
    return object to free list
Buddy allocator
    resizable, more memory regions/pages
    many types stored in powers of 2
    free by merging with matching budy

interface Allocator {
    alloc fn <T>(count usize) []T;
    realloc fn <T>(mem [$]T, count usize);
    dealloc fn <T>(mew [$]T);
}
module allocator {
    struct Heap; # compiler magic
    module Heap; # compiler magic
    module chunked {
        struct buddy; # uses unsafe
        module buddy; # uses unsafe
        expose struct Stack<T> {
            chunkSize usize;
            chunks List<[]T>;
            cursor usize;
        }
        module Example {
            interface T {
                fn doThing();
            }

            fn foo<interface T>(value T) {
                value.doThing();
            }
            # equivalent to
            fn foo<T implements T>(value T) {
                #  ^            ^        ^
                #  |            |        use of the generic type T defined in
                #  |            |        scope of foo
                #  |            search for an interface outside of foo's scope
                #  |            for an interface named T. Finds Example.T
                #  declaration of the generic type T in the scope of foo
                value.doThing();
            }
    }
        expose module Stack {
            fn create<
                T,
                interface List  # the second generics arg of this function must
                                # implement List, the name of the type that
                                # implements List in the functions scope is the
                                # same as the interface.
            >(chunkSize usize, chunkCount usize) !Stack<T> {
                const stack Stack {
                    .chunkSize = chunkSize,
                    .chunks = try list.create<[]T>($chunkCount);
                };
                const chunks [][]T = try Heap.split(
                    try Heap.alloc<T>(chunkSize * chunkCount),
                    $chunkCount
                );
                try stack.chunks.addAll(chunks);
                return stack;
            }
        }
        module stack;
        struct pool;
        module pool;
    }
    module backed {
        struct stack;
        module stack;
        struct pool;
        module pool;
    }
}
Heap
ChunkedBinaryAllocator
ChunkedStackAllocator
ChunkedPoolAllocator
StackAllocator
PoolAllocator

Stack allocator
*/
#define DEFAULT_ARENA_SIZE 1024

#include "logging.h"

typedef enum {
    ALLOCATOR_STATUS_UNINITIALIZED,
    ALLOCATOR_STATUS_OK,
    ALLOCATOR_STATUS_OUT_OF_MEMORY,
    ALLOCATOR_STATUS_OVERFLOW
} AllocatorStatus;
#define ArenaAllocator(kind) struct {\
    kind *buffer;\
    size_t count;\
    size_t capacity;\
    AllocatorStatus status\
}
#define PageAllocator(kind) struct {\
    kind **pages;\
    size_t pageCount;\
    size_t pageCapacity;\
    AllocatorStatus status\
}
typedef ArenaAllocator(void) ArenaAllocator;
typedef PageAllocator(void) PageAllocator;

#define ArenaAllocator_init(allocator, initCapacity) ArenaAllocator_Init(\
    (ArenaAllocator *) allocator,\
    sizeof(allocator->buffer),\
    initCapacity\
);
void ArenaAllocator_Init(
    ArenaAllocator *arena,
    size_t elementSize,
    size_t initCapacity
) {
    if (!arena) return;
    if (arena->status != ALLOCATOR_STATUS_UNINITIALIZED) {
        fatal("ArenaAllocator_init, allocator allready initialized");
        return;
    }

    arena->elementSize = elementSize;

    size_t capacity = initCapacity;
    if (DEFAULT_ARENA_SIZE > capacity) capacity = DEFAULT_ARENA_SIZE;

    arena->buffer = malloc(elementSize * capacity);
    if (!arena->buffer) {
        arena->status = ALLOCATOR_STATUS_OUT_OF_MEMORY;
        return;
    }

    arena->status = ALLOCATOR_STATUS_OK;
}
#define ArenaAllocator_free(arena) do {\
    free(arena->buffer);\
    arena->count = 0;\
    arena->capacity = 0;\
    arena->status = ALLOCATOR_STATUS_UNINITIALIZED;\
while (0);
#define ArenaAllocator_get(arena, index)\
(typeof(arena->buffer) *)ArenaAllocator_Get(\
    (ArenaAllocator *)arena,\
    sizeof(allocator->buffer),\
    index\
);
void *ArenaAllocator_Get(ArenaAllocator *arena, size_t elementSize, size_t index) {
    if (!arena || arena->status != ALLOCATOR_STATUS_OK) return NULL;

    if (index > arena->capacity) {
        size_t newCapacity = arena->capacity;
        if (DEFAULT_ARENA_SIZE > newCapacity) newCapacity = DEFUALT_ARENA_SIZE;
        while (index > newCapacity) newCapacity *= 2;

        void *temp = realloc(arena->buffer, elementSize * capacity);
        if (!temp) {
            arena->status = ALLOCATOR_STATUS_OUT_OF_MEMORY;
            return NULL;
        }

        arena->buffer = temp;
        arena->capacity = newCapacity;
        if (arena->count < index) arena->count = index + 1;
    }
}

#endif
