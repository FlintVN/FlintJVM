
#ifndef __MJVM_H
#define __MJVM_H

#include <stdint.h>
#include "mjvm_execution.h"
#include "mjvm_class.h"
#include "mjvm_string.h"
#include "mjvm_throwable.h"
#include "mjvm_class_loader.h"
#include "mjvm_fields_data.h"
#include "mjvm_out_of_memory.h"
#include "mjvm_load_file_error.h"

class MjvmExecutionNode : public MjvmExecution {
public:
    MjvmExecutionNode *prev;
    MjvmExecutionNode *next;

    MjvmExecutionNode(Mjvm &mjvm);
    MjvmExecutionNode(Mjvm &mjvm, uint32_t stackSize);
private:
    MjvmExecutionNode(void) = delete;
    MjvmExecutionNode(const MjvmExecutionNode &) = delete;
    void operator=(const MjvmExecutionNode &) = delete;
};

class Mjvm {
private:
    static Mjvm mjvmInstance;
    MjvmDebugger *dbg;
    MjvmExecutionNode *executionList;
    ClassData *classDataList;
    MjvmObject *objectList;
    MjvmConstClass *constClassList;
    MjvmConstString *constStringList;
    uint32_t objectSizeToGc;

    Mjvm(void);
    Mjvm(const Mjvm &) = delete;
    void operator=(const Mjvm &) = delete;
public:
    static void *malloc(uint32_t size);
    static void *realloc(void *p, uint32_t size);
    static void free(void *p);

    static void lock(void);
    static void unlock(void);

    static Mjvm &getInstance(void);
public:
    MjvmDebugger *getDebugger(void) const;
    void setDebugger(MjvmDebugger *dbg);

    MjvmExecution &newExecution(void);
    MjvmExecution &newExecution(uint32_t stackSize);

    MjvmObject *newObject(uint32_t size, MjvmConstUtf8 &type, uint8_t dimensions = 0);

    MjvmObject *newMultiArray(MjvmConstUtf8 &typeName, uint8_t dimensions, int32_t *counts);

    MjvmClass *newClass(MjvmString &typeName);
    MjvmClass *newClass(const char *typeName, uint16_t length);
    MjvmClass *getConstClass(const char *text, uint16_t length);
    MjvmClass *getConstClass(MjvmString &str);

    MjvmString *newString(uint16_t length, uint8_t coder);
    MjvmString *newString(const char *text, uint16_t size, bool isUtf8 = false);
    MjvmString *newString(const char *latin1Str[], uint16_t count);
    MjvmString *getConstString(MjvmConstUtf8 &utf8);
    MjvmString *getConstString(MjvmString &str);

    MjvmThrowable *newThrowable(MjvmString *strObj, MjvmConstUtf8 &excpType);
    MjvmThrowable *newArrayStoreException(MjvmString *strObj);
    MjvmThrowable *newArithmeticException(MjvmString *strObj);
    MjvmThrowable *newNullPointerException(MjvmString *strObj);
    MjvmThrowable *newClassNotFoundException(MjvmString *strObj);
    MjvmThrowable *newCloneNotSupportedException(MjvmString *strObj);
    MjvmThrowable *newNegativeArraySizeException(MjvmString *strObj);
    MjvmThrowable *newArrayIndexOutOfBoundsException(MjvmString *strObj);
    MjvmThrowable *newUnsupportedOperationException(MjvmString *strObj);

    void freeAllObject(void);
    void clearProtectObjectNew(MjvmObject *obj);
    void garbageCollectionProtectObject(MjvmObject *obj);

    void initStaticField(ClassData &classData);
    MjvmFieldsData &getStaticFields(MjvmConstUtf8 &className) const;

    MjvmMethodInfo &findMethod(MjvmConstMethod &constMethod);

    bool isInstanceof(MjvmObject *obj, const char *typeName, uint16_t length);

    void garbageCollection(void);

    MjvmClassLoader &load(const char *className, uint16_t length);
    MjvmClassLoader &load(const char *className);
    MjvmClassLoader &load(MjvmConstUtf8 &className);

    void terminateAll(void);
};

#endif /* __MJVM_H */
