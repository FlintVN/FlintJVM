
#include <new>
#include <stdlib.h>
#include <string.h>
#include "mjvm.h"

static uint32_t objectCount = 0;

Mjvm::ClassLoaderNode *Mjvm::classLoaderHead = 0;
Mjvm::ExecutionNode *Mjvm::executionHead = 0;

Mjvm::ClassLoaderNode::ClassLoaderNode(const char *fileName) : ClassLoader(fileName) {
    referenceCount = 0;
    prev = 0;
    next = 0;
}

Mjvm::ClassLoaderNode::ClassLoaderNode(const ConstUtf8 &fileName) : ClassLoader(fileName) {
    referenceCount = 0;
    prev = 0;
    next = 0;
}

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

const ClassLoader &Mjvm::load(const char *fileName) {
    uint32_t len = strlen(fileName);
    lock();
    for(ClassLoaderNode *node = classLoaderHead; node != 0; node = node->next) {
        const ConstUtf8 &name = node->getThisClass();
        if(len == name.length && strncmp(fileName, name.getText(), len)) {
            node->referenceCount++;
            unlock();
            return *node;
        }
    }
    ClassLoaderNode *newLoader = (ClassLoaderNode *)Mjvm::malloc(sizeof(ClassLoaderNode));
    new (newLoader)ClassLoaderNode(fileName);
    newLoader->next = classLoaderHead;
    if(classLoaderHead)
        classLoaderHead->prev = newLoader;
    newLoader->referenceCount++;
    classLoaderHead = newLoader;
    unlock();
    return *newLoader;
}

const ClassLoader &Mjvm::load(const ConstUtf8 &fileName) {
    lock();
    for(ClassLoaderNode *node = classLoaderHead; node != 0; node = node->next) {
        if(fileName == node->getThisClass()) {
            node->referenceCount++;
            unlock();
            return *node;
        }
    }
    ClassLoaderNode *newLoader = (ClassLoaderNode *)Mjvm::malloc(sizeof(ClassLoaderNode));
    new (newLoader)ClassLoaderNode(fileName);
    newLoader->next = classLoaderHead;
    if(classLoaderHead)
        classLoaderHead->prev = newLoader;
    newLoader->referenceCount++;
    classLoaderHead = newLoader;
    unlock();
    return *newLoader;
}

void Mjvm::destroy(const ClassLoader &classLoader) {
    ClassLoaderNode *node = (ClassLoaderNode *)&classLoader;
    lock();
    if(--node->referenceCount == 0) {
        if(node->prev)
            node->prev->next = node->next;
        else
            classLoaderHead = node->next;
        if(node->next)
            node->next->prev = node->prev;
        node->~ClassLoader();
        Mjvm::free(node);
    }
    unlock();
}

Execution &Mjvm::newExecution(void) {
    ExecutionNode *newNode = (ExecutionNode *)Mjvm::malloc(sizeof(ExecutionNode));
    lock();
    newNode->next = executionHead;
    if(executionHead)
        executionHead->prev = newNode;
    executionHead = newNode;
    unlock();
    return *new (newNode)ExecutionNode();
}

Execution &Mjvm::newExecution(uint32_t stackSize) {
    ExecutionNode *newNode = (ExecutionNode *)Mjvm::malloc(sizeof(ExecutionNode));
    lock();
    newNode->next = executionHead;
    if(executionHead)
        executionHead->prev = newNode;
    executionHead = newNode;
    unlock();
    return *new (newNode)ExecutionNode(stackSize);
}

void Mjvm::destroy(const Execution &execution) {
    ExecutionNode *node = (ExecutionNode *)&execution;
    lock();
    if(node->prev)
        node->prev->next = node->next;
    else
        executionHead = node->next;
    if(node->next)
        node->next->prev = node->prev;
    unlock();
    node->~Execution();
    Mjvm::free(node);
}

void Mjvm::garbageCollection(void) {
    lock();
    for(ExecutionNode *node = executionHead; node != 0; node = node->next)
        node->garbageCollection();
    unlock();
}
