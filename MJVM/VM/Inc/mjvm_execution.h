
#ifndef __MJVM_EXECUTION_H
#define __MJVM_EXECUTION_H

#include "mjvm_common.h"
#include "mjvm_class.h"
#include "mjvm_string.h"
#include "mjvm_debugger.h"
#include "mjvm_throwable.h"
#include "mjvm_class_loader.h"
#include "mjvm_fields_data.h"

#define STR_AND_SIZE(str)           str, (sizeof(str) - 1)

class MjvmExecution {
private:
    const uint32_t stackLength;
    MjvmMethodInfo *method;
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
    } MjvmStackType;

    typedef struct {
        MjvmStackType type;
        int32_t value;
    } MjvmStackValue;
protected:
    MjvmExecution(void);
    MjvmExecution(uint32_t stackSize);
    MjvmExecution(const MjvmExecution &) = delete;
    void operator=(const MjvmExecution &) = delete;

    ~MjvmExecution(void);
public:
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
private:
    void freeAllObject(void);
    void clearProtectObjectNew(MjvmObject *obj);
    void garbageCollectionProtectObject(MjvmObject *obj);

    void initStaticField(ClassData &classData);
    MjvmFieldsData &getStaticFields(MjvmConstUtf8 &className) const;

    MjvmStackType getStackType(uint32_t index);
    MjvmStackValue getStackValue(uint32_t index);
    void setStackValue(uint32_t index, MjvmStackValue &value);

    void stackPush(MjvmStackValue &value);
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
private:
    void stackInitExitPoint(uint32_t exitPc);
    void stackRestoreContext(void);

    void initNewContext(MjvmMethodInfo &methodInfo, uint16_t argc = 0);

    MjvmMethodInfo &findMethod(MjvmConstMethod &constMethod);
    bool invoke(MjvmMethodInfo &methodInfo, uint8_t argc);
    bool invokeStatic(MjvmConstMethod &constMethod);
    bool invokeSpecial(MjvmConstMethod &constMethod);
    bool invokeVirtual(MjvmConstMethod &constMethod);
    bool invokeInterface(MjvmConstInterfaceMethod &interfaceMethod, uint8_t argc);

    void run(MjvmMethodInfo &method, MjvmDebugger *dbg);

    bool getStackTrace(uint32_t index, MjvmStackFrame *stackTrace, bool *isEndStack) const;
    bool readLocal(uint32_t stackIndex, uint32_t localIndex, uint32_t &value, bool &isObject) const;
    bool readLocal(uint32_t stackIndex, uint32_t localIndex, uint64_t &value) const;

    friend class Mjvm;
    friend class MjvmDebugger;
public:
    bool isInstanceof(MjvmObject *obj, const char *typeName, uint16_t length);

    void garbageCollection(void);

    MjvmClassLoader &load(const char *className, uint16_t length);
    MjvmClassLoader &load(const char *className);
    MjvmClassLoader &load(MjvmConstUtf8 &className);

    void runToMain(const char *mainClass, MjvmDebugger *dbg = 0);
};

#endif /* __MJVM_EXECUTION_H */
