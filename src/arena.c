#include "arena.h"

// Arena_init:
Status GenericArena_init(Arena *arena, size_t elementSize, size_t capacity) {
    if (capacity < MIN_ARENA_CAPACITY) capacity = MIN_ARENA_CAPACITY;

    arena->chunks = calloc(capacity, sizeof(void *));
    if (!arena->chunks) return STATUS_OUT_OF_MEMORY;

    void *temp = malloc(elementSize * CHUNK_CAPACITY);
    if (!temp) {
        free(arena->chunks);
        return STATUS_OUT_OF_MEMORY;
    }

    arena->chunks[0] = temp;
    arena->capacity = capacity;
    arena->elementCount = 0;
    arena->elementSize = elementSize;
    return STATUS_SUCCESS;
}

// Arena_get:
void *Arena_get(Arena *arena, size_t index) {
    *status = STATUS_OUT_OF_MEMORY;
    size_t chunkIndex = index / CHUNK_CAPACITY;
    size_t elementIndex = index % CHUNK_CAPACITY;

    // allocate more chunk slots as needed
    size_t arenaCapacity = arena->capacity;
    if (chunkIndex >= arenaCapacity) {
        do { arenaCapacity *= 2; } while (chunkIndex > arenaCapacity);
        void **temp = realloc(
            arena->chunks,
            sizeof(void *) * arenaCapacity
        );
        if (!temp) return NULL;

        // set all the newly allocated pointers to null pointers
        memset(
            arena->chunks + arena->capacity,
            0,
            sizeof(void *) * (arenaCapacity - arena->capacity)
        );

        arena->chunks = temp;
        arena->capacity = arenaCapacity;
    }

    // allocate the chunk if it was not allready allocated
    void *chunk = arena->chunks[chunkIndex];
    if (!chunk) {
        chunk = malloc(arena->elementSize * CHUNK_CAPACITY);
        if (!chunk) return NULL;
        arena->chunks[chunkIndex] = chunk;
    }
    
    *status = STATUS_SUCCESS;
    return (char *)chunk + elementIndex * arena->elementSize,
}

// Arena_push:
Status Arena_push(Arena *arena, void *element) {
    void *memory = Arena_get(arena, arena->elementCount, element);
    if (!memory) return STATUS_OUT_OF_MEMORY;

    memcpy(memory, element, arena->elementSize);
    arena->elementCount += 1;

    return STATUS_SUCCESS;
}

// Arena_pop:
Status Arena_pop(Arena *arena, void *memory) {
    if (arena->elementCount == 0) return STATUS_FAILURE;

    // not using Arena_get to avoid unessesary resize logic
    size_t index = arena->elementCount - 1;
    size_t chunkIndex = index / CHUNK_CAPACITY;
    size_t elementIndex = index % CHUNK_CAPACITY;

    assert(chunkIndex < arena->capacity);
    void *element = (char *)arena->chunks[chunkIndex] +
        elementIndex * arena->elementSize;
    assert(element);

    memcpy(memory, element, arena->elementSize);
    arena->elementCount = index;

    return STATUS_SUCCESS;
}

// Arena_free:
void Arena_free(Arena *arena) {
    if (!arena) return;
    for (size_t i = 0; i < arena->capacity; i++) {
        free(arena->chunks[i]);
    }
    free(arena->chunks);
    arena->chunks = NULL;
    arena->capacity = 0;
    arena->status = ARENA_STATUS_UNINITIALIZED;
    arena->elementSize = 0;
    arena->elementCount = 0;
}
#endif
