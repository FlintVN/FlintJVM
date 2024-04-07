
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

void MjvmHeap::freeObject(MjvmObjectNode *objNode) {
    if(objNode->prev)
        objNode->prev->next = objNode->next;
    else
        objectList = objNode->next;
    if(objNode->next)
        objNode->next->prev = objNode->prev;
    ::free(objNode);
}

void MjvmHeap::freeObject(MjvmObject *obj) {
    freeObject((MjvmObjectNode *)((uint8_t *)obj) - sizeof(MjvmObjectNode));
}

void MjvmHeap::freeAllObject(uint32_t id) {
    for(MjvmObjectNode *node = objectList; node != 0;) {
        if(node->id == id) {
            MjvmObjectNode *next = node->next;
            freeObject(node);
            node = next;
            continue;
        }
        node = node->next;
    }
}

MjvmObject *MjvmHeap::newObject(uint32_t id, uint32_t size, const ConstUtf8 &type, uint8_t dimensions) {
    MjvmObjectNode *newNode = (MjvmObjectNode *)MjvmHeap::malloc(sizeof(MjvmObjectNode) + sizeof(MjvmObject) + size);
    newNode->id = id;
    newNode->prev = 0;
    newNode->next = objectList;
    if(objectList)
        objectList->prev = newNode;
    objectList = newNode;
    MjvmObject *ret = newNode->getMjvmObject();
    new (ret)MjvmObject(size, type, dimensions);
    return ret;
}

MjvmObject *MjvmHeap::MjvmObjectNode::getMjvmObject(void) const {
    return (MjvmObject *)data;
}

MjvmHeap::MjvmObjectNode *MjvmHeap::objectList = 0;
