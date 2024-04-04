
#include <new>
#include <stdlib.h>
#include "mjvm_heap.h"

static uint32_t objectCount = 0;

MjvmObject::MjvmObject(uint32_t size, const ConstUtf8 &type, uint8_t dimensions) : size(size), type(type), dimensions(dimensions) {

}

uint8_t MjvmObject::parseTypeSize(void) const {
    switch(this->type.getText()[0]) {
        case 'Z':
        case 'B':
            return 1;
        case 'C':
        case 'S':
            return 2;
        case 'J':
        case 'D':
            return 8;
        default:
            return 4;
    }
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

MjvmObject *MjvmHeap::newObject(uint32_t size, const ConstUtf8 &type, uint8_t dimensions) {
    MjvmObject *ret = (MjvmObject *)MjvmHeap::malloc(sizeof(MjvmObject) + size);
    new (ret)MjvmObject(size, type, dimensions);
    return ret;
}
