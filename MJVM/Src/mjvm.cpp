
#include <new>
#include <stdlib.h>
#include <string.h>
#include "mjvm.h"

static uint32_t objectCount = 0;

Mjvm::ClassLoaderNode *Mjvm::classLoaderHead = 0;

Mjvm::ClassLoaderNode::ClassLoaderNode(const char *fileName) : ClassLoader(fileName) {
    referenceCount = 0;
    next = 0;
}

Mjvm::ClassLoaderNode::ClassLoaderNode(const ConstUtf8 &fileName) : ClassLoader(fileName) {
    referenceCount = 0;
    next = 0;
}

void *Mjvm::malloc(uint32_t size) {
    void *ret = ::malloc(size);
    if(ret == 0)
        throw "not enough memory to allocate";
    objectCount++;
    return ret;
}

void Mjvm::free(void *p) {
    objectCount--;
    ::free(p);
}

const ClassLoader &Mjvm::load(const char *fileName) {
    uint32_t len = strlen(fileName);
    for(ClassLoaderNode *node = classLoaderHead; node != 0; node = node->next) {
        const ConstUtf8 &name = node->getThisClass();
        if(len == name.length && strncmp(fileName, name.getText(), len)) {
            node->referenceCount++;
            return *node;
        }
    }
    ClassLoaderNode *newLoader = (ClassLoaderNode *)Mjvm::malloc(sizeof(ClassLoaderNode));
    new (newLoader)ClassLoaderNode(fileName);
    newLoader->next = classLoaderHead;
    newLoader->referenceCount++;
    classLoaderHead = newLoader;
    return *newLoader;
}

const ClassLoader &Mjvm::load(const ConstUtf8 &fileName) {
    for(ClassLoaderNode *node = classLoaderHead; node != 0; node = node->next) {
        if(fileName == node->getThisClass()) {
            node->referenceCount++;
            return *node;
        }
    }
    ClassLoaderNode *newLoader = (ClassLoaderNode *)Mjvm::malloc(sizeof(ClassLoaderNode));
    new (newLoader)ClassLoaderNode(fileName);
    newLoader->next = classLoaderHead;
    newLoader->referenceCount++;
    classLoaderHead = newLoader;
    return *newLoader;
}

void Mjvm::destroy(const ClassLoader &classLoader) {
    ClassLoaderNode *prev = 0;
    for(ClassLoaderNode *node = classLoaderHead; node != 0; node = node->next) {
        if(node == &classLoader) {
            if(--node->referenceCount == 0) {
                if(prev == 0)
                    classLoaderHead = node->next;
                else
                    prev->next = node->next;
                node->~ClassLoader();
                Mjvm::free(node);
            }
            return;
        }
        prev = node;
    }
    throw "the class is not loaded";
}

Execution &Mjvm::newExecution(void) {
    Execution *execution = (Execution *)Mjvm::malloc(sizeof(Execution));
    return *new (execution)Execution();
}

Execution &Mjvm::newExecution(uint32_t stackSize) {
    Execution *execution = (Execution *)Mjvm::malloc(sizeof(Execution));
    return *new (execution)Execution(stackSize);
}

void Mjvm::destroy(const Execution &execution) {
    execution.~Execution();
    Mjvm::free((void *)&execution);
}
