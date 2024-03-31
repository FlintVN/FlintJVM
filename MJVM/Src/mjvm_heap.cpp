
#include <new>
#include <stdlib.h>
#include "mjvm_heap.h"

static uint32_t objectCount = 0;

MjvmObject::MjvmObject(uint32_t size, const ConstUtf8 &type) : size(size), type(type) {

}

void *MjvmHeap::malloc(uint32_t size) {
    void *ret = ::malloc(size);
    if(ret == 0)
        throw "not enough memory to allocate";
    objectCount++;
    return ret;
}

void MjvmHeap::free(void *p) {
    objectCount--;
    ::free(p);
}

MjvmObject *MjvmHeap::createNew(uint32_t size, const ConstUtf8 &type) {
    MjvmObject *ret = (MjvmObject *)MjvmHeap::malloc(sizeof(MjvmObject) + size);
    new (ret)MjvmObject(size, type);
    return ret;
}
