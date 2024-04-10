
#include <new>
#include <stdlib.h>
#include <string.h>
#include "mjvm.h"

static uint32_t objectCount = 0;

ClassLoader *Mjvm::classLoaderHead = 0;

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
    for(ClassLoader *loader = classLoaderHead; loader != 0; loader = loader->next) {
        const ConstUtf8 &name = loader->getThisClass();
        if(len == name.length && strncmp(fileName, name.getText(), len)) {
            loader->referenceCount++;
            return *loader;
        }
    }
    ClassLoader *newLoader = (ClassLoader *)Mjvm::malloc(sizeof(ClassLoader));
    new (newLoader)ClassLoader(fileName);
    newLoader->next = classLoaderHead;
    newLoader->referenceCount++;
    classLoaderHead = newLoader;
    return *newLoader;
}

const ClassLoader &Mjvm::load(const ConstUtf8 &fileName) {
    for(ClassLoader *loader = classLoaderHead; loader != 0; loader = loader->next) {
        if(fileName == loader->getThisClass()) {
            loader->referenceCount++;
            return *loader;
        }
    }
    ClassLoader *newLoader = (ClassLoader *)Mjvm::malloc(sizeof(ClassLoader));
    new (newLoader)ClassLoader(fileName);
    newLoader->next = classLoaderHead;
    newLoader->referenceCount++;
    classLoaderHead = newLoader;
    return *newLoader;
}

void Mjvm::destroy(const ClassLoader &classLoader) {
    ClassLoader *prev = 0;
    for(ClassLoader *loader = classLoaderHead; loader != 0; loader = loader->next) {
        if(loader == &classLoader) {
            if(--loader->referenceCount == 0) {
                if(prev == 0)
                    classLoaderHead = loader->next;
                else
                    prev->next = loader->next;
                loader->~ClassLoader();
                Mjvm::free(loader);
            }
            return;
        }
        prev = loader;
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
