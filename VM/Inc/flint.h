
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

    FlintExecutionNode(Flint &flint, JThread *onwerThread, uint32_t stackSize = DEFAULT_STACK_SIZE);
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
    JObject *objectList;
    FlintClassDataBinaryTree classDataTree;
    FlintClassBinaryTree constClassTree;
    FlintStringBinaryTree constStringTree;
    FlintConstUtf8BinaryTree constUtf8Tree;
    JObjectArray * volatile classArray0;
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
    void print(FlintConstUtf8 &utf8);
    void print(JString *str);
    void print(const char *text, uint32_t length, uint8_t coder);
    void println(int64_t num);
    void println(const char *text);
    void println(FlintConstUtf8 &utf8);
    void println(JString *str);

    FlintResult<FlintExecution> newExecution(JThread *onwerThread = NULL_PTR, uint32_t stackSize = DEFAULT_STACK_SIZE);
    FlintResult<FlintExecution> getExcutionByThread(JThread *thread) const;
    void freeExecution(FlintExecution *execution);

    FlintResult<JObject> newObject(uint32_t size, FlintConstUtf8 *type, uint8_t dimensions);
    FlintResult<JObject> newObject(FlintConstUtf8 *type);
    FlintResult<JInt8Array> newBooleanArray(uint32_t length);
    FlintResult<JInt8Array> newByteArray(uint32_t length);
    FlintResult<JInt16Array> newCharArray(uint32_t length);
    FlintResult<JInt16Array> newShortArray(uint32_t length);
    FlintResult<JInt32Array> newIntegerArray(uint32_t length);
    FlintResult<JFloatArray> newFloatArray(uint32_t length);
    FlintResult<JInt64Array> newLongArray(uint32_t length);
    FlintResult<JDoubleArray> newDoubleArray(uint32_t length);
    FlintResult<JObjectArray> newObjectArray(FlintConstUtf8 *type, uint32_t length);

    FlintResult<JObject> newMultiArray(FlintConstUtf8 *typeName, int32_t *counts, uint8_t startDims, uint8_t endDims);
private:
    FlintResult<JClass> newClass(JString *typeName);
    FlintResult<JClass> newClass(const char *typeName, uint16_t length);
public:
    FlintResult<JClass> getConstClass(const char *text, uint16_t length);
    FlintResult<JClass> getConstClass(JString *str);

    FlintResult<JString> newString(uint16_t length, uint8_t coder);
    FlintResult<JString> newString(const char *text);
    FlintResult<JString> newString(const char *text, uint16_t size, bool isUtf8);
    FlintResult<JString> getConstString(FlintConstUtf8 &utf8);
    FlintResult<JString> getConstString(JString *str);

    FlintResult<FlintConstUtf8> getConstUtf8(const char *text, uint16_t length);
    FlintResult<FlintConstUtf8> getTypeNameConstUtf8(const char *typeName, uint16_t length);

    FlintResult<JObjectArray> getClassArray0(void);

    FlintResult<JThrowable> newThrowable(JString *str, FlintConstUtf8 *excpType);

    FlintResult<FlintJavaBoolean> newBoolean(bool value);
    FlintResult<FlintJavaByte> newByte(int8_t value);
    FlintResult<FlintJavaCharacter> newCharacter(uint16_t value);
    FlintResult<FlintJavaShort> newShort(int16_t value);
    FlintResult<FlintJavaInteger> newInteger(int32_t value);
    FlintResult<FlintJavaFloat> newFloat(float value);
    FlintResult<FlintJavaLong> newLong(int64_t value);
    FlintResult<FlintJavaDouble> newDouble(double value);

    FlintError initStaticField(FlintClassData *classData);

    FlintResult<FlintMethodInfo> findMethod(FlintConstMethod &constMethod);
    FlintResult<FlintMethodInfo> findMethod(FlintConstUtf8 &className, FlintConstNameAndType &nameAndType);

    FlintResult<bool> isInstanceof(JObject *obj, const char *typeName, uint16_t length = 0);
    FlintResult<bool> isInstanceof(FlintConstUtf8 &typeName1, uint32_t dimensions1, FlintConstUtf8 &typeName2, uint32_t dimensions2);
private:
    static void garbageCollectionProtectObject(JObject *obj);
public:
    bool isObject(uint32_t address) const;
    void clearProtectObjectNew(JObject *obj);
    void garbageCollection(void);
private:
    FlintResult<FlintClassData> createFlintClassData(const char *className, uint16_t length);
public:
    FlintResult<FlintClassLoader> load(const char *className, uint16_t length = 0);

    FlintError runToMain(const char *mainClass, uint32_t stackSize = DEFAULT_STACK_SIZE);

    bool isRunning(void) const;
    void stopRequest(void);
    void terminateRequest(void);
    void terminate(void);
    void freeObject(JObject *obj);
    void clearAllStaticFields(void);
    void freeAllObject(void);
    void freeAllExecution(void);
    void freeAllClassLoader(void);
    void freeAllConstUtf8(void);
    void freeAll(void);
    void reset(void);
};

#endif /* __FLINT_H */
