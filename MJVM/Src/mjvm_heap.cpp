
#include <new>
#include <stdlib.h>
#include "mjvm_heap.h"

static uint32_t objectCount = 0;

MjvmObject::MjvmObject(uint32_t size, const ConstUtf8 &type, uint8_t dimensions) : size(size), type(type), dimensions(dimensions) {
    prot = 0;
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

void MjvmObject::setProtected(void) {
    prot = 1;
}

bool MjvmObject::isProtected(void) const {
    return prot;
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

void MjvmHeap::addToObjectList(MjvmObjectNode *objNode) {
    objNode->prev = 0;
    objNode->next = objectList;
    if(objectList)
        objectList->prev = objNode;
    objectList = objNode;
}

void MjvmHeap::removeFromObjectList(MjvmObjectNode *objNode) {
    if(objNode->prev)
        objNode->prev->next = objNode->next;
    else
        objectList = objNode->next;
    if(objNode->next)
        objNode->next->prev = objNode->prev;
}

void MjvmHeap::freeObject(MjvmObjectNode *objNode) {
    removeFromObjectList(objNode);
    MjvmHeap::free(objNode);
}

void MjvmHeap::freeObject(MjvmObject *obj) {
    freeObject((MjvmObjectNode *)((uint8_t *)obj) - sizeof(MjvmObjectNode));
}

void MjvmHeap::freeAllObject(uint32_t id) {
    for(MjvmObjectNode *node = objectList; node != 0;) {
        MjvmObjectNode *next = node->next;
        if(node->id == id)
            freeObject(node);
        node = next;
    }
}

void MjvmHeap::garbageCollection(uint32_t id) {
    for(MjvmObjectNode *node = objectList; node != 0;) {
        MjvmObjectNode *next = node->next;
        if(node->id == id) {
            MjvmObject *obj = node->getMjvmObject();
            if(!obj->prot)
                freeObject(node);
            else
                obj->prot = 0;
        }
        node = next;
    }
}

MjvmObject *MjvmHeap::newObject(uint32_t id, uint32_t size, const ConstUtf8 &type, uint8_t dimensions) {
    MjvmObjectNode *newNode = (MjvmObjectNode *)MjvmHeap::malloc(sizeof(MjvmObjectNode) + sizeof(MjvmObject) + size);
    newNode->id = id;
    addToObjectList(newNode);
    MjvmObject *ret = newNode->getMjvmObject();
    new (ret)MjvmObject(size, type, dimensions);
    return ret;
}

MjvmObject *MjvmHeap::MjvmObjectNode::getMjvmObject(void) const {
    return (MjvmObject *)data;
}

MjvmHeap::MjvmObjectNode *MjvmHeap::objectList = 0;
