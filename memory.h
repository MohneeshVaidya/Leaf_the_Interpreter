#ifndef leaf_memory_h
#define leaf_memory_h


#include <stdint.h>


#define INIT_CAPACITY               (10)
#define GROW_CAPACITY(capacity)     ((capacity < INIT_CAPACITY) ? INIT_CAPACITY : (2 * capacity))


#define REALLOCATE(pointer, size, type) \
    ((type *)reallocate(pointer, size * sizeof(type)))


#define FREE(pointer, type) \
    ((type *)reallocate(pointer, 0))


#define ENSURE_CAPACITY(container, type)                                            \
    do {                                                                            \
        if ((container)->count == (container)->capacity) {                          \
            int newCapacity = GROW_CAPACITY((container)->capacity);                 \
            (container)->array = REALLOCATE((container)->array, newCapacity, type); \
            (container)->capacity = newCapacity;                                    \
        }                                                                           \
    } while (false)


void *reallocate(void *pointer, int size);


#endif
