
#ifndef __MJVM_EXECUTION_H
#define __MJVM_EXECUTION_H

#include "mjvm_common.h"
#include "mjvm_string.h"
#include "mjvm_throwable.h"
#include "mjvm_class_loader.h"
#include "mjvm_fields_data.h"

#define KILO_BYTE(value)        (value * 1024)
#define MEAG_BYTE(value)        (value * KILO_BYTE(1024))

#define DEFAULT_STACK_SIZE      (MEAG_BYTE(1))

#define NUM_OBJECT_TO_GC        1000

class Execution {
private:
    const uint32_t stackLength;
    const MethodInfo *method;
    const uint8_t *code;
    uint32_t pc;
    uint32_t lr;
    int32_t sp;
    int32_t startSp;
    int32_t peakSp;
    int32_t *stack;
    int32_t *locals;
    uint8_t *stackType;
    ClassDataNode *staticClassDataList;
    MjvmObjectNode *objectList;
    MjvmObjectNode *constStringList;
    uint32_t objectCountToGc;

    typedef enum : uint8_t {
        STACK_TYPE_NON_OBJECT = 0,
        STACK_TYPE_OBJECT = 1,
    } StackType;

    typedef struct {
        StackType type;
        int32_t value;
    } StackValue;

    Execution(void);
    Execution(uint32_t stackSize);
    Execution(const Execution &) = delete;
    void operator=(const Execution &) = delete;

    void addToList(MjvmObjectNode **list, MjvmObjectNode *objNode);
    void removeFromList(MjvmObjectNode **list, MjvmObjectNode *objNode);
    MjvmObject *newObject(uint32_t size, const ConstUtf8 &type, uint8_t dimensions = 0);
    MjvmObject *newString(const char *str);
    MjvmObject *newString(const char *str, uint16_t length);
    MjvmObject *newString(const char *str[], uint16_t count);
    MjvmObjectNode *newStringNode(const char *str, uint16_t length);
    MjvmObjectNode *newStringNode(const char *str[], uint16_t count);
    MjvmObject *getConstString(const ConstUtf8 &str);
    MjvmObject *newArithmeticException(MjvmObject *strObj);
    MjvmObject *newNullPointerException(MjvmObject *strObj);
    MjvmObject *newNegativeArraySizeException(MjvmObject *strObj);
    MjvmObject *newArrayIndexOutOfBoundsException(MjvmObject *strObj);
    void freeAllObject(void);
    void garbageCollectionProtectObject(MjvmObject *obj);

    void initStaticField(const ClassLoader &classLoader);
    const FieldsData &getStaticFields(const ConstUtf8 &className) const;
    const FieldsData &getStaticFields(const ClassLoader &classLoader) const;

    StackType getStackType(uint32_t index);
    StackValue getStackValue(uint32_t index);
    void setStackValue(uint32_t index, const StackValue &value);

    void stackPush(const StackValue &value);
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

    void stackRestoreContext(void);

    void initNewContext(const MethodInfo &methodInfo, uint16_t argc = 0);

    const MethodInfo &findMethod(const ConstMethod &constMethod);
    void invoke(const MethodInfo &methodInfo, uint8_t argc);
    void invokeStatic(const ConstMethod &constMethod);
    void invokeSpecial(const ConstMethod &constMethod);
    void invokeVirtual(const ConstMethod &constMethod);
    void invokeInterface(const ConstInterfaceMethod &interfaceMethod, uint8_t argc);

    MjvmObject *newMultiArray(const ConstUtf8 &typeName, uint8_t dimensions, int32_t *counts);

    bool isInstanceof(MjvmObject *obj, const ConstUtf8 &type);

    void garbageCollection(void);

    ~Execution(void);

    friend class Mjvm;
public:
    const ClassLoader &load(const char *className);
    const ClassLoader &load(const char *className, uint16_t length);
    const ClassLoader &load(const ConstUtf8 &className);

    int64_t run(const char *mainClass);
};

#endif /* __MJVM_EXECUTION_H */
