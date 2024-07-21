
#ifndef __MJVM_H
#define __MJVM_H

#include <stdint.h>
#include "mjvm_execution.h"
#include "mjvm_out_of_memory.h"

class MjvmLoadFileError {
public:
    const char *getFileName(void) const;
private:
    MjvmLoadFileError(void) = delete;
    MjvmLoadFileError(const MjvmLoadFileError &) = delete;
    void operator=(const MjvmLoadFileError &) = delete;
};

class ExecutionNode : public MjvmExecution {
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

    static void destroy(const MjvmExecution &execution);

    static MjvmExecution &newExecution(void);
    static MjvmExecution &newExecution(uint32_t stackSize);

    static void garbageCollection(void);
};

#endif /* __MJVM_H */
