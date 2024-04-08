
#ifndef __MJVM_HEAP_H
#define __MJVM_HEAP_H

#include <stdint.h>
#include "mjvm_const_pool.h"

class MjvmHeap {
public:
    static void *malloc(uint32_t size);
    static void free(void *p);
private:
    MjvmHeap(void) = delete;
    MjvmHeap(const MjvmHeap &) = delete;
    void operator=(const MjvmHeap &) = delete;
};

#endif /* __MJVM_HEAP_H */
