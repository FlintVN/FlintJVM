
#ifndef __FLINT_H
#define __FLINT_H

#include <stdint.h>
#include "flint_execution.h"
#include "flint_class.h"
#include "flint_string.h"
#include "flint_throwable.h"
#include "flint_class_loader.h"
#include "flint_fields_data.h"
#include "flint_out_of_memory.h"
#include "flint_load_file_error.h"

class FlintExecutionNode : public FlintExecution {
public:
    FlintExecutionNode *prev;
    FlintExecutionNode *next;

    FlintExecutionNode(Flint &flint);
    FlintExecutionNode(Flint &flint, uint32_t stackSize);
private:
    FlintExecutionNode(void) = delete;
    FlintExecutionNode(const FlintExecutionNode &) = delete;
    void operator=(const FlintExecutionNode &) = delete;
};

class Flint {
private:
    static Flint flintInstance;
    FlintDebugger *dbg;
    FlintExecutionNode *executionList;
    ClassData *classDataList;
    FlintObject *objectList;
    FlintConstClass *constClassList;
    FlintConstString *constStringList;
    uint32_t objectSizeToGc;

    Flint(void);
    Flint(const Flint &) = delete;
    void operator=(const Flint &) = delete;
public:
    static void *malloc(uint32_t size);
    static void *realloc(void *p, uint32_t size);
    static void free(void *p);

    static void lock(void);
    static void unlock(void);

    static Flint &getInstance(void);

    FlintExecution &newExecution(void);
    FlintExecution &newExecution(uint32_t stackSize);

    void freeAllObject(void);
    void freeAllExecution(void);
    void freeAllClassLoader(void);
public:
    FlintDebugger *getDebugger(void) const;
    void setDebugger(FlintDebugger *dbg);

    FlintObject *newObject(uint32_t size, FlintConstUtf8 &type, uint8_t dimensions = 0);

    FlintObject *newMultiArray(FlintConstUtf8 &typeName, uint8_t dimensions, int32_t *counts);

    FlintClass *newClass(FlintString &typeName);
    FlintClass *newClass(const char *typeName, uint16_t length);
    FlintClass *getConstClass(const char *text, uint16_t length);
    FlintClass *getConstClass(FlintString &str);

    FlintString *newString(uint16_t length, uint8_t coder);
    FlintString *newString(const char *text, uint16_t size, bool isUtf8 = false);
    FlintString *newString(const char *latin1Str[], uint16_t count);
    FlintString *getConstString(FlintConstUtf8 &utf8);
    FlintString *getConstString(FlintString &str);

    FlintThrowable *newThrowable(FlintString *strObj, FlintConstUtf8 &excpType);
    FlintThrowable *newArrayStoreException(FlintString *strObj);
    FlintThrowable *newArithmeticException(FlintString *strObj);
    FlintThrowable *newNullPointerException(FlintString *strObj);
    FlintThrowable *newClassNotFoundException(FlintString *strObj);
    FlintThrowable *newCloneNotSupportedException(FlintString *strObj);
    FlintThrowable *newNegativeArraySizeException(FlintString *strObj);
    FlintThrowable *newArrayIndexOutOfBoundsException(FlintString *strObj);
    FlintThrowable *newUnsupportedOperationException(FlintString *strObj);

    void clearProtectObjectNew(FlintObject *obj);
    void garbageCollectionProtectObject(FlintObject *obj);

    void initStaticField(ClassData &classData);
    FlintFieldsData &getStaticFields(FlintConstUtf8 &className) const;

    FlintMethodInfo &findMethod(FlintConstMethod &constMethod);

    bool isInstanceof(FlintObject *obj, const char *typeName, uint16_t length);

    void garbageCollection(void);

    FlintClassLoader &load(const char *className, uint16_t length);
    FlintClassLoader &load(const char *className);
    FlintClassLoader &load(FlintConstUtf8 &className);

    void runToMain(const char *mainClass);
    void runToMain(const char *mainClass, uint32_t stackSize);

    bool isRunning(void) const;
    void terminateRequest(void);
    void terminateAll(void);
};

#endif /* __FLINT_H */
