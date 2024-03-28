
#include <stdlib.h>
#include "mjvm_heap.h"

static uint32_t objectCount = 0;

void *MJVM_Malloc(uint32_t size) {
    void *ret = malloc(size);
    if(ret == 0)
        throw "not enough memory to allocate";
    objectCount++;
    return ret;
}

void MJVM_Free(void *p) {
    objectCount--;
    free(p);
}
