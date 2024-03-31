
#ifndef __MJVM_HEAP_H
#define __MJVM_HEAP_H

#include <stdint.h>
#include "mjvm_const_pool.h"

class MjvmObject {
public:
    const uint32_t size;
    const ConstUtf8 &type;
    uint8_t data[];
private:
    MjvmObject(uint32_t size, const ConstUtf8 &type);
    MjvmObject(const MjvmObject &) = delete;
    void operator=(const MjvmObject &) = delete;

    friend class MjvmHeap;
};

class MjvmHeap {
public:
    static void *malloc(uint32_t size);
    static void free(void *p);

    static MjvmObject *createNew(uint32_t size, const ConstUtf8 &type);
private:
    MjvmHeap(void) = delete;
    MjvmHeap(const MjvmHeap &) = delete;
    void operator=(const MjvmHeap &) = delete;
};

#endif /* __MJVM_HEAP_H */
