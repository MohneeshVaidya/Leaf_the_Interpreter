#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>


#include "arena.h"
#include "memory.h"


void initArena(Arena *arena) {
    arena->capacity = ARENA_CAPACITY;
    arena->memory = REALLOCATE(NULL, 0, arena->capacity, NULL, byte);
    arena->current = arena->memory;
}


void freeArena(Arena *arena) {
    FREE(arena->memory, arena->capacity, byte);
    arena->memory = arena->current = NULL;
    arena->capacity = 0;
}


void *arenaAllocate(Arena *arena, size_t bytes) {
    bytes = (bytes + ARENA_ALIGNMENT - 1) & ~(ARENA_ALIGNMENT - 1);
    if ((size_t)(arena->current - arena->memory) > arena->capacity - bytes) {
        fprintf(stderr, "out of memory\n");
        return NULL;
    }

    byte *pointer = arena->current;
    arena->current += bytes;

    return pointer;
}
