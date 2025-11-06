
#ifndef __FLINT_DEBUGGER_H
#define __FLINT_DEBUGGER_H

#include "flint_method_info.h"
#include "flint_java_throwable.h"
#include "flint_mutex.h"
#include "flint_system_api.h"
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
    DBG_CMD_READ_VM_INFO,
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
    DBG_CMD_READ_CONSOLE,
    DBG_CMD_OPEN_FILE,
    DBG_CMD_READ_FILE,
    DBG_CMD_WRITE_FILE,
    DBG_CMD_SEEK_FILE,
    DBG_CMD_CLOSE_FILE,
    DBG_CMD_FILE_INFO,
    DBG_CMD_DELETE_FILE,
    DBG_CMD_OPEN_DIR,
    DBG_CMD_READ_DIR,
    DBG_CMD_CREATE_DIR,
    DBG_CMD_CLOSE_DIR,
} DbgCmd;

typedef enum : uint8_t {
    DBG_RESP_OK = 0,
    DBG_RESP_BUSY = 1,
    DBG_RESP_FAIL = 2,
    DBG_RESP_CRC_FAIL = 3,
    DBG_RESP_LENGTH_INVAILD = 4,
    DBG_RESP_UNKNOW = 0xFF,
} DbgRespCode;

class BreakPoint {
public:
    uint8_t opcode;
    uint32_t pc;
    MethodInfo *method;

    BreakPoint(void);
    BreakPoint(uint8_t opcode, uint32_t pc, MethodInfo *method);
private:
    BreakPoint(const BreakPoint &) = delete;
};

class StackFrame {
public:
    const uint32_t pc;
    const uint32_t baseSp;
    MethodInfo * const method;

    StackFrame(void);
    StackFrame(uint32_t pc, uint32_t baseSp, MethodInfo *method);
private:
    StackFrame(const StackFrame &) = delete;
    void operator=(const StackFrame &) = delete;
};

class FDbg {
private:
    FMutex dbgMutex;
    class FExec *exec;
    void *dirHandle;
    void *fileHandle;
    uint32_t stepCodeLength;
    uint32_t consoleOffset;
    uint32_t consoleLength;
    uint32_t txDataLength;
    uint16_t txDataCrc;
    volatile uint16_t csr;
    uint8_t breakPointCount;
    StackFrame startPoint;
    BreakPoint breakPoints[MAX_OF_BREAK_POINT];
    uint8_t consoleBuff[DBG_CONSOLE_BUFFER_SIZE];
    uint8_t txBuff[DBG_TX_BUFFER_SIZE];
public:
    FDbg(void);

    virtual bool sendData(uint8_t *data, uint32_t length) = 0;

    void consoleWrite(uint8_t *utf8, uint32_t length);
private:
    void consolePut(uint8_t ch);
    void consoleClear(void);

    void clearTxBuffer(void);
    void initDataFrame(DbgCmd cmd, DbgRespCode responseCode, uint32_t dataLength);
    bool dataFrameAppend(uint8_t data);
    bool dataFrameAppend(uint16_t data);
    bool dataFrameAppend(uint32_t data);
    bool dataFrameAppend(uint64_t data);
    bool dataFrameAppend(uint8_t *data, uint16_t length);
    bool dataFrameAppend(const char *utf8);
    bool dataFrameFinish(void);
    bool sendRespCode(DbgCmd cmd, DbgRespCode responseCode);

    void responseInfo(void);
    void responseStatus(void);
    void responseStackTrace(uint32_t stackIndex);
    void responseExceptionInfo(void);
    void responseLocalVariable(uint32_t stackIndex, uint32_t localIndex, uint8_t variableType);
    void responseField(JObject *obj, const char *fieldName);
    void responseArray(JObject *array, uint32_t index, uint32_t length);
    void responseObjSizeAndType(JObject *obj);
    void responseOpenFile(char *fileName, FlintAPI::IO::FileMode mode);
    void responseReadFile(uint32_t size);
    void responseWriteFile(uint8_t *data, uint32_t size);
    void responseSeekFile(uint32_t offset);
    void responseCloseFile(void);
    void responseFileInfo(const char *fileName);
    void responseCreateDelete(DbgCmd cmd, const char *path);
    void responseOpenDir(const char *path);
    void responseReadDir(void);
    void responseCloseDir(void);
    void responseConsoleBuffer(void);
public:
    bool receivedDataHandler(uint8_t *data, uint32_t length);
    bool exceptionIsEnabled(void);
    bool waitStop(FExec *exec);
    bool checkStop(FExec *exec);
    void caughtException(FExec *exec);
    void hitBreakpoint(FExec *exec);
    uint8_t getSavedOpcode(uint32_t pc, MethodInfo *method);
private:
    FDbg(const FDbg &) = delete;
    void operator=(const FDbg &) = delete;

    bool addBreakPoint(uint32_t pc, const char *clsName, const char *name, const char *desc);
    bool removeBreakPoint(uint32_t pc, const char *clsName, const char *name, const char *desc);

    void lock(void);
    void unlock(void);
};

#endif /* __FLINT_DEBUGGER_H */
