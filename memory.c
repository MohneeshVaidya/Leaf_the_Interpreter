#include <stdlib.h>


#include "memory.h"


void *reallocate(void *pointer, int size) {
    if (size == 0) {
        free(pointer);
        return NULL;
    }

    uint8_t *result = realloc(pointer, size);
    return result;
}
