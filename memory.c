#include <stdio.h>
#include <stdlib.h>


#include "memory.h"
#include "garbage_collector.h"


static size_t bytesAllocated = 0;
static size_t threshold = 1024 * 1024;


void *reallocate(void *pointer, int oldSize, int size, Interpreter *interpreter) {
    if (size == 0) {
        if (interpreter) {
            bytesAllocated -= oldSize;
        }
        free(pointer);
        return NULL;
    }

    if (interpreter && bytesAllocated >= threshold) {
        collect(interpreter);
        bytesAllocated = 0;
    }

    if (interpreter) {
        bytesAllocated += size;

    }

    uint8_t *result = realloc(pointer, size);
    return result;
}
