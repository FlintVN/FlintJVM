
#ifndef __MJVM_DEBUGGER_H
#define __MJVM_DEBUGGER_H

#include "mjvm_method_info.h"
#include "mjvm_throwable.h"
#include "mjvm_const_name.h"

#if __has_include("mjvm_conf.h")
#include "mjvm_conf.h"
#endif
#include "mjvm_default_conf.h"

#define DBG_STATUS_STOP             0x0001
#define DBG_STATUS_STOP_SET         0x0002
#define DBG_STATUS_EXCP             0x0004
#define DBG_STATUS_RESET            0x0080

#define DBG_CONTROL_STOP            0x0100
#define DBG_CONTROL_STEP_IN         0x0200
#define DBG_CONTROL_STEP_OVER       0x0400
#define DBG_CONTROL_STEP_OUT        0x0800
#define DBG_CONTROL_EXCP_EN         0x1000

class Mjvm;
class MjvmExecution;

typedef enum : uint8_t {
    DBG_CMD_READ_STATUS,
    DBG_CMD_READ_STACK_TRACE,
    DBG_CMD_ADD_BKP,
    DBG_CMD_REMOVE_BKP,
    DBG_CMD_REMOVE_ALL_BKP,
    DBG_CMD_RUN,
    DBG_CMD_STOP,
    DBG_CMD_RESTART,
    DBG_CMD_TERMINATE,
    DBG_CMD_STEP_IN,
    DBG_CMD_STEP_OVER,
    DBG_CMD_STEP_OUT,
    DBG_CMD_SET_EXCP_MODE,
    DBG_CMD_READ_EXCP_INFO,
    DBG_CMD_READ_LOCAL,
    DBG_CMD_WRITE_LOCAL,
    DBG_CMD_READ_FIELD,
    DBG_CMD_WRITE_FIELD,
    DBG_CMD_READ_ARRAY,
    DBG_CMD_READ_SIZE_AND_TYPE,
} MjvmDbgCmd;

typedef enum : uint8_t {
    DBG_RESP_OK = 0,
    DBG_RESP_BUSY = 1,
    DBG_RESP_FAIL = 2,
    DBG_RESP_UNKNOW = 0xFF,
} MjvmDbgRespCode;

class MjvmBreakPoint {
public:
    uint32_t pc;
    MjvmMethodInfo *method;

    MjvmBreakPoint(void);
    MjvmBreakPoint(uint32_t pc, MjvmMethodInfo &method);
private:
    MjvmBreakPoint(const MjvmBreakPoint &) = delete;
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
    Mjvm &mjvm;
    MjvmExecution *execution;
    MjvmThrowable *exception;
    volatile uint32_t stepCodeLength;
    uint32_t txDataLength;
    volatile uint16_t csr;
    volatile uint8_t breakPointCount;
    MjvmStackFrame startPoint;
    MjvmBreakPoint breakPoints[MAX_OF_BREAK_POINT];
    uint8_t txBuff[MAX_OF_DBG_BUFFER];
public:
    MjvmDebugger(Mjvm &mjvm);

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

    void responseStatus(void);
    void responseStackTrace(uint32_t stackIndex);
    void responseExceptionInfo(void);
    void responseLocalVariable(bool isU64, uint32_t stackIndex, uint32_t localIndex);
    void responseField(MjvmObject *obj, MjvmConstUtf8 &fieldName);
    void responseArray(MjvmObject *array, uint32_t index, uint32_t length);
    void responseObjSizeAndType(MjvmObject *obj);

    void receivedDataHandler(uint8_t *data, uint32_t length);

    bool exceptionIsEnabled(void);
    void checkBreakPoint(MjvmExecution *exec);
    void caughtException(MjvmExecution *exec, MjvmThrowable *excp);

    void clearResetStatus(void);
private:
    MjvmDebugger(const MjvmDebugger &) = delete;
    void operator=(const MjvmDebugger &) = delete;

    bool addBreakPoint(uint32_t pc, MjvmConstUtf8 &className, MjvmConstUtf8 &methodName, MjvmConstUtf8 &descriptor);
    bool removeBreakPoint(uint32_t pc, MjvmConstUtf8 &className, MjvmConstUtf8 &methodName, MjvmConstUtf8 &descriptor);
};

#endif /* __MJVM_DEBUGGER_H */
