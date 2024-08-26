
#ifndef __FLINT_EXECUTION_H
#define __FLINT_EXECUTION_H

#include "flint_common.h"
#include "flint_debugger.h"
#include "flint_stack_info.h"
#include "flint_const_pool.h"
#include "flint_method_info.h"

#define STR_AND_SIZE(str)           str, (sizeof(str) - 1)

class FlintExecution {
public:
    class Flint &flint;
private:
    const void ** volatile opcodes;
    const uint32_t stackLength;
    FlintMethodInfo *method;
    const uint8_t *code;
    uint32_t pc;
    uint32_t lr;
    int32_t sp;
    int32_t startSp;
    int32_t peakSp;
    int32_t *stack;
    int32_t *locals;
    uint8_t *stackType;
protected:
    FlintExecution(Flint &flint);
    FlintExecution(Flint &flint, uint32_t stackSize);
    FlintExecution(const FlintExecution &) = delete;
    void operator=(const FlintExecution &) = delete;

    ~FlintExecution(void);
private:
    FlintStackType getStackType(uint32_t index);
    FlintStackValue getStackValue(uint32_t index);
    void setStackValue(uint32_t index, FlintStackValue &value);

    void stackPush(FlintStackValue &value);
public:
    void stackPushInt32(int32_t value);
    void stackPushInt64(int64_t value);
    void stackPushFloat(float value);
    void stackPushDouble(double value);
    void stackPushObject(FlintObject *obj);

    int32_t stackPopInt32(void);
    int64_t stackPopInt64(void);
    float stackPopFloat(void);
    double stackPopDouble(void);
    FlintObject *stackPopObject(void);
private:
    void stackInitExitPoint(uint32_t exitPc);
    void stackRestoreContext(void);

    void initNewContext(FlintMethodInfo &methodInfo, uint16_t argc = 0);

    bool invoke(FlintMethodInfo &methodInfo, uint8_t argc);
    bool invokeStatic(FlintConstMethod &constMethod);
    bool invokeSpecial(FlintConstMethod &constMethod);
    bool invokeVirtual(FlintConstMethod &constMethod);
    bool invokeInterface(FlintConstInterfaceMethod &interfaceMethod, uint8_t argc);

    void run(void);
    void terminateRequest(void);
    bool getStackTrace(uint32_t index, FlintStackFrame *stackTrace, bool *isEndStack) const;
    bool readLocal(uint32_t stackIndex, uint32_t localIndex, uint32_t &value, bool &isObject) const;
    bool readLocal(uint32_t stackIndex, uint32_t localIndex, uint64_t &value) const;

    static void runTask(FlintExecution *execution);
public:
    bool run(FlintMethodInfo &method);

    friend class Flint;
    friend class FlintDebugger;
};

#endif /* __FLINT_EXECUTION_H */
