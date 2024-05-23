
#include <new>
#include <string.h>
#include "mjvm.h"
#include "mjvm_system_api.h"

static uint32_t objectCount = 0;

ExecutionNode *Mjvm::executionList = 0;

const char *OutOfMemoryError::getMessage(void) const {
    return (const char *)this;
}

const char *LoadFileError::getFileName(void) const {
    return (const char *)this;
}

ExecutionNode::ExecutionNode(void) : Execution() {
    prev = 0;
    next = 0;
}

ExecutionNode::ExecutionNode(uint32_t stackSize) : Execution(stackSize) {
    prev = 0;
    next = 0;
}

void Mjvm::lock(void) {

}

void Mjvm::unlock(void) {

}

void *Mjvm::malloc(uint32_t size) {
    void *ret = MjvmSystem_Malloc(size);
    if(ret == 0) {
        Mjvm::garbageCollection();
        ret = MjvmSystem_Malloc(size);
        if(ret == 0)
            throw (OutOfMemoryError *)"not enough memory to allocate";
    }
    objectCount++;
    return ret;
}

void *Mjvm::realloc(void *p, uint32_t size) {
    void *ret = MjvmSystem_Realloc(p, size);
    if(ret == 0) {
        Mjvm::garbageCollection();
        ret = MjvmSystem_Realloc(p, size);
        if(ret == 0)
            throw (OutOfMemoryError *)"not enough memory to allocate";
    }
    return ret;
}

void Mjvm::free(void *p) {
    objectCount--;
    MjvmSystem_Free(p);
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
    new (newNode)ExecutionNode(stackSize);
    lock();
    newNode->next = executionList;
    if(executionList)
        executionList->prev = newNode;
    executionList = newNode;
    unlock();
    return *newNode;
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
    node->~ExecutionNode();
    MjvmSystem_Free(node);
}

void Mjvm::garbageCollection(void) {
    lock();
    for(ExecutionNode *node = executionList; node != 0; node = node->next)
        node->garbageCollection();
    unlock();
}
