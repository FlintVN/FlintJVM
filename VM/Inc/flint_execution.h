
#ifndef __FLINT_EXECUTION_H
#define __FLINT_EXECUTION_H

#include "flint_common.h"
#include "flint_debugger.h"
#include "flint_const_pool.h"
#include "flint_method_info.h"
#include "flint_java_thread.h"
#include "flint_class_data_binary_tree.h"

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
    JThread *onwerThread;
protected:
    FlintExecution(Flint &flint, JThread *onwerThread, uint32_t stackSize = DEFAULT_STACK_SIZE);
    FlintExecution(const FlintExecution &) = delete;
    void operator=(const FlintExecution &) = delete;

    ~FlintExecution(void);
public:
    void stackPushInt32(int32_t value);
    void stackPushInt64(int64_t value);
    void stackPushFloat(float value);
    void stackPushDouble(double value);
    void stackPushObject(JObject *obj);

    int32_t stackPopInt32(void);
    int64_t stackPopInt64(void);
    float stackPopFloat(void);
    double stackPopDouble(void);
    JObject *stackPopObject(void);
private:
    FlintError initNewContext(FlintMethodInfo *methodInfo, uint16_t argc);

    FlintError stackInitExitPoint(uint32_t exitPc);
    void stackRestoreContext(void);

    FlintError lockClass(FlintClassData *cls);
    void unlockClass(FlintClassData *cls);
    FlintError lockObject(JObject *obj);
    void unlockObject(JObject *obj);

    FlintError invoke(FlintMethodInfo *methodInfo, uint8_t argc);
    FlintError invokeStatic(FlintConstMethod *constMethod);
    FlintError invokeSpecial(FlintConstMethod *constMethod);
    FlintError invokeVirtual(FlintConstMethod *constMethod);
    FlintError invokeInterface(FlintConstInterfaceMethod *interfaceMethod, uint8_t argc);
    FlintError invokeStaticCtor(FlintClassData *classData);

    FlintError run(void);
    void stopRequest(void);
    void terminateRequest(void);
    bool getStackTrace(uint32_t index, FlintStackFrame *stackTrace, bool *isEndStack) const;
    bool readLocal(uint32_t stackIndex, uint32_t localIndex, uint32_t &value, bool &isObject) const;
    bool readLocal(uint32_t stackIndex, uint32_t localIndex, uint64_t &value) const;

    static void runTask(FlintExecution *execution);
public:
    bool run(FlintMethodInfo *method);
    bool hasTerminateRequest(void) const;
    FlintResult<JThread> getOnwerThread(void);

    friend class Flint;
    friend class FlintDebugger;
};

#endif /* __FLINT_EXECUTION_H */
