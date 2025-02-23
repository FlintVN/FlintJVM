
#ifndef __FLINT_H
#define __FLINT_H

#include <stdint.h>
#include "flint_execution.h"
#include "flint_java_class.h"
#include "flint_java_string.h"
#include "flint_java_throwable.h"
#include "flint_java_thread.h"
#include "flint_class_loader.h"
#include "flint_array_object.h"
#include "flint_fields_data.h"
#include "flint_out_of_memory.h"
#include "flint_load_file_error.h"
#include "flint_find_native_error.h"
#include "flint_system_api.h"
#include "flint_java_boolean.h"
#include "flint_java_byte.h"
#include "flint_java_character.h"
#include "flint_java_short.h"
#include "flint_java_integer.h"
#include "flint_java_float.h"
#include "flint_java_long.h"
#include "flint_java_double.h"

class FlintExecutionNode : public FlintExecution {
public:
    FlintExecutionNode *prev;
    FlintExecutionNode *next;

    FlintExecutionNode(Flint &flint, FlintJavaThread *onwerThread);
    FlintExecutionNode(Flint &flint, FlintJavaThread *onwerThread, uint32_t stackSize);
private:
    FlintExecutionNode(void) = delete;
    FlintExecutionNode(const FlintExecutionNode &) = delete;
    void operator=(const FlintExecutionNode &) = delete;
};

class FlintConstUtf8Node {
public:
    FlintConstUtf8Node *next;
    FlintConstUtf8 value;
private:
    FlintConstUtf8Node(void) = delete;
    FlintConstUtf8Node(const FlintConstUtf8Node &) = delete;
    void operator=(const FlintConstUtf8Node &) = delete;
};

class Flint {
private:
    static FlintAPI::Thread::LockHandle *flintLockHandle;
    static Flint flintInstance;
    FlintDebugger *dbg;
    FlintExecutionNode *executionList;
    ClassData *classDataList;
    FlintJavaObject *objectList;
    FlintConstClass *constClassList;
    FlintConstString *constStringList;
    FlintConstUtf8Node *constUtf8List;
    FlintObjectArray *classArray0;
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

    FlintDebugger *getDebugger(void) const;
    void setDebugger(FlintDebugger *dbg);

    void print(const char *text, uint32_t length, uint8_t coder);

    FlintExecution &newExecution(FlintJavaThread *onwerThread = 0);
    FlintExecution &newExecution(FlintJavaThread *onwerThread, uint32_t stackSize);
    FlintExecution *getExcutionByThread(FlintJavaThread &thread) const;
    void freeExecution(FlintExecution &execution);

    FlintJavaObject &newObject(uint32_t size, const FlintConstUtf8 &type, uint8_t dimensions = 0);
    FlintJavaObject &newObject(const FlintConstUtf8 &type);
    FlintInt8Array &newBooleanArray(uint32_t length);
    FlintInt8Array &newByteArray(uint32_t length);
    FlintInt16Array &newCharArray(uint32_t length);
    FlintInt16Array &newShortArray(uint32_t length);
    FlintInt32Array &newIntegerArray(uint32_t length);
    FlintFloatArray &newFloatArray(uint32_t length);
    FlintInt64Array &newLongArray(uint32_t length);
    FlintDoubleArray &newDoubleArray(uint32_t length);
    FlintObjectArray &newObjectArray(const FlintConstUtf8 &type, uint32_t length);

    FlintJavaObject &newMultiArray(const FlintConstUtf8 &typeName, int32_t *counts, uint8_t startDims, uint8_t endDims = 1);

private:
    FlintJavaClass &newClass(FlintJavaString &typeName);
    FlintJavaClass &newClass(const char *typeName, uint16_t length);
public:
    FlintJavaClass &getConstClass(const char *text, uint16_t length);
    FlintJavaClass &getConstClass(FlintJavaString &str);

    FlintJavaString &newString(uint16_t length, uint8_t coder);
    FlintJavaString &newString(const char *text, uint16_t size, bool isUtf8 = false);
    FlintJavaString &newString(const char *latin1Str[], uint16_t count);
    FlintJavaString &getConstString(const FlintConstUtf8 &utf8);
    FlintJavaString &getConstString(FlintJavaString &str);

    FlintConstUtf8 &getConstUtf8(const char *text, uint16_t length);

    FlintObjectArray &getClassArray0(void);

private:
    FlintJavaThrowable &newThrowable(FlintJavaString *strObj, const FlintConstUtf8 &excpType);
public:
    FlintJavaThrowable &newException(FlintJavaString *strObj = 0);
    FlintJavaThrowable &newIOException(FlintJavaString *strObj = 0);
    FlintJavaThrowable &newErrorException(FlintJavaString *strObj = 0);
    FlintJavaThrowable &newClassCastException(FlintJavaString *strObj = 0);
    FlintJavaThrowable &newArrayStoreException(FlintJavaString *strObj = 0);
    FlintJavaThrowable &newArithmeticException(FlintJavaString *strObj = 0);
    FlintJavaThrowable &newNullPointerException(FlintJavaString *strObj = 0);
    FlintJavaThrowable &newInterruptedException(FlintJavaString *strObj = 0);
    FlintJavaThrowable &newClassNotFoundException(FlintJavaString *strObj = 0);
    FlintJavaThrowable &newIllegalArgumentException(FlintJavaString *strObj = 0);
    FlintJavaThrowable &newCloneNotSupportedException(FlintJavaString *strObj = 0);
    FlintJavaThrowable &newNegativeArraySizeException(FlintJavaString *strObj = 0);
    FlintJavaThrowable &newArrayIndexOutOfBoundsException(FlintJavaString *strObj = 0);
    FlintJavaThrowable &newUnsupportedOperationException(FlintJavaString *strObj = 0);
    FlintJavaThrowable &newUnsatisfiedLinkErrorException(FlintJavaString *strObj = 0);

    FlintJavaBoolean &newBoolean(bool value = false);
    FlintJavaByte &newByte(int8_t value = 0);
    FlintJavaCharacter &newCharacter(uint16_t value = 0);
    FlintJavaShort &newShort(int16_t value = 0);
    FlintJavaInteger &newInteger(int32_t value = 0);
    FlintJavaFloat &newFloat(float value = 0);
    FlintJavaLong &newLong(int64_t value = 0);
    FlintJavaDouble &newDouble(double value = 0);

    void clearProtectObjectNew(FlintJavaObject &obj);
    void garbageCollectionProtectObject(FlintJavaObject &obj);

    void initStaticField(ClassData &classData);
    FlintFieldsData &getStaticFields(const FlintConstUtf8 &className) const;

    FlintMethodInfo &findMethod(FlintConstMethod &constMethod);

    bool isInstanceof(FlintJavaObject *obj, const char *typeName, uint16_t length);
    bool isInstanceof(FlintJavaObject *obj, const FlintConstUtf8 &typeName);
    bool isInstanceof(const FlintConstUtf8 &typeName1, uint32_t dimensions1, const FlintConstUtf8 &typeName2, uint32_t dimensions2);

    void garbageCollection(void);

    FlintClassLoader &load(const char *className, uint16_t length);
    FlintClassLoader &load(const char *className);
    FlintClassLoader &load(const FlintConstUtf8 &className);

    void runToMain(const char *mainClass);
    void runToMain(const char *mainClass, uint32_t stackSize);

    bool isRunning(void) const;
    void terminateRequest(void);
    void terminate(void);
    void freeObject(FlintJavaObject &obj);
    void clearAllStaticFields(void);
    void freeAllObject(void);
    void freeAllExecution(void);
    void freeAllClassLoader(void);
    void freeAllConstUtf8(void);
    void freeAll(void);
    void reset(void);
};

#endif /* __FLINT_H */
