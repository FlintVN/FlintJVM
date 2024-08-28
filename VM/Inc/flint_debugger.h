
#ifndef __FLINT_DEBUGGER_H
#define __FLINT_DEBUGGER_H

#include "flint_method_info.h"
#include "flint_throwable.h"
#include "flint_const_name.h"
#include "flint_system_api.h"

#if __has_include("flint_conf.h")
#include "flint_conf.h"
#endif
#include "flint_default_conf.h"

#define DBG_STATUS_STOP             0x0001
#define DBG_STATUS_STOP_SET         0x0002
#define DBG_STATUS_EXCP             0x0004
#define DBG_STATUS_CONSOLE          0x0008
#define DBG_STATUS_DONE             0x0040
#define DBG_STATUS_RESET            0x0080

#define DBG_CONTROL_STOP            0x0100
#define DBG_CONTROL_STEP_IN         0x0200
#define DBG_CONTROL_STEP_OVER       0x0400
#define DBG_CONTROL_STEP_OUT        0x0800
#define DBG_CONTROL_EXCP_EN         0x1000

typedef enum : uint8_t {
    DBG_CMD_ENTER_DEBUG,
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
    DBG_CMD_OPEN_FILE,
    DBG_CMD_READ_FILE,
    DBG_CMD_WRITE_FILE,
    DBG_CMD_CLOSE_FILE,
    DBG_CMD_DELETE_FILE,
    DBG_CMD_READ_CONSOLE,
} FlintDbgCmd;

typedef enum : uint8_t {
    DBG_RESP_OK = 0,
    DBG_RESP_BUSY = 1,
    DBG_RESP_FAIL = 2,
    DBG_RESP_CRC_FAIL = 3,
    DBG_RESP_LENGTH_INVAILD = 4,
    DBG_RESP_UNKNOW = 0xFF,
} FlintDbgRespCode;

class FlintBreakPoint {
public:
    uint32_t pc;
    FlintMethodInfo *method;

    FlintBreakPoint(void);
    FlintBreakPoint(uint32_t pc, FlintMethodInfo &method);
private:
    FlintBreakPoint(const FlintBreakPoint &) = delete;
};

class FlintStackFrame {
public:
    const uint32_t pc;
    const uint32_t baseSp;
    FlintMethodInfo &method;

    FlintStackFrame(void);
    FlintStackFrame(uint32_t pc, uint32_t baseSp, FlintMethodInfo &method);
private:
    FlintStackFrame(const FlintStackFrame &) = delete;
    void operator=(const FlintStackFrame &) = delete;
};

class FlintDebugger {
private:
    FlintAPI::Thread::LockHandle *dbgLockHandle;
    class Flint &flint;
    class FlintExecution *execution;
    FlintThrowable *exception;
    void *fileHandle;
    volatile uint32_t stepCodeLength;
    volatile uint32_t consoleOffset;
    volatile uint32_t consoleLength;
    uint32_t txDataLength;
    uint16_t txDataCrc;
    volatile uint16_t csr;
    volatile uint8_t breakPointCount;
    FlintStackFrame startPoint;
    FlintBreakPoint breakPoints[MAX_OF_BREAK_POINT];
    uint8_t consoleBuff[DBG_CONSOLE_BUFFER_SIZE];
    uint8_t txBuff[DBG_TX_BUFFER_SIZE];
    uint8_t fileBuff[256];
public:
    FlintDebugger(Flint &flint);

    virtual bool sendData(uint8_t *data, uint32_t length) = 0;

    void print(const char *text, uint32_t length, uint8_t coder);
private:
    void consolePut(uint16_t ch);
    void consoleClear(void);

    void clearTxBuffer(void);
    void initDataFrame(FlintDbgCmd cmd, FlintDbgRespCode responseCode, uint32_t dataLength);
    bool dataFrameAppend(uint8_t data);
    bool dataFrameAppend(uint16_t data);
    bool dataFrameAppend(uint32_t data);
    bool dataFrameAppend(uint64_t data);
    bool dataFrameAppend(uint8_t *data, uint16_t length);
    bool dataFrameAppend(FlintConstUtf8 &utf8);
    bool dataFrameFinish(void);
    bool sendRespCode(FlintDbgCmd cmd, FlintDbgRespCode responseCode);

    void responseStatus(void);
    void responseStackTrace(uint32_t stackIndex);
    void responseExceptionInfo(void);
    void responseLocalVariable(bool isU64, uint32_t stackIndex, uint32_t localIndex);
    void responseField(FlintObject *obj, FlintConstUtf8 &fieldName);
    void responseArray(FlintObject *array, uint32_t index, uint32_t length);
    void responseObjSizeAndType(FlintObject *obj);
    void responseConsoleBuffer(void);
public:
    bool receivedDataHandler(uint8_t *data, uint32_t length);
    bool exceptionIsEnabled(void);
    void checkBreakPoint(FlintExecution *exec);
    void caughtException(FlintExecution *exec, FlintThrowable *excp);
private:
    FlintDebugger(const FlintDebugger &) = delete;
    void operator=(const FlintDebugger &) = delete;

    bool addBreakPoint(uint32_t pc, FlintConstUtf8 &className, FlintConstUtf8 &methodName, FlintConstUtf8 &descriptor);
    bool removeBreakPoint(uint32_t pc, FlintConstUtf8 &className, FlintConstUtf8 &methodName, FlintConstUtf8 &descriptor);

    void lock(void);
    void unlock(void);
};

#endif /* __FLINT_DEBUGGER_H */
