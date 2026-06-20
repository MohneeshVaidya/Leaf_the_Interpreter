#ifndef leaf_memory_h
#define leaf_memory_h


#include "forward.h"


#define INIT_CAPACITY               (10)
#define GROW_CAPACITY(capacity)     ((capacity < INIT_CAPACITY) ? INIT_CAPACITY : (2 * capacity))


#define REALLOCATE(pointer, oldSize, size, interpreter, type) \
    ((type *)reallocate(pointer, oldSize * sizeof(type), size * sizeof(type), interpreter))


#define FREE(pointer, oldSize, type) \
    ((type *)reallocate(pointer, oldSize * sizeof(type), 0, NULL))


#define ENSURE_CAPACITY(container, type)                                                                            \
    do {                                                                                                            \
        if ((container)->count == (container)->capacity) {                                                          \
            int newCapacity = GROW_CAPACITY((container)->capacity);                                                 \
            (container)->array = REALLOCATE((container)->array, (container)->capacity, newCapacity, NULL, type);    \
            (container)->capacity = newCapacity;                                                                    \
        }                                                                                                           \
    } while (false)


void *reallocate(void *pointer,
                 int oldSize,
                 int size,
                 Interpreter *interpreter);


#endif
