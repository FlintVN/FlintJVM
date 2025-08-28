
#ifndef __FLINT_EXECUTION_H
#define __FLINT_EXECUTION_H

#include <cstdarg>
#include "flint_common.h"
// #include "flint_debugger.h"
#include "flint_list.h"
#include "flint_const_pool.h"
#include "flint_method_info.h"
#include "flint_java_thread.h"
#include "flint_java_throwable.h"
#include "flint_default_conf.h"

class FExec : ListNode<FExec> {
private:
    const void ** volatile opcodes;
    const uint32_t stackLength;
    MethodInfo *method;
    const uint8_t *code;
    uint32_t pc;
    uint32_t lr;
    int32_t sp;
    int32_t startSp;
    int32_t peakSp;
    int32_t *locals;
    JThread *onwerThread;
    JThrowable *excp;
    int32_t stack[];

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

    void initNewContext(MethodInfo *methodInfo, uint16_t argc);
    void stackInitExitPoint(uint32_t exitPc);
    void stackRestoreContext(void);

    bool lockClass(ClassLoader *cls);
    void unlockClass(ClassLoader *cls);
    bool lockObject(JObject *obj);
    void unlockObject(JObject *obj);

    void invokeNativeMethod(MethodInfo *methodInfo, uint8_t argc);
    void invoke(MethodInfo *methodInfo, uint8_t argc);
    void invokeStatic(ConstMethod *constMethod);
    void invokeSpecial(ConstMethod *constMethod);
    void invokeVirtual(ConstMethod *constMethod);
    void invokeInterface(ConstInterfaceMethod *interfaceMethod, uint8_t argc);
    void invokeStaticCtor(ClassLoader *cls);

    void exec(void);
    void stopRequest(void);
    void terminateRequest(void);
    // bool getStackTrace(uint32_t index, FlintStackFrame *stackTrace, bool *isEndStack) const;
    // bool readLocal(uint32_t stackIndex, uint32_t localIndex, uint32_t &value, bool &isObject) const;
    // bool readLocal(uint32_t stackIndex, uint32_t localIndex, uint64_t &value) const;

    static void runTask(FExec *execution);
public:
    bool run(MethodInfo *method, uint32_t argc = 0, ...);

    void throwNew(JClass *cls, const char *msg = NULL, ...);
    void vThrowNew(JClass *cls, const char *msg, va_list args);

    bool hasTerminateRequest(void) const;
    JThread *getOnwerThread(void);
private:
    FExec(JThread *onwer, uint32_t stackSize);
    FExec(const FExec &) = delete;
    void operator=(const FExec &) = delete;

    friend class Flint;
    friend class FlintDebugger;
};

#endif /* __FLINT_EXECUTION_H */
