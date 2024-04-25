
#include <new>
#include <stdlib.h>
#include <string.h>
#include "mjvm.h"

static uint32_t objectCount = 0;

Mjvm::ClassLoaderNode *Mjvm::classLoaderList = 0;
Mjvm::ExecutionNode *Mjvm::executionList = 0;

Mjvm::ClassLoaderNode::ClassLoaderNode(const char *fileName) : ClassLoader(fileName) {
    referenceCount = 0;
    prev = 0;
    next = 0;
}

Mjvm::ClassLoaderNode::ClassLoaderNode(const char *fileName, uint16_t length) : ClassLoader(fileName, length) {
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

Mjvm::ClassLoaderNode *Mjvm::newLoaderNode(const char *fileName, uint16_t length) {
    lock();
    ClassLoaderNode *newLoader = (ClassLoaderNode *)Mjvm::malloc(sizeof(ClassLoaderNode));
    new (newLoader)ClassLoaderNode(fileName, length);
    newLoader->next = classLoaderList;
    if(classLoaderList)
        classLoaderList->prev = newLoader;
    newLoader->referenceCount++;
    classLoaderList = newLoader;
    unlock();
    return newLoader;
}

Mjvm::ClassLoaderNode *Mjvm::findClassLoaderNode(const char *fileName, uint16_t length) {
    lock();
    for(ClassLoaderNode *node = classLoaderList; node != 0; node = node->next) {
        const ConstUtf8 &name = node->getThisClass();
        if(length == name.length && strncmp(fileName, name.text, length) == 0) {
            unlock();
            return node;
        }
    }
    unlock();
    return 0;
}

const ClassLoader &Mjvm::load(const char *fileName) {
    uint32_t len = strlen(fileName);
    return Mjvm::load(fileName, len);
}

const ClassLoader &Mjvm::load(const char *fileName, uint16_t length) {
    ClassLoaderNode *node = findClassLoaderNode(fileName, length);
    if(node) {
        lock();
        node->referenceCount++;
        unlock();
        return *node;
    }
    return *newLoaderNode(fileName, length);
}

const ClassLoader &Mjvm::load(const ConstUtf8 &fileName) {
    return load(fileName.text, fileName.length);
}

void Mjvm::destroy(const ClassLoader &classLoader) {
    ClassLoaderNode *node = (ClassLoaderNode *)&classLoader;
    lock();
    if(--node->referenceCount == 0) {
        if(node->prev)
            node->prev->next = node->next;
        else
            classLoaderList = node->next;
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
