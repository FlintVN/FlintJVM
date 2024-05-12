
#include <new>
#include <stdlib.h>
#include <string.h>
#include "mjvm.h"

static uint32_t objectCount = 0;

Mjvm::ExecutionNode *Mjvm::executionList = 0;

Mjvm::ExecutionNode::ExecutionNode(void) : Execution() {
    prev = 0;
    next = 0;
}

Mjvm::ExecutionNode::ExecutionNode(uint32_t stackSize) : Execution(stackSize) {
    prev = 0;
    next = 0;
}

void Mjvm::lock(void) {

}

void Mjvm::unlock(void) {

}

void *Mjvm::malloc(uint32_t size) {
    void *ret = ::malloc(size);
    if(ret == 0) {
        Mjvm::garbageCollection();
        ret = ::malloc(size);
        if(ret == 0)
            throw "not enough memory to allocate";
    }
    objectCount++;
    return ret;
}

void Mjvm::free(void *p) {
    objectCount--;
    ::free(p);
}

Execution &Mjvm::newExecution(void) {
    ExecutionNode *newNode = (ExecutionNode *)Mjvm::malloc(sizeof(ExecutionNode));
    lock();
    newNode->next = executionList;
    if(executionList)
        executionList->prev = newNode;
    executionList = newNode;
    unlock();
    return *new (newNode)ExecutionNode();
}

Execution &Mjvm::newExecution(uint32_t stackSize) {
    ExecutionNode *newNode = (ExecutionNode *)Mjvm::malloc(sizeof(ExecutionNode));
    lock();
    newNode->next = executionList;
    if(executionList)
        executionList->prev = newNode;
    executionList = newNode;
    unlock();
    return *new (newNode)ExecutionNode(stackSize);
}

void Mjvm::destroy(const Execution &execution) {
    ExecutionNode *node = (ExecutionNode *)&execution;
    lock();
    if(node->prev)
        node->prev->next = node->next;
    else
        executionList = node->next;
    if(node->next)
        node->next->prev = node->prev;
    unlock();
    node->~Execution();
    Mjvm::free(node);
}

void Mjvm::garbageCollection(void) {
    lock();
    for(ExecutionNode *node = executionList; node != 0; node = node->next)
        node->garbageCollection();
    unlock();
}
