
#ifndef __MJVM_EXECUTION_H
#define __MJVM_EXECUTION_H

#include "mjvm_common.h"
#include "mjvm_debugger.h"
#include "mjvm_stack_info.h"
#include "mjvm_const_pool.h"
#include "mjvm_method_info.h"

#define STR_AND_SIZE(str)           str, (sizeof(str) - 1)

class Mjvm;

class MjvmExecution {
public:
    Mjvm &mjvm;
private:
    const void ** volatile opcodes;
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
    const char *mainClass;
protected:
    MjvmExecution(Mjvm &mjvm);
    MjvmExecution(Mjvm &mjvm, uint32_t stackSize);
    MjvmExecution(const MjvmExecution &) = delete;
    void operator=(const MjvmExecution &) = delete;

    ~MjvmExecution(void);
private:
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

    bool invoke(MjvmMethodInfo &methodInfo, uint8_t argc);
    bool invokeStatic(MjvmConstMethod &constMethod);
    bool invokeSpecial(MjvmConstMethod &constMethod);
    bool invokeVirtual(MjvmConstMethod &constMethod);
    bool invokeInterface(MjvmConstInterfaceMethod &interfaceMethod, uint8_t argc);

    void run(MjvmMethodInfo &method);
    bool isRunning(void) const;
    void terminateRequest(void);
    static void runToMainTask(MjvmExecution *execution);

    friend class Mjvm;
    friend class MjvmDebugger;
public:
    bool runToMain(const char *mainClass);

    bool getStackTrace(uint32_t index, MjvmStackFrame *stackTrace, bool *isEndStack) const;
    bool readLocal(uint32_t stackIndex, uint32_t localIndex, uint32_t &value, bool &isObject) const;
    bool readLocal(uint32_t stackIndex, uint32_t localIndex, uint64_t &value) const;
};

#endif /* __MJVM_EXECUTION_H */
