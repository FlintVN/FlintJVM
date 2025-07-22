
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
#include "flint_const_utf8_binary_tree.h"
#include "flint_string_binary_tree.h"
#include "flint_class_binary_tree.h"
#include "flint_class_data_binary_tree.h"
#include "flint_mutex.h"
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

class Flint {
private:
    static FlintMutex flintMutex;
    static Flint flintInstance;
    static uint32_t objectCount;
    FlintDebugger *dbg;
    FlintExecutionNode * volatile executionList;
    FlintJavaObject *objectList;
    FlintClassDataBinaryTree classDataTree;
    FlintClassBinaryTree constClassTree;
    FlintStringBinaryTree constStringTree;
    FlintConstUtf8BinaryTree constUtf8Tree;
    FlintObjectArray * volatile classArray0;
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

    void print(int64_t num);
    void print(const char *text);
    void print(const FlintConstUtf8 &utf8);
    void print(FlintJavaString *str);
    void print(const char *text, uint32_t length, uint8_t coder);
    void println(int64_t num);
    void println(const char *text);
    void println(const FlintConstUtf8 &utf8);
    void println(FlintJavaString *str);

    FlintExecution &newExecution(FlintJavaThread *onwerThread = 0);
    FlintExecution &newExecution(FlintJavaThread *onwerThread, uint32_t stackSize);
    FlintExecution *getExcutionByThread(FlintJavaThread &thread) const;
    void freeExecution(FlintExecution &execution);

    FlintError newObject(uint32_t size, const FlintConstUtf8 &type, uint8_t dimensions, FlintJavaObject *&obj);
    FlintError newObject(const FlintConstUtf8 &type, FlintJavaObject *&obj);
    FlintError newBooleanArray(uint32_t length, FlintInt8Array *&array);
    FlintError newByteArray(uint32_t length, FlintInt8Array *&array);
    FlintError newCharArray(uint32_t length, FlintInt16Array *&array);
    FlintError newShortArray(uint32_t length, FlintInt16Array *&array);
    FlintError newIntegerArray(uint32_t length, FlintInt32Array *&array);
    FlintError newFloatArray(uint32_t length, FlintFloatArray *&array);
    FlintError newLongArray(uint32_t length, FlintInt64Array *&array);
    FlintError newDoubleArray(uint32_t length, FlintDoubleArray *&array);
    FlintError newObjectArray(const FlintConstUtf8 &type, uint32_t length, FlintObjectArray *&array);

    FlintError newMultiArray(const FlintConstUtf8 &typeName, int32_t *counts, uint8_t startDims, uint8_t endDims, FlintJavaObject *&array);
private:
    FlintError newClass(FlintJavaString &typeName, FlintJavaClass *&cls);
    FlintError newClass(const char *typeName, uint16_t length, FlintJavaClass *&cls);
public:
    FlintError getConstClass(const char *text, uint16_t length, FlintJavaClass *&cls);
    FlintError getConstClass(FlintJavaString &str, FlintJavaClass *&cls);

    FlintError newString(uint16_t length, uint8_t coder, FlintJavaString *&str);
    FlintError newString(const char *text, FlintJavaString *&str);
    FlintError newString(const char *text, uint16_t size, bool isUtf8, FlintJavaString *&str);
    FlintError getConstString(const FlintConstUtf8 &utf8, FlintJavaString *&str);
    FlintError getConstString(FlintJavaString &str, FlintJavaString *&strRet);

    FlintConstUtf8 &getConstUtf8(const char *text, uint16_t length);
    FlintConstUtf8 &getTypeNameConstUtf8(const char *typeName, uint16_t length);

    FlintError getClassArray0(FlintObjectArray *&obj);

    FlintError newThrowable(FlintJavaString *str, const FlintConstUtf8 &excpType, FlintJavaThrowable *&excp);

    FlintError newBoolean(bool value, FlintJavaBoolean *&obj);
    FlintError newByte(int8_t value, FlintJavaByte *&obj);
    FlintError newCharacter(uint16_t value, FlintJavaCharacter *&obj);
    FlintError newShort(int16_t value, FlintJavaShort *&obj);
    FlintError newInteger(int32_t value, FlintJavaInteger *&obj);
    FlintError newFloat(float value, FlintJavaFloat *&obj);
    FlintError newLong(int64_t value, FlintJavaLong *&obj);
    FlintError newDouble(double value, FlintJavaDouble *&obj);

    FlintError initStaticField(FlintClassData &classData);

    FlintError findMethod(FlintConstMethod &constMethod, FlintMethodInfo *&methodInfo);
    FlintError findMethod(FlintConstUtf8 &className, FlintConstNameAndType &nameAndType, FlintMethodInfo *&methodInfo);

    FlintError isInstanceof(FlintJavaObject *obj, const char *typeName, uint16_t length, FlintConstUtf8 **classError);
    FlintError isInstanceof(FlintJavaObject *obj, const FlintConstUtf8 &typeName, FlintConstUtf8 **classError);
    FlintError isInstanceof(const FlintConstUtf8 &typeName1, uint32_t dimensions1, const FlintConstUtf8 &typeName2, uint32_t dimensions2,  FlintConstUtf8 **classError);
private:
    static void garbageCollectionProtectObject(FlintJavaObject &obj);
public:
    bool isObject(uint32_t address) const;
    void clearProtectObjectNew(FlintJavaObject &obj);
    void garbageCollection(void);
private:
    FlintError createFlintClassData(Flint *flint, const char *className, uint16_t length, FlintClassData *&classData);
public:
    FlintError load(const char *className, uint16_t length, FlintClassLoader *&loader);
    FlintError load(const char *className, FlintClassLoader *&loader);
    FlintError load(const FlintConstUtf8 &className, FlintClassLoader *&loader);

    FlintError runToMain(const char *mainClass);
    FlintError runToMain(const char *mainClass, uint32_t stackSize);

    bool isRunning(void) const;
    void stopRequest(void);
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
