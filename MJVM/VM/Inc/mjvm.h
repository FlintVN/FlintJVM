
#ifndef __MJVM_H
#define __MJVM_H

#include <stdint.h>
#include "mjvm_execution.h"

class OutOfMemoryError {
public:
    const char *getMessage(void) const;
private:
    OutOfMemoryError(void) = delete;
    OutOfMemoryError(const OutOfMemoryError &) = delete;
    void operator=(const OutOfMemoryError &) = delete;
};

class LoadFileError {
public:
    const char *getFileName(void) const;
private:
    LoadFileError(void) = delete;
    LoadFileError(const LoadFileError &) = delete;
    void operator=(const LoadFileError &) = delete;
};

class ExecutionNode : public Execution {
public:
    ExecutionNode *prev;
    ExecutionNode *next;

    ExecutionNode(void);
    ExecutionNode(uint32_t stackSize);
};

class Mjvm {
private:
    static ExecutionNode *executionList;

    Mjvm(void) = delete;
    Mjvm(const Mjvm &) = delete;
    void operator=(const Mjvm &) = delete;
public:
    static void *malloc(uint32_t size);
    static void *realloc(void *p, uint32_t size);
    static void free(void *p);

    static void lock(void);
    static void unlock(void);

    static void destroy(const Execution &execution);

    static Execution &newExecution(void);
    static Execution &newExecution(uint32_t stackSize);

    static void garbageCollection(void);
};

#endif /* __MJVM_H */
