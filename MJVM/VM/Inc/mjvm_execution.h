
#ifndef __MJVM_EXECUTION_H
#define __MJVM_EXECUTION_H

#include "mjvm_common.h"
#include "mjvm_class.h"
#include "mjvm_string.h"
#include "mjvm_debugger.h"
#include "mjvm_throwable.h"
#include "mjvm_class_loader.h"
#include "mjvm_fields_data.h"

#if __has_include("mjvm_conf.h")
#include "mjvm_conf.h"
#endif
#include "mjvm_default_conf.h"

#define STR_AND_SIZE(str)           str, (sizeof(str) - 1)

class Execution {
private:
    const uint32_t stackLength;
    MethodInfo *method;
    const uint8_t *code;
    uint32_t pc;
    uint32_t lr;
    int32_t sp;
    int32_t startSp;
    int32_t peakSp;
    int32_t *stack;
    int32_t *locals;
    uint8_t *stackType;
    ClassData *classDataList;
    MjvmObject *objectList;
    MjvmConstClass *constClassList;
    MjvmConstString *constStringList;
    uint32_t objectSizeToGc;

    typedef enum : uint8_t {
        STACK_TYPE_NON_OBJECT = 0,
        STACK_TYPE_OBJECT = 1,
    } StackType;

    typedef struct {
        StackType type;
        int32_t value;
    } StackValue;
protected:
    Execution(void);
    Execution(uint32_t stackSize);
    Execution(const Execution &) = delete;
    void operator=(const Execution &) = delete;

    ~Execution(void);
public:
    MjvmObject *newObject(uint32_t size, ConstUtf8 &type, uint8_t dimensions = 0);

    MjvmObject *newMultiArray(ConstUtf8 &typeName, uint8_t dimensions, int32_t *counts);

    MjvmClass *newClass(MjvmString &typeName);
    MjvmClass *newClass(const char *typeName, uint16_t length);
    MjvmClass *getConstClass(const char *text, uint16_t length);
    MjvmClass *getConstClass(MjvmString &str);

    MjvmString *newString(uint16_t length, uint8_t coder);
    MjvmString *newString(const char *text, uint16_t size, bool isUtf8 = false);
    MjvmString *newString(const char *latin1Str[], uint16_t count);
    MjvmString *getConstString(ConstUtf8 &utf8);
    MjvmString *getConstString(MjvmString &str);

    MjvmThrowable *newThrowable(MjvmString *strObj, ConstUtf8 &excpType);
    MjvmThrowable *newArrayStoreException(MjvmString *strObj);
    MjvmThrowable *newArithmeticException(MjvmString *strObj);
    MjvmThrowable *newNullPointerException(MjvmString *strObj);
    MjvmThrowable *newClassNotFoundException(MjvmString *strObj);
    MjvmThrowable *newCloneNotSupportedException(MjvmString *strObj);
    MjvmThrowable *newNegativeArraySizeException(MjvmString *strObj);
    MjvmThrowable *newArrayIndexOutOfBoundsException(MjvmString *strObj);
private:
    void freeAllObject(void);
    void clearProtectObjectNew(MjvmObject *obj);
    void garbageCollectionProtectObject(MjvmObject *obj);

    void initStaticField(ClassData &classData);
    FieldsData &getStaticFields(ConstUtf8 &className) const;

    StackType getStackType(uint32_t index);
    StackValue getStackValue(uint32_t index);
    void setStackValue(uint32_t index, StackValue &value);

    void stackPush(StackValue &value);
public:
    void stackPushInt32(int32_t value);
    void stackPushInt64(int64_t value);
    void stackPushFloat(float value);
    void stackPushDouble(double value);
    void stackPushObject(MjvmObject *obj);

    int32_t stackPopInt32(void);
    int64_t stackPopInt64(void);
    float stackPopFloat(void);
    double stackPopDouble(void);
    MjvmObject *stackPopObject(void);

    void getStackTrace(uint32_t index, StackTrace *stackTrace) const;
private:
    void stackInitExitPoint(uint32_t exitPc);
    void stackRestoreContext(void);

    void initNewContext(MethodInfo &methodInfo, uint16_t argc = 0);

    MethodInfo &findMethod(ConstMethod &constMethod);
    bool invoke(MethodInfo &methodInfo, uint8_t argc);
    bool invokeStatic(ConstMethod &constMethod);
    bool invokeSpecial(ConstMethod &constMethod);
    bool invokeVirtual(ConstMethod &constMethod);
    bool invokeInterface(ConstInterfaceMethod &interfaceMethod, uint8_t argc);

    void run(MethodInfo &method, Debugger *dbg);

    friend class Mjvm;
public:
    bool isInstanceof(MjvmObject *obj, const char *typeName, uint16_t length);

    void garbageCollection(void);

    ClassLoader &load(const char *className, uint16_t length);
    ClassLoader &load(const char *className);
    ClassLoader &load(ConstUtf8 &className);

    void runToMain(const char *mainClass, Debugger *dbg = 0);
};

#endif /* __MJVM_EXECUTION_H */
