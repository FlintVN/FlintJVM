
#ifndef __MJVM_DEBUGGER_H
#define __MJVM_DEBUGGER_H

#include "mjvm_method_info.h"

#if __has_include("mjvm_conf.h")
#include "mjvm_conf.h"
#endif
#include "mjvm_default_conf.h"

#define DBG_STATUS_STOP             0x01
#define DBG_STATUS_STOP_SET         0x02
#define DBG_STATUS_STEP_IN          0x04
#define DBG_STATUS_STEP_OVER        0x08
#define DBG_STATUS_STEP_OUT         0x10

class Execution;

typedef enum : uint8_t {
    DBG_READ_STATUS,
    DBG_READ_STACK_TRACE,
    DBG_ADD_BKP,
    DBG_REMOVE_BKP,
    DBG_REMOVE_ALL_BKP,
    DBG_RUN,
    DBG_STOP,
    DBG_STEP_IN,
    DBG_STEP_OVER,
    DBG_STEP_OUT,
    DBG_READ_VARIABLE,
    DBG_WRITE_VARIABLE,
} DebuggerCmd;

class BreakPoint {
public:
    uint32_t pc;
    MethodInfo *method;

    BreakPoint(void);
    BreakPoint(uint32_t pc, MethodInfo &method);
private:
    BreakPoint(const BreakPoint &) = delete;
};

class StackTrace {
public:
    const uint32_t pc;
    const uint32_t baseSp;
    MethodInfo &method;

    StackTrace(void);
    StackTrace(uint32_t pc, uint32_t baseSp, MethodInfo &method);
private:
    StackTrace(const StackTrace &) = delete;
    void operator=(const StackTrace &) = delete;
};

class Debugger {
private:
    Execution &execution;
    volatile uint32_t stepCodeLength;
    StackTrace startPoint;
public:
    volatile uint8_t status;
    volatile uint8_t breakPointCount;
    BreakPoint breakPoints[MAX_OF_BREAK_POINT];

    Debugger(Execution &execution);

    virtual bool sendData(uint8_t *data, uint32_t length) = 0;

    void receivedDataHandler(uint8_t *data, uint32_t length);

    void checkBreakPoint(uint32_t pc, uint32_t baseSp, const MethodInfo *method);
private:
    Debugger(const Debugger &) = delete;
    void operator=(const Debugger &) = delete;

    bool addBreakPoint(uint32_t pc, ConstUtf8 &className, ConstUtf8 &methodName, ConstUtf8 &descriptor);
    bool removeBreakPoint(uint32_t pc, ConstUtf8 &className, ConstUtf8 &methodName, ConstUtf8 &descriptor);
};

#endif /* __MJVM_DEBUGGER_H */
