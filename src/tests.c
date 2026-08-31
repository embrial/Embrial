#define LANG_LOGGING_IMPLEMENTATION

#include "arena.c"

typedef Arena IntArena;
int main(void) {
    beginTest("Arena testing");
    IntArena arena = {0};
    Arena_init(int, &arena, 0);
    log_assertEq(arena.capacity, 1024, "%zu", "%d");
    log_assertEq(arena.status, ARENA_STATUS_OK, "%d", "%d");
    log_assertEq(arena.elementSize, sizeof(int), "%zu", "%zu");
    log_assertEq(arena.elementCount, 0, "%zu", "%d");
    Arena_free(&arena);
    Arena_init(int, &arena, 2048);
    log_assertEq(arena.capacity, 2048, "%zu", "%d");
    log_assertEq(arena.status, ARENA_STATUS_OK, "%d", "%d");
    log_assertEq(arena.elementSize, sizeof(int), "%zu", "%zu");
    log_assertEq(arena.elementCount, 0, "%zu", "%d");
    int x = 1;
    Arena_push(&arena, &x);
    x = 2;
    Arena_push(&arena, &x);
    x = 42;
    Arena_pop(&arena, &x);
    log_assertEq(2, x, "%d", "%d");
    Arena_pop(&arena, &x);
    log_assertEq(1, x, "%d", "%d");
    assertEq(Arena_pop(&arena, &x), 0, "%d", "%d");
    log_assertEq(1, x, "%d", "%d");
    log_assert(arena.status == ARENA_STATUS_OK);

    beginTest("Epic");

    return 0;
}
