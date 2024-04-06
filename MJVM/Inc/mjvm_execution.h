
#include "mjvm_common.h"
#include "mjvm_heap.h"
#include "mjvm_class_loader.h"
#include "mjvm_fields_data.h"

#define KILO_BYTE(value)        (value * 1024)
#define MEAG_BYTE(value)        (value * KILO_BYTE(1024))

#define DEFAULT_STACK_SIZE      (MEAG_BYTE(1))

typedef enum : uint8_t {
    STACK_TYPE_NON_OBJECT = 0,
    STACK_TYPE_OBJECT = 1,
} StackType;

typedef struct {
    StackType type;
    int32_t value;
} StackValue;

class Execution {
private:
    const uint32_t stackLength;
    const MethodInfo *method;
    const uint8_t *code;
    uint32_t pc;
    int32_t sp;
    int32_t startSp;
    int32_t *stack;
    int32_t *locals;
    uint8_t *stackType;
    ClassDataNode *staticClassDataHead;

    MjvmObject *newObject(uint32_t size, const ConstUtf8 &type, uint8_t dimensions = 0) const;

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

    void stackSaveContext(uint32_t retPc);
    void stackRestoreContext(void);

    void initNewContext(const MethodInfo &methodInfo, uint16_t argc = 0);

    const MethodInfo &findMethod(const ConstMethod &constMethod);
    void invokeStatic(const ConstMethod &constMethod, uint32_t retPc);
    void invokeSpecial(const ConstMethod &constMethod, uint32_t retPc);
    void invokeVirtual(const ConstMethod &constMethod, uint32_t retPc);
public:
    Execution(void);
    Execution(uint32_t stackSize);

    const ClassLoader &load(const char *className);
    const ClassLoader &load(const char *className, uint16_t length);
    const ClassLoader &load(const ConstUtf8 &className);

    int64_t run(const char *mainClass);

    ~Execution(void);
};
