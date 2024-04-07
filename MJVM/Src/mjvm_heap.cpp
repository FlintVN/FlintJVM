
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

void MjvmHeap::freeObject(MjvmObject *obj) {
    free(((uint8_t *)obj) - sizeof(MemInfoNode));
}

void MjvmHeap::freeAllObject(uint32_t id) {
    MemInfoNode *prev = 0;
    for(MemInfoNode *node = objectList; node != 0;) {
        if(node->id == id) {
            MemInfoNode *next = node->next;
            if(prev == 0)
                objectList = next;
            else
                prev->next = next;
            free(node);
            node = next;
            continue;
        }
        prev = node;
        node = node->next;
    }
}

MjvmObject *MjvmHeap::newObject(uint32_t id, uint32_t size, const ConstUtf8 &type, uint8_t dimensions) {
    MemInfoNode *newNode = (MemInfoNode *)MjvmHeap::malloc(sizeof(MemInfoNode) + sizeof(MjvmObject) + size);
    newNode->id = id;
    newNode->next = objectList;
    objectList = newNode;
    MjvmObject *ret = (MjvmObject *)(((uint8_t *)newNode) + sizeof(MemInfoNode));
    new (ret)MjvmObject(size, type, dimensions);
    return ret;
}

MjvmHeap::MemInfoNode *MjvmHeap::objectList = 0;
