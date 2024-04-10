
#ifndef __MJVM_H
#define __MJVM_H

#include <stdint.h>
#include "mjvm_execution.h"

class Mjvm {
private:
    class ClassLoaderNode : public ClassLoader {
    public:
        uint32_t referenceCount;
        ClassLoaderNode *prev;
        ClassLoaderNode *next;

        ClassLoaderNode(const char *fileName);
        ClassLoaderNode(const ConstUtf8 &fileName);
        ClassLoaderNode(const ClassLoaderNode &) = delete;
        void operator=(const ClassLoaderNode &) = delete;
    };

    class ExecutionNode : public Execution {
    public:
        ExecutionNode *prev;
        ExecutionNode *next;

        ExecutionNode(void);
        ExecutionNode(uint32_t stackSize);
    };

    static ClassLoaderNode *classLoaderHead;
    static ExecutionNode *executionHead;

    Mjvm(void) = delete;
    Mjvm(const Mjvm &) = delete;
    void operator=(const Mjvm &) = delete;
public:
    static void *malloc(uint32_t size);
    static void free(void *p);

    static const ClassLoader &load(const char *fileName);
    static const ClassLoader &load(const ConstUtf8 &fileName);
    static void destroy(const ClassLoader &classLoader);
    static void destroy(const Execution &execution);

    static Execution &newExecution(void);
    static Execution &newExecution(uint32_t stackSize);
};

#endif /* __MJVM_H */
