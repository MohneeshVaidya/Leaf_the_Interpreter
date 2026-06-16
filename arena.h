#ifndef leaf_arena_h
#define leaf_arena_h


#include <stdint.h>


#define ARENA_CAPACITY  (5 * 1024 * 1024)
#define ARENA_ALIGNMENT (8)


#define ARENA_ALLOCATE(arena, type) \
    ((type *)arenaAllocate(arena, sizeof(type)))


typedef uint8_t byte;


typedef struct Arena {
    byte *memory;
    byte *current;
    size_t capacity;
} Arena;


void initArena(Arena *arena);
void freeArena(Arena *arena);
void *arenaAllocate(Arena *arena, size_t bytes);


#endif
