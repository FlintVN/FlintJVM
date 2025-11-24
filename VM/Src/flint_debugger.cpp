
#include <iostream>
#include <string.h>
#include "flint.h"
#include "flint_utf8.h"
#include "flint_opcodes.h"
#include "flint_debugger.h"

BreakPoint::BreakPoint(void) : pc(0), method(NULL) {

}

BreakPoint::BreakPoint(uint8_t opcode, uint32_t pc, MethodInfo *method) : opcode(opcode), pc(pc), method(method) {

}

StackFrame::StackFrame(void) : pc(0), baseSp(0), method(NULL) {

}

StackFrame::StackFrame(uint32_t pc, uint32_t baseSp, MethodInfo *method) : pc(pc), baseSp(baseSp), method(method) {

}

FDbg::FDbg(void) : dbgMutex() {
    exec = NULL;
    dirHandle = NULL;
    fileHandle = NULL;
    stepCodeLength = 0;
    consoleOffset = 0;
    consoleLength = 0;
    csr = DBG_STATUS_RESET;
    breakPointCount = 0;
    txDataLength = 0;
}

void FDbg::consoleWrite(uint8_t *utf8, uint32_t length) {
    lock();
    while(length) {
        consolePut((uint8_t)*utf8);
        utf8++;
        length--;
    }
    unlock();
}

void FDbg::consolePut(uint8_t ch) {
    uint32_t nextOffset = (consoleOffset + 1) % sizeof(consoleBuff);
    if(consoleLength == sizeof(consoleBuff))
        consoleLength -= Utf8EncodeSize(consoleBuff[nextOffset]);
    consoleBuff[consoleOffset] = ch;
    consoleOffset = nextOffset;
    if(consoleLength < sizeof(consoleBuff))
        consoleLength++;
}

void FDbg::consoleClear(void) {
    consoleOffset = 0;
    consoleLength = 0;
}

void FDbg::clearTxBuffer(void) {
    txDataLength = 0;
}

void FDbg::initDataFrame(DbgCmd cmd, DbgRespCode responseCode, uint32_t dataLength) {
    dataLength += 7;
    txBuff[0] = (uint8_t)(cmd | 0x80);
    txBuff[1] = (uint8_t)(dataLength >> 0);
    txBuff[2] = (uint8_t)(dataLength >> 8);
    txBuff[3] = (uint8_t)(dataLength >> 16);
    txBuff[4] = (uint8_t)responseCode;
    txDataLength = 5;
    txDataCrc = txBuff[0] + txBuff[1] + txBuff[2] + txBuff[3] + txBuff[4];
}

bool FDbg::dataFrameAppend(uint8_t data) {
    if(txDataLength == sizeof(txBuff)) {
        if(!sendData(txBuff, sizeof(txBuff)))
            return false;
        txDataLength = 0;
    }
    txBuff[txDataLength++] = data;
    txDataCrc += data;
    return true;
}

bool FDbg::dataFrameAppend(uint16_t data) {
    if(!dataFrameAppend((uint8_t)data))
        return false;
    return dataFrameAppend((uint8_t)(data >> 8));
}

bool FDbg::dataFrameAppend(uint32_t data) {
    if(!dataFrameAppend((uint8_t)data))
        return false;
    else if(!dataFrameAppend((uint8_t)(data >> 8)))
        return false;
    else if(!dataFrameAppend((uint8_t)(data >> 16)))
        return false;
    return dataFrameAppend((uint8_t)(data >> 24));
}

bool FDbg::dataFrameAppend(uint64_t data) {
    if(!dataFrameAppend((uint8_t)data))
        return false;
    else if(!dataFrameAppend((uint8_t)(data >> 8)))
        return false;
    else if(!dataFrameAppend((uint8_t)(data >> 16)))
        return false;
    else if(!dataFrameAppend((uint8_t)(data >> 24)))
        return false;
    else if(!dataFrameAppend((uint8_t)(data >> 32)))
        return false;
    else if(!dataFrameAppend((uint8_t)(data >> 40)))
        return false;
    else if(!dataFrameAppend((uint8_t)(data >> 48)))
        return false;
    return dataFrameAppend((uint8_t)(data >> 56));
}

bool FDbg::dataFrameAppend(uint8_t *data, uint16_t length) {
    for(uint16_t i = 0; i < length; i++)
        if(!dataFrameAppend(data[i])) return false;
    return true;
}

bool FDbg::dataFrameAppend(const char *utf8) {
    uint16_t len = strlen(utf8);
    dataFrameAppend(len);
    return dataFrameAppend((uint8_t *)utf8, len + 1);
}

bool FDbg::dataFrameFinish(void) {
    dataFrameAppend((uint16_t)txDataCrc);
    if(txDataLength) {
        uint32_t length = txDataLength;
        txDataLength = 0;
        return sendData(txBuff, length);
    }
    return true;
}

bool FDbg::sendRespCode(DbgCmd cmd, DbgRespCode responseCode) {
    initDataFrame(cmd, responseCode, 0);
    return dataFrameFinish();
}

void FDbg::responseInfo(void) {
    initDataFrame(DBG_CMD_READ_VM_INFO, DBG_RESP_OK, 7 + sizeof(FLINT_VARIANT_NAME));

    if(!dataFrameAppend((uint8_t)FLINT_VERSION_MAJOR)) return;
    if(!dataFrameAppend((uint8_t)FLINT_VERSION_MINOR)) return;
    if(!dataFrameAppend((uint8_t)FLINT_VERSION_PATCH)) return;

    if(!dataFrameAppend((uint16_t)(sizeof(FLINT_VARIANT_NAME) - 1))) return;
    if(!dataFrameAppend((uint8_t *)FLINT_VARIANT_NAME, sizeof(FLINT_VARIANT_NAME) - 1)) return;

    dataFrameFinish();
}

void FDbg::responseStatus(void) {
    uint16_t tmp = csr | (consoleLength ? DBG_STATUS_CONSOLE : 0);
    if(tmp & (DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT))
        tmp &= ~(DBG_STATUS_STOP_SET | DBG_STATUS_STOP);
    if(!Flint::isRunning())
        tmp |= DBG_STATUS_DONE;
    initDataFrame(DBG_CMD_READ_STATUS, DBG_RESP_OK, 1);
    dataFrameAppend((uint8_t)tmp);
    if(dataFrameFinish() && (tmp & DBG_STATUS_STOP_SET)) {
        lock();
        csr &= ~DBG_STATUS_STOP_SET;
        unlock();
    }
}

void FDbg::responseStackTrace(uint32_t stackIndex) {
    if(csr & DBG_STATUS_STOP) {
        StackFrame stackTrace;
        bool isEndStack = false;
        if(exec->getStackTrace(stackIndex, &stackTrace, &isEndStack)) {
            const char *clsName = stackTrace.method->loader->getName();
            const char *name = stackTrace.method->name;
            const char *desc = stackTrace.method->desc;

            uint32_t responseSize = 8;
            responseSize += 2 + strlen(clsName) + 1;
            responseSize += 2 + strlen(name) + 1;
            responseSize += 2 + strlen(desc) + 1;

            initDataFrame(DBG_CMD_READ_STACK_TRACE, DBG_RESP_OK, responseSize);
            if(!dataFrameAppend((uint32_t)(stackIndex | (isEndStack << 31)))) return;
            if(!dataFrameAppend((uint32_t)stackTrace.pc)) return;
            if(!dataFrameAppend(clsName)) return;
            if(!dataFrameAppend(name)) return;
            if(!dataFrameAppend(desc)) return;
            dataFrameFinish();
        }
        else
            sendRespCode(DBG_CMD_READ_STACK_TRACE, DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_READ_STACK_TRACE, DBG_RESP_BUSY);
}

static uint32_t getUft8Size(JString *str) {
    uint32_t length = str->getLength();
    const char *text = str->getAscii();
    uint32_t ret = 0;
    if(str->getCoder() == 0) {
        for(uint32_t i = 0; i < length; i++)
            ret += Utf8EncodeSize(text[i]);
    }
    else {
        for(uint32_t i = 0; i < length; i++)
            ret += Utf8EncodeSize(((uint16_t *)text)[i]);
    }
    return ret;
}

void FDbg::responseExceptionInfo(void) {
    if(csr & DBG_STATUS_STOP) {
        if(exec->excp != NULL && (csr & DBG_STATUS_EXCP)) {
            const char *type = exec->excp->getTypeName();
            JString *msg = exec->excp->getDetailMessage();
            uint8_t coder = msg ? msg->getCoder() : 0;
            const char *text = msg ? msg->getAscii() : 0;
            uint32_t msgLen = msg ? msg->getLength() : 0;

            uint32_t responseSize = (2 + strlen(type) + 1) + (2 + (msg ? (getUft8Size(msg) + 1) : 0));
            char utf8Buff[3];

            initDataFrame(DBG_CMD_READ_EXCP_INFO, DBG_RESP_OK, responseSize);
            if(!dataFrameAppend(type)) return;
            if(!dataFrameAppend((uint16_t)msgLen)) return;
            if(coder == 0) for(uint32_t i = 0; i < msgLen; i++) {
                uint8_t encodeSize = Utf8EncodeOneChar(text[i], utf8Buff);
                for(uint8_t j = 0; j < encodeSize; j++)
                    if(!dataFrameAppend((uint8_t)utf8Buff[j])) return;
            }
            else for(uint32_t i = 0; i < msgLen; i++) {
                uint8_t encodeSize = Utf8EncodeOneChar(((uint16_t *)text)[i], utf8Buff);
                for(uint8_t j = 0; j < encodeSize; j++)
                    if(!dataFrameAppend((uint8_t)utf8Buff[j])) return;
            }
            if(msg) if(!dataFrameAppend((uint8_t)0)) return;
            dataFrameFinish();
        }
        else
            sendRespCode(DBG_CMD_READ_EXCP_INFO, DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_READ_EXCP_INFO, DBG_RESP_BUSY);
}

void FDbg::responseLocalVariable(uint32_t stackIndex, uint32_t localIndex, uint8_t variableType) {
    if(csr & DBG_STATUS_STOP) {
        if(variableType < 2) { /* 32 bit value or object */
            uint32_t value;
            bool isObject = !!variableType;
            if(exec->readLocal(stackIndex, localIndex, &value, &isObject)) {
                uint32_t responseSize = 8;
                uint32_t valueSize = isObject ? ((JObject *)value)->size : 4;
                if(isObject) responseSize += 2 + strlen(((JObject *)value)->getTypeName()) + 1;
                initDataFrame(DBG_CMD_READ_LOCAL, DBG_RESP_OK, responseSize);
                if(!dataFrameAppend((uint32_t)valueSize)) return;
                if(!dataFrameAppend((uint32_t)value)) return;
                if(isObject) if(!dataFrameAppend(((JObject *)value)->getTypeName())) return;
                dataFrameFinish();
            }
            else
                sendRespCode(DBG_CMD_READ_LOCAL, DBG_RESP_FAIL);
        }
        else { /* 64 bit value */
            uint64_t value;
            if(exec->readLocal(stackIndex, localIndex, &value)) {
                initDataFrame(DBG_CMD_READ_LOCAL, DBG_RESP_OK, 12);
                if(!dataFrameAppend((uint32_t)8)) return;
                if(!dataFrameAppend((uint64_t)value)) return;
                dataFrameFinish();
            }
            else
                sendRespCode(DBG_CMD_READ_LOCAL, DBG_RESP_FAIL);
        }
    }
    else
        sendRespCode(DBG_CMD_READ_LOCAL, DBG_RESP_BUSY);
}

void FDbg::responseField(JObject *obj, const char *fieldName) {
    if(csr & DBG_STATUS_STOP) {
        if(!Flint::isObject(obj))
            return (void)sendRespCode(DBG_CMD_READ_FIELD, DBG_RESP_FAIL);
        FieldValue *field = obj->getField(NULL, fieldName);
        if(field == NULL)
            return (void)sendRespCode(DBG_CMD_READ_FIELD, DBG_RESP_FAIL);
        char c = field->getFieldInfo()->desc[0];
        if(c == 'J' || c == 'D') {
            initDataFrame(DBG_CMD_READ_FIELD, DBG_RESP_OK, 12);
            if(!dataFrameAppend((uint32_t)8)) return;
            if(!dataFrameAppend((uint64_t)field->getInt64())) return;
        }
        else if(c == 'L' || c == '[') {
            JObject *subObj = field->getObj();
            if(subObj != NULL) {
                const char *type = subObj->getTypeName();
                initDataFrame(DBG_CMD_READ_FIELD, DBG_RESP_OK, 8 + (2 + strlen(type) + 1));
                if(!dataFrameAppend((uint32_t)subObj->size)) return;
                if(!dataFrameAppend((uint32_t)subObj)) return;
                if(!dataFrameAppend(type)) return;
            }
            else {
                initDataFrame(DBG_CMD_READ_FIELD, DBG_RESP_OK, 4);
                if(!dataFrameAppend((uint32_t)0)) return;
            }
        }
        else {
            initDataFrame(DBG_CMD_READ_FIELD, DBG_RESP_OK, 8);
            if(!dataFrameAppend((uint32_t)4)) return;
            if(!dataFrameAppend((uint32_t)field->getInt32())) return;
        }
        dataFrameFinish();
    }
    else sendRespCode(DBG_CMD_READ_FIELD, DBG_RESP_BUSY);
}

void FDbg::responseArray(JObject *array, uint32_t index, uint32_t length) {
    if(csr & DBG_STATUS_STOP) {
        if(Flint::isObject(array) && array->isArray()) {
            uint8_t compSz = array->type->componentSize();
            uint32_t arrayLen = ((JArray *)array)->getLength();
            uint32_t arrayEnd = index + length;
            arrayEnd = (arrayEnd < arrayLen) ? arrayEnd : arrayLen;
            if(index < arrayEnd) {
                initDataFrame(DBG_CMD_READ_ARRAY, DBG_RESP_OK, (arrayEnd - index) * compSz);
                switch(compSz) {
                    case 1:
                        for(uint32_t i = index; i < arrayEnd; i++)
                            dataFrameAppend(((uint8_t *)array->data)[i]);
                        break;
                    case 2:
                        for(uint32_t i = index; i < arrayEnd; i++)
                            dataFrameAppend(((uint16_t *)array->data)[i]);
                        break;
                    case 4:
                        for(uint32_t i = index; i < arrayEnd; i++)
                            dataFrameAppend(((uint32_t *)array->data)[i]);
                        break;
                    case 8:
                        for(uint32_t i = index; i < arrayEnd; i++)
                            dataFrameAppend(((uint64_t *)array->data)[i]);
                        break;
                }
                dataFrameFinish();
            }
            else
                sendRespCode(DBG_CMD_READ_ARRAY, DBG_RESP_FAIL);
        }
        else
            sendRespCode(DBG_CMD_READ_ARRAY, DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_READ_ARRAY, DBG_RESP_BUSY);
}

void FDbg::responseObjSizeAndType(JObject *obj) {
    if(csr & DBG_STATUS_STOP) {
        if(obj == NULL || Flint::isObject(obj) == false) {
            sendRespCode(DBG_CMD_READ_SIZE_AND_TYPE, DBG_RESP_FAIL);
            return;
        }
        const char *type = obj->getTypeName();
        initDataFrame(DBG_CMD_READ_SIZE_AND_TYPE, DBG_RESP_OK, 4 + (2 + strlen(type) + 1));
        if(!dataFrameAppend((uint32_t)obj->size)) return;
        if(!dataFrameAppend(type)) return;
        dataFrameFinish();
    }
    else
        sendRespCode(DBG_CMD_READ_SIZE_AND_TYPE, DBG_RESP_BUSY);
}

void FDbg::responseOpenFile(char *fileName, FlintAPI::IO::FileMode mode) {
    if(csr & DBG_STATUS_RESET) {
        if(fileHandle)
            FlintAPI::IO::fclose(fileHandle);
        for(uint16_t i = 0; fileName[i]; i++) {
            if((fileName[i] == '/') || (fileName[i] == '\\')) {
                fileName[i] = 0;
                if(FlintAPI::IO::finfo(fileName, NULL) != FlintAPI::IO::FILE_RESULT_OK) {
                    if(FlintAPI::IO::mkdir(fileName) != FlintAPI::IO::FILE_RESULT_OK) {
                        sendRespCode(DBG_CMD_OPEN_FILE, DBG_RESP_FAIL);
                        return;
                    }
                }
                fileName[i] = '/';
            }
        }
        fileHandle = FlintAPI::IO::fopen(fileName, mode);
        if(fileHandle)
            sendRespCode(DBG_CMD_OPEN_FILE, DBG_RESP_OK);
        else
            sendRespCode(DBG_CMD_OPEN_FILE, DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_OPEN_FILE, DBG_RESP_BUSY);
}

void FDbg::responseReadFile(uint32_t size) {
    if(csr & DBG_STATUS_RESET) {
        if(fileHandle) {
            uint32_t br = 0;
            uint8_t fileBuff[256];
            size = (size < sizeof(fileBuff)) ? size : sizeof(fileBuff);
            if(FlintAPI::IO::fread(fileHandle, fileBuff, size, &br) == FlintAPI::IO::FILE_RESULT_OK) {
                initDataFrame(DBG_CMD_READ_FILE, DBG_RESP_OK, br + sizeof(uint32_t));
                if(!dataFrameAppend((uint32_t)br)) return;
                if(!dataFrameAppend(fileBuff, size)) return;
                dataFrameFinish();
                return;
            }
        }
        sendRespCode(DBG_CMD_READ_FILE, DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_READ_FILE, DBG_RESP_BUSY);
}

void FDbg::responseWriteFile(uint8_t *data, uint32_t size) {
    if(csr & DBG_STATUS_RESET) {
        uint32_t bw = 0;
        if(
            size > 0 &&
            fileHandle &&
            FlintAPI::IO::fwrite(fileHandle, data, size, &bw) == FlintAPI::IO::FILE_RESULT_OK
        ) {
            sendRespCode(DBG_CMD_WRITE_FILE, DBG_RESP_OK);
        }
        else
            sendRespCode(DBG_CMD_WRITE_FILE, DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_WRITE_FILE, DBG_RESP_BUSY);
}

void FDbg::responseSeekFile(uint32_t offset) {
    if(csr & DBG_STATUS_RESET) {
        if(fileHandle && FlintAPI::IO::fseek(fileHandle, offset) == FlintAPI::IO::FILE_RESULT_OK)
            sendRespCode(DBG_CMD_SEEK_FILE, DBG_RESP_OK);
        else
            sendRespCode(DBG_CMD_SEEK_FILE, DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_SEEK_FILE, DBG_RESP_BUSY);
}

void FDbg::responseCloseFile(void) {
    if(csr & DBG_STATUS_RESET) {
        if(
            fileHandle &&
            FlintAPI::IO::fclose(fileHandle) == FlintAPI::IO::FILE_RESULT_OK
        ) {
            fileHandle = NULL;
            sendRespCode(DBG_CMD_CLOSE_FILE, DBG_RESP_OK);
        }
        else
            sendRespCode(DBG_CMD_CLOSE_FILE, DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_CLOSE_FILE, DBG_RESP_BUSY);
}

void FDbg::responseFileInfo(const char *fileName) {
    if(csr & DBG_STATUS_RESET) {
        FlintAPI::IO::FileInfo fileInfo;
        if(FlintAPI::IO::finfo(fileName, &fileInfo) != FlintAPI::IO::FILE_RESULT_OK)
            sendRespCode(DBG_CMD_FILE_INFO, DBG_RESP_FAIL);
        else {
            initDataFrame(DBG_CMD_FILE_INFO, DBG_RESP_OK, sizeof(fileInfo.attribute) + sizeof(fileInfo.size) + sizeof(fileInfo.time));
            if(!dataFrameAppend(fileInfo.attribute)) return;
            if(!dataFrameAppend(fileInfo.size)) return;
            if(!dataFrameAppend(fileInfo.time)) return;
            dataFrameFinish();
        }
    }
    else
        sendRespCode(DBG_CMD_FILE_INFO, DBG_RESP_BUSY);
}

void FDbg::responseCreateDelete(DbgCmd cmd, const char *path) {
    if(csr & DBG_STATUS_RESET) {
        FlintAPI::IO::FileResult ret = (cmd == DBG_CMD_DELETE_FILE) ? FlintAPI::IO::fremove(path) : FlintAPI::IO::mkdir(path);
        sendRespCode(cmd, (ret == FlintAPI::IO::FILE_RESULT_OK) ? DBG_RESP_OK : DBG_RESP_FAIL);
    }
    else
        sendRespCode(cmd, DBG_RESP_BUSY);
}

void FDbg::responseOpenDir(const char *path) {
    if(csr & DBG_STATUS_RESET) {
        if(dirHandle)
            FlintAPI::IO::closedir(dirHandle);
        dirHandle = FlintAPI::IO::opendir(path);
        sendRespCode(DBG_CMD_OPEN_DIR, dirHandle ? DBG_RESP_OK : DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_OPEN_DIR, DBG_RESP_BUSY);
}

void FDbg::responseReadDir(void) {
    if(csr & DBG_STATUS_RESET) {
        FlintAPI::IO::FileInfo fileInfo;
        if(dirHandle && FlintAPI::IO::readdir(dirHandle, &fileInfo) == FlintAPI::IO::FILE_RESULT_OK) {
            if(fileInfo.name[0] != 0) {
                uint16_t nameLen = strlen(fileInfo.name);
                uint32_t respLen = (2 + nameLen + 1) + sizeof(fileInfo.attribute) + sizeof(fileInfo.size) + sizeof(fileInfo.time);
                initDataFrame(DBG_CMD_READ_DIR, DBG_RESP_OK, respLen);
                if(!dataFrameAppend((uint16_t)nameLen)) return;
                if(!dataFrameAppend((uint8_t *)fileInfo.name, nameLen)) return;
                if(!dataFrameAppend((uint8_t)0)) return;
                if(!dataFrameAppend(fileInfo.attribute)) return;
                if(!dataFrameAppend(fileInfo.size)) return;
                if(!dataFrameAppend(fileInfo.time)) return;
                dataFrameFinish();
            }
            else
                sendRespCode(DBG_CMD_READ_DIR, DBG_RESP_OK);
        }
        else
            sendRespCode(DBG_CMD_READ_DIR, DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_READ_DIR, DBG_RESP_BUSY);
}

void FDbg::responseCloseDir(void) {
    if(csr & DBG_STATUS_RESET) {
        if(
            dirHandle &&
            FlintAPI::IO::closedir(fileHandle) == FlintAPI::IO::FILE_RESULT_OK
        ) {
            dirHandle = NULL;
            sendRespCode(DBG_CMD_CLOSE_DIR, DBG_RESP_OK);
        }
        else
            sendRespCode(DBG_CMD_CLOSE_DIR, DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_CLOSE_DIR, DBG_RESP_BUSY);
}

void FDbg::responseConsoleBuffer(void) {
    lock();
    if(consoleLength) {
        initDataFrame(DBG_CMD_READ_CONSOLE, DBG_RESP_OK, consoleLength);
        uint32_t index = (consoleOffset + sizeof(consoleBuff) - consoleLength) % sizeof(consoleBuff);
        uint32_t endIndex = consoleOffset;
        while(index != endIndex) {
            dataFrameAppend(consoleBuff[index]);
            index = (index + 1) % sizeof(consoleBuff);
        }
        dataFrameFinish();
        consoleClear();
    }
    else
        sendRespCode(DBG_CMD_READ_CONSOLE, DBG_RESP_OK);
    unlock();
}

bool FDbg::receivedDataHandler(uint8_t *data, uint32_t length) {
    DbgCmd cmd = (DbgCmd)data[0];
    uint32_t rxLength = data[1] | (data[2] << 8) | (data[3] << 16);
    if(rxLength != length) {
        sendRespCode(cmd, DBG_RESP_LENGTH_INVAILD);
        return true;
    }
    uint16_t crc = data[rxLength - 2] | (data[rxLength - 1] << 8);
    if(crc != Crc(data, rxLength - 2)) {
        sendRespCode(cmd, DBG_RESP_CRC_FAIL);
        return true;
    }
    switch(cmd) {
        case DBG_CMD_READ_VM_INFO: {
            responseInfo();
            return true;
        }
        case DBG_CMD_ENTER_DEBUG: {
            Flint::setDebugger(this);
            sendRespCode(DBG_CMD_ENTER_DEBUG, DBG_RESP_OK);
            return true;
        }
        case DBG_CMD_READ_STATUS: {
            responseStatus();
            return true;
        }
        case DBG_CMD_READ_STACK_TRACE: {
            uint32_t stackIndex = (*(uint32_t *)&data[4]) & 0x7FFFFFFF;
            responseStackTrace(stackIndex);
            return true;
        }
        case DBG_CMD_ADD_BKP:
        case DBG_CMD_REMOVE_BKP: {
            uint32_t index = 4;

            uint32_t pc = *(uint32_t *)&data[index];
            index += sizeof(pc);

            uint16_t clsNameLen = (data[index + 1] << 8) | data[index];
            index += sizeof(clsNameLen);

            const char *clsName = (char *)&data[index];
            index += clsNameLen + 1;

            uint16_t nameLen = (data[index + 1] << 8) | data[index];
            index += sizeof(nameLen);

            const char *name = (char *)&data[index];
            index += nameLen + 1;

            uint16_t descLen = (data[index + 1] << 8) | data[index];
            index += sizeof(descLen);

            const char *desc = (char *)&data[index];
            if(cmd == DBG_CMD_ADD_BKP)
                sendRespCode(DBG_CMD_ADD_BKP, addBreakPoint(pc, clsName, name, desc) ? DBG_RESP_OK : DBG_RESP_FAIL);
            else
                sendRespCode(DBG_CMD_REMOVE_BKP, removeBreakPoint(pc, clsName, name, desc) ? DBG_RESP_OK : DBG_RESP_FAIL);
            return true;
        }
        case DBG_CMD_REMOVE_ALL_BKP: {
            breakPointCount = 0;
            sendRespCode(DBG_CMD_REMOVE_ALL_BKP, DBG_RESP_OK);
            return true;
        }
        case DBG_CMD_RUN: {
            lock();
            csr &= ~(DBG_STATUS_STOP | DBG_STATUS_STOP_SET | DBG_STATUS_EXCP | DBG_CONTROL_STOP | DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT);
            exec = NULL;
            unlock();
            sendRespCode(DBG_CMD_RUN, DBG_RESP_OK);
            return true;
        }
        case DBG_CMD_STOP: {
            lock();
            csr = (csr & ~(DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT)) | DBG_CONTROL_STOP;
            exec = NULL;
            unlock();
            Flint::stopRequest();
            sendRespCode(DBG_CMD_STOP, DBG_RESP_OK);
            return true;
        }
        case DBG_CMD_RESTART: {
            const char *mainClass = (char *)&data[4 + 2];
            lock();
            csr &= DBG_CONTROL_EXCP_EN;
            exec = NULL;
            unlock();
            Flint::setDebugger(this);
            Flint::terminate();
            Flint::clearAllStaticFields();
            Flint::freeAllExecution();
            Flint::gc();
            Flint::reset();
            if(Flint::runToMain(mainClass) == true)
                sendRespCode(DBG_CMD_RESTART, DBG_RESP_OK);
            else
                sendRespCode(DBG_CMD_RESTART, DBG_RESP_FAIL);
            return true;
        }
        case DBG_CMD_TERMINATE: {
            bool endDbg = data[4] != 0;
            lock();
            csr = (csr & DBG_CONTROL_EXCP_EN) | DBG_STATUS_RESET;
            unlock();
            Flint::terminate();
            Flint::freeAll();
            Flint::reset();
            sendRespCode(DBG_CMD_TERMINATE, DBG_RESP_OK);
            return !endDbg;
        }
        case DBG_CMD_STEP_IN:
        case DBG_CMD_STEP_OVER: {
            if(csr & DBG_STATUS_STOP) {
                stepCodeLength = *(uint32_t *)&data[4];
                if(length == 10 && stepCodeLength && exec->getStackTrace(0, &startPoint, 0)) {
                    lock();
                    if(cmd == DBG_CMD_STEP_IN)
                        csr = (csr & ~(DBG_STATUS_STOP_SET | DBG_STATUS_EXCP | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT)) | DBG_CONTROL_STEP_IN;
                    else
                        csr = (csr & ~(DBG_STATUS_STOP_SET | DBG_STATUS_EXCP | DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OUT)) | DBG_CONTROL_STEP_OVER;
                    unlock();
                    sendRespCode(cmd, DBG_RESP_OK);
                }
                else
                    sendRespCode(cmd, DBG_RESP_FAIL);
            }
            else
                sendRespCode(cmd, DBG_RESP_BUSY);
            return true;
        }
        case DBG_CMD_STEP_OUT: {
            if(csr & DBG_STATUS_STOP) {
                if(exec->getStackTrace(0, &startPoint, 0)) {
                    lock();
                    csr = (csr & ~(DBG_STATUS_STOP_SET | DBG_STATUS_EXCP | DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER)) | DBG_CONTROL_STEP_OUT;
                    unlock();
                    sendRespCode(DBG_CMD_STEP_OUT, DBG_RESP_OK);
                }
                else
                    sendRespCode(DBG_CMD_STEP_OUT, DBG_RESP_FAIL);
            }
            else
                sendRespCode(DBG_CMD_STEP_OUT, DBG_RESP_BUSY);
            return true;
        }
        case DBG_CMD_SET_EXCP_MODE: {
            lock();
            if(data[4] & 0x01)
                csr |= DBG_CONTROL_EXCP_EN;
            else
                csr &= ~DBG_CONTROL_EXCP_EN;
            unlock();
            sendRespCode(DBG_CMD_SET_EXCP_MODE, DBG_RESP_OK);
            return true;
        }
        case DBG_CMD_READ_EXCP_INFO: {
            responseExceptionInfo();
            return true;
        }
        case DBG_CMD_READ_LOCAL: {
            if(length == 14) {
                uint32_t stackIndex = (*(uint32_t *)&data[4]) & 0x3FFFFFFF;
                uint32_t localIndex = (*(uint32_t *)&data[8]);
                uint8_t variableType = data[7] >> 6;
                responseLocalVariable(stackIndex, localIndex, variableType);
            }
            else
                sendRespCode(DBG_CMD_READ_LOCAL, DBG_RESP_FAIL);
            return true;
        }
        case DBG_CMD_WRITE_LOCAL: {
            // TODO
            return true;
        }
        case DBG_CMD_READ_FIELD: {
            JObject *obj = (JObject *)*(uint32_t *)&data[4];
            const char *fieldName = (char *)&data[8 + 2];
            responseField(obj, fieldName);
            return true;
        }
        case DBG_CMD_WRITE_FIELD: {
            // TODO
            return true;
        }
        case DBG_CMD_READ_ARRAY: {
            if(length == 18) {
                uint32_t length = (*(uint32_t *)&data[4]);
                uint32_t index = *(uint32_t *)&data[8];
                JObject *array = (JObject *)*(uint32_t *)&data[12];
                responseArray(array, index, length);
            }
            else
                sendRespCode(DBG_CMD_READ_ARRAY, DBG_RESP_FAIL);
            return true;
        }
        case DBG_CMD_READ_SIZE_AND_TYPE: {
            if(length == 10) {
                JObject *obj = (JObject *)*(uint32_t *)&data[4];
                responseObjSizeAndType(obj);
            }
            else
                sendRespCode(DBG_CMD_READ_SIZE_AND_TYPE, DBG_RESP_FAIL);
            return true;
        }
        case DBG_CMD_OPEN_FILE: {
            FlintAPI::IO::FileMode fileMode = (FlintAPI::IO::FileMode)data[4];
            char *fileName = (char *)&data[5 + 2];
            responseOpenFile(fileName, fileMode);
            return true;
        }
        case DBG_CMD_READ_FILE: {
            if(length == 10) {
                uint32_t size = *(uint32_t *)&data[4];
                responseReadFile(size);
            }
            else
                sendRespCode(DBG_CMD_READ_FILE, DBG_RESP_FAIL);
            return true;
        }
        case DBG_CMD_WRITE_FILE: {
            if(length > 6)
                responseWriteFile(&data[4], length - 6);
            else
                sendRespCode(DBG_CMD_WRITE_FILE, DBG_RESP_FAIL);
            return true;
        }
        case DBG_CMD_SEEK_FILE: {
            if(length == 10) {
                uint32_t offset = *(uint32_t *)&data[4];
                responseSeekFile(offset);
            }
            else
                sendRespCode(DBG_CMD_SEEK_FILE, DBG_RESP_FAIL);
            return true;
        }
        case DBG_CMD_CLOSE_FILE: {
            responseCloseFile();
            return true;
        }
        case DBG_CMD_FILE_INFO: {
            if(length >= 12) {
                const char *path = (char *)&data[4 + 2];
                responseFileInfo(path);
            }
            else
                sendRespCode(DBG_CMD_CREATE_DIR, DBG_RESP_FAIL);
            return true;
        }
        case DBG_CMD_DELETE_FILE:
        case DBG_CMD_CREATE_DIR: {
            if(length >= 12) {
                const char *path = (char *)&data[4 + 2];
                responseCreateDelete(cmd, path);
            }
            else
                sendRespCode(DBG_CMD_CREATE_DIR, DBG_RESP_FAIL);
            return true;
        }
        case DBG_CMD_OPEN_DIR: {
            if(length >= 11) {
                char *path = (char *)&data[4 + 2];
                responseOpenDir(path);
            }
            else
                sendRespCode(DBG_CMD_OPEN_DIR, DBG_RESP_FAIL);
            return true;
        }
        case DBG_CMD_READ_DIR: {
            responseReadDir();
            return true;
        }
        case DBG_CMD_CLOSE_DIR: {
            responseCloseDir();
            return true;
        }
        case DBG_CMD_READ_CONSOLE: {
            responseConsoleBuffer();
            return true;
        }
        default: {
            sendRespCode(cmd, DBG_RESP_UNKNOW);
            return true;
        }
    }
}

bool FDbg::addBreakPoint(uint32_t pc, const char *clsName, const char *name, const char *desc) {
    if(breakPointCount < LENGTH(breakPoints)) {
        ClassLoader *loader = Flint::findLoader(NULL, clsName);
        if(loader == NULL) return false;
        MethodInfo *method = loader->getMethodInfo(NULL, name, desc);
        if(method == NULL) return false;
        if(method->accessFlag & METHOD_NATIVE) return false;
        uint8_t *code = method->getCode();
        for(uint8_t i = 0; i < breakPointCount; i++) {
            if(method == breakPoints[i].method && pc == breakPoints[i].pc) {
                code[pc] = (uint8_t)OP_BREAKPOINT;
                return true;
            }
        }
        new (&breakPoints[breakPointCount])BreakPoint(code[pc], pc, method);
        breakPointCount++;
        code[pc] = (uint8_t)OP_BREAKPOINT;
        return true;
    }
    return false;
}

uint8_t FDbg::getSavedOpcode(uint32_t pc, MethodInfo *method) {
    for(uint8_t i = 0; i < breakPointCount; i++) {
        if(method == breakPoints[i].method && pc == breakPoints[i].pc)
            return breakPoints[i].opcode;
    }
    return OP_UNKNOW;
}

bool FDbg::removeBreakPoint(uint32_t pc, const char *clsName, const char *name, const char *desc) {
    if(breakPointCount) {
        ClassLoader *loader = Flint::findLoader(NULL, clsName);
        if(loader == NULL) return false;
        MethodInfo *method = loader->getMethodInfo(NULL, name, desc);
        if(method == NULL) return false;
        for(uint8_t i = 0; i < breakPointCount; i++) {
            if(method == breakPoints[i].method && pc == breakPoints[i].pc) {
                method->getCode()[breakPoints[i].pc] = breakPoints[i].opcode;
                breakPoints[i] = breakPoints[breakPointCount - 1];
                breakPointCount--;
            }
        }
        return true;
    }
    return false;
}

bool FDbg::exceptionIsEnabled(void) {
    return (csr & DBG_CONTROL_EXCP_EN) == DBG_CONTROL_EXCP_EN;
}

bool FDbg::checkStop(FExec *exec) {
    if((csr & DBG_CONTROL_STOP) && (this->exec == NULL)) {
        lock();
        if(this->exec == NULL) {
            this->exec = exec;
            csr = (csr | DBG_STATUS_STOP | DBG_STATUS_STOP_SET) & ~(DBG_CONTROL_STOP | DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT);
        }
        unlock();
    }
    return waitStop(exec);
}

void FDbg::caughtException(FExec *exec) {
    lock();
    uint16_t tmp = csr & ~(DBG_CONTROL_STOP | DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT);
    tmp |= DBG_STATUS_STOP | DBG_STATUS_STOP_SET | DBG_STATUS_EXCP;
    csr = tmp;
    this->exec = exec;
    unlock();
    Flint::stopRequest();
    waitStop(exec);
}

void FDbg::hitBreakpoint(FExec *exec) {
    lock();
    csr = (csr | DBG_STATUS_STOP | DBG_STATUS_STOP_SET) & ~(DBG_CONTROL_STOP | DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT);
    this->exec = exec;
    unlock();
    Flint::stopRequest();
    waitStop(exec);
}

bool FDbg::waitStop(FExec *exec) {
    while(csr & (DBG_STATUS_STOP | DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT | DBG_STATUS_EXCP)) {
        uint16_t tmp = csr;
        if(this->exec == exec) {
            static uint32_t oldPc = 0;
            if(tmp & (DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT)) {
                if(tmp & DBG_STATUS_STOP) {
                    lock();
                    csr &= ~(DBG_STATUS_STOP | DBG_STATUS_STOP_SET);
                    unlock();
                    oldPc = exec->pc;
                    return true;
                }
                bool isStopped = false;
                if(
                    (tmp & DBG_CONTROL_STEP_IN) &&
                    (startPoint.method != exec->method || (exec->pc - startPoint.pc) >= stepCodeLength || exec->pc <= oldPc)
                ) {
                    isStopped = true;
                }
                else if(
                    (tmp & DBG_CONTROL_STEP_OVER) &&
                    (exec->startSp <= startPoint.baseSp) &&
                    (startPoint.method != exec->method || (exec->pc - startPoint.pc) >= stepCodeLength || exec->pc <= oldPc)
                ) {
                    isStopped = true;
                }
                else if(exec->startSp < startPoint.baseSp)
                    isStopped = true;

                if(exec->pc > oldPc)
                    oldPc = exec->pc;

                if(isStopped) {
                    if(exec->code[exec->pc] == OP_BREAKPOINT)
                        return false;
                    Flint::stopRequest();
                    lock();
                    csr = (csr & ~(DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OUT)) | DBG_STATUS_STOP | DBG_STATUS_STOP_SET;
                    unlock();
                }
                else
                    return true;
            }
        }
        else if(!(tmp & DBG_STATUS_STOP))
            break;
        FlintAPI::Thread::yield();
    }
    return (csr & DBG_CONTROL_STOP) ? true : false;
}

void FDbg::lock(void) {
    dbgMutex.lock();
}

void FDbg::unlock(void) {
    dbgMutex.unlock();
}
