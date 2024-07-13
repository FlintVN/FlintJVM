
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
#define DBG_STATUS_EXCP_EN          0x20
#define DBG_STATUS_EXCP             0x40
#define DBG_STATUS_DONE             0x80

class MjvmExecution;

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
    DBG_SET_EXCP_MODE,
    DBG_READ_LOCAL,
    DBG_WRITE_LOCAL,
} MjvmDbgCmd;

typedef enum : uint8_t {
    DBG_RESP_OK = 0,
    DBG_RESP_BUSY = 1,
    DBG_RESP_FAIL = 2,
    DBG_RESP_UNKNOW = 2,
} MjvmDbgRespCode;

class BreakPoint {
public:
    uint32_t pc;
    MjvmMethodInfo *method;

    BreakPoint(void);
    BreakPoint(uint32_t pc, MjvmMethodInfo &method);
private:
    BreakPoint(const BreakPoint &) = delete;
};

class MjvmStackFrame {
public:
    const uint32_t pc;
    const uint32_t baseSp;
    MjvmMethodInfo &method;

    MjvmStackFrame(void);
    MjvmStackFrame(uint32_t pc, uint32_t baseSp, MjvmMethodInfo &method);
private:
    MjvmStackFrame(const MjvmStackFrame &) = delete;
    void operator=(const MjvmStackFrame &) = delete;
};

class MjvmDebugger {
private:
    MjvmExecution &execution;
    volatile uint32_t stepCodeLength;
    uint32_t txDataLength;
    volatile uint8_t status;
    volatile uint8_t breakPointCount;
    MjvmStackFrame startPoint;
    BreakPoint breakPoints[MAX_OF_BREAK_POINT];
    uint8_t txBuff[MAX_OF_DBG_BUFFER];
public:
    MjvmDebugger(MjvmExecution &execution);

    virtual bool sendData(uint8_t *data, uint32_t length) = 0;
    void clearTxBuffer(void);
    void initDataFrame(MjvmDbgCmd cmd, MjvmDbgRespCode responseCode, uint32_t dataLength);
    bool dataFrameAppend(uint8_t data);
    bool dataFrameAppend(uint16_t data);
    bool dataFrameAppend(uint32_t data);
    bool dataFrameAppend(uint64_t data);
    bool dataFrameAppend(uint8_t *data, uint16_t length);
    bool dataFrameAppend(MjvmConstUtf8 &utf8);
    bool dataFrameFinish(void);
    bool sendRespCode(MjvmDbgCmd cmd, MjvmDbgRespCode responseCode);

    void receivedDataHandler(uint8_t *data, uint32_t length);

    bool exceptionIsEnabled(void);
    void caughtException(void);
    void checkBreakPoint(void);
    void done(void);
private:
    MjvmDebugger(const MjvmDebugger &) = delete;
    void operator=(const MjvmDebugger &) = delete;

    bool addBreakPoint(uint32_t pc, MjvmConstUtf8 &className, MjvmConstUtf8 &methodName, MjvmConstUtf8 &descriptor);
    bool removeBreakPoint(uint32_t pc, MjvmConstUtf8 &className, MjvmConstUtf8 &methodName, MjvmConstUtf8 &descriptor);
};

#endif /* __MJVM_DEBUGGER_H */
