
#include <iostream>
#include <string.h>
#include "flint.h"
#include "flint_debugger.h"

FlintBreakPoint::FlintBreakPoint(void) : pc(0), method(0) {

}

FlintBreakPoint::FlintBreakPoint(uint32_t pc, FlintMethodInfo &method) : pc(pc), method(&method) {

}

FlintStackFrame::FlintStackFrame(void) : pc(0), baseSp(0), method(*(FlintMethodInfo *)0) {

}

FlintStackFrame::FlintStackFrame(uint32_t pc, uint32_t baseSp, FlintMethodInfo &method) : pc(pc), baseSp(baseSp), method(method) {

}

FlintDebugger::FlintDebugger(Flint &flint) : flint(flint) {
    dbgLockHandle = FlintAPI::Thread::createLockHandle();
    execution = 0;
    exception = 0;
    dirHandle = 0;
    fileHandle = 0;
    stepCodeLength = 0;
    consoleOffset = 0;
    consoleLength = 0;
    csr = DBG_STATUS_RESET;
    breakPointCount = 0;
    txDataLength = 0;
}

void FlintDebugger::print(const char *text, uint32_t length, uint8_t coder) {
    lock();
    if(coder == 0) {
        while(length) {
            consolePut((uint8_t)*text);
            text++;
            length--;
        }
    }
    else {
        while(length) {
            consolePut(*(uint16_t *)text);
            text += 2;
            length--;
        }
    }
    unlock();
}

void FlintDebugger::consolePut(uint16_t ch) {
    char buff[3];
    uint8_t count = FlintJavaString::utf8Encode(ch, buff);
    for(uint8_t i = 0; i < count; i++) {
        uint32_t nextOffset = (consoleOffset + 1) % sizeof(consoleBuff);
        if(consoleLength == sizeof(consoleBuff))
            consoleLength -= FlintJavaString::getUtf8EncodeSize(consoleBuff[nextOffset]);
        consoleBuff[consoleOffset] = buff[i];
        consoleOffset = nextOffset;
        if(consoleLength < sizeof(consoleBuff))
            consoleLength++;
    }
}

void FlintDebugger::consoleClear(void) {
    consoleOffset = 0;
    consoleLength = 0;
}

void FlintDebugger::clearTxBuffer(void) {
    txDataLength = 0;
}

void FlintDebugger::initDataFrame(FlintDbgCmd cmd, FlintDbgRespCode responseCode, uint32_t dataLength) {
    dataLength += 7;
    txBuff[0] = (uint8_t)(cmd | 0x80);
    txBuff[1] = (uint8_t)(dataLength >> 0);
    txBuff[2] = (uint8_t)(dataLength >> 8);
    txBuff[3] = (uint8_t)(dataLength >> 16);
    txBuff[4] = (uint8_t)responseCode;
    txDataLength = 5;
    txDataCrc = txBuff[0] + txBuff[1] + txBuff[2] + txBuff[3] + txBuff[4];
}

bool FlintDebugger::dataFrameAppend(uint8_t data) {
    if(txDataLength == sizeof(txBuff)) {
        if(!sendData(txBuff, sizeof(txBuff)))
            return false;
        txDataLength = 0;
    }
    txBuff[txDataLength++] = data;
    txDataCrc += data;
    return true;
}

bool FlintDebugger::dataFrameAppend(uint16_t data) {
    if(!dataFrameAppend((uint8_t)data))
        return false;
    return dataFrameAppend((uint8_t)(data >> 8));
}

bool FlintDebugger::dataFrameAppend(uint32_t data) {
    if(!dataFrameAppend((uint8_t)data))
        return false;
    else if(!dataFrameAppend((uint8_t)(data >> 8)))
        return false;
    else if(!dataFrameAppend((uint8_t)(data >> 16)))
        return false;
    return dataFrameAppend((uint8_t)(data >> 24));
}

bool FlintDebugger::dataFrameAppend(uint64_t data) {
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

bool FlintDebugger::dataFrameAppend(uint8_t *data, uint16_t length) {
    for(uint16_t i = 0; i < length; i++) {
        if(!dataFrameAppend(data[i]))
            return false;
    }
    return true;
}

bool FlintDebugger::dataFrameAppend(const FlintConstUtf8 &utf8) {
    uint32_t size = sizeof(FlintConstUtf8) + utf8.length + 1;
    uint8_t *buff = (uint8_t *)&utf8;
    for(uint32_t i = 0; i < size; i++) {
        if(!dataFrameAppend(buff[i]))
            return false;
    }
    return true;
}

bool FlintDebugger::dataFrameFinish(void) {
    dataFrameAppend((uint16_t)txDataCrc);
    if(txDataLength) {
        uint32_t length = txDataLength;
        txDataLength = 0;
        return sendData(txBuff, length);
    }
    return true;
}

bool FlintDebugger::sendRespCode(FlintDbgCmd cmd, FlintDbgRespCode responseCode) {
    initDataFrame(cmd, responseCode, 0);
    return dataFrameFinish();
}

void FlintDebugger::responseInfo(void) {
    initDataFrame(DBG_CMD_READ_VM_INFO, DBG_RESP_OK, 7 + sizeof(FLINT_VARIANT_NAME));

    if(!dataFrameAppend((uint8_t)FLINT_VERSION_MAJOR)) return;
    if(!dataFrameAppend((uint8_t)FLINT_VERSION_MINOR)) return;
    if(!dataFrameAppend((uint8_t)FLINT_VERSION_PATCH)) return;

    if(!dataFrameAppend((uint16_t)(sizeof(FLINT_VARIANT_NAME) - 1))) return;
    if(!dataFrameAppend((uint16_t)0)) return;
    if(!dataFrameAppend((uint16_t)0)) return;
    if(!dataFrameAppend((uint8_t *)FLINT_VARIANT_NAME, sizeof(FLINT_VARIANT_NAME) - 1)) return;
    if(!dataFrameAppend((uint8_t)0)) return;

    dataFrameFinish();
}

void FlintDebugger::responseStatus(void) {
    uint16_t tmp = csr | (consoleLength ? DBG_STATUS_CONSOLE : 0);
    if(tmp & (DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT))
        tmp &= ~(DBG_STATUS_STOP_SET | DBG_STATUS_STOP);
    if(!flint.isRunning())
        tmp |= DBG_STATUS_DONE;
    initDataFrame(DBG_CMD_READ_STATUS, DBG_RESP_OK, 1);
    dataFrameAppend((uint8_t)tmp);
    if(dataFrameFinish() && (tmp & DBG_STATUS_STOP_SET)) {
        lock();
        csr &= ~DBG_STATUS_STOP_SET;
        unlock();
    }
}

void FlintDebugger::responseStackTrace(uint32_t stackIndex) {
    if(csr & DBG_STATUS_STOP) {
        FlintStackFrame stackTrace;
        bool isEndStack = false;
        if(execution->getStackTrace(stackIndex, &stackTrace, &isEndStack)) {
            FlintMethodInfo &method = stackTrace.method;
            FlintConstUtf8 &className = method.classLoader.getThisClass();

            uint32_t responseSize = 8;
            responseSize += sizeof(FlintConstUtf8) + className.length + 1;
            responseSize += sizeof(FlintConstUtf8) + method.name.length + 1;
            responseSize += sizeof(FlintConstUtf8) + method.descriptor.length + 1;

            initDataFrame(DBG_CMD_READ_STACK_TRACE, DBG_RESP_OK, responseSize);
            if(!dataFrameAppend((uint32_t)(stackIndex | (isEndStack << 31)))) return;
            if(!dataFrameAppend((uint32_t)stackTrace.pc)) return;
            if(!dataFrameAppend(className)) return;
            if(!dataFrameAppend(method.name)) return;
            if(!dataFrameAppend(method.descriptor)) return;
            dataFrameFinish();
        }
        else
            sendRespCode(DBG_CMD_READ_STACK_TRACE, DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_READ_STACK_TRACE, DBG_RESP_BUSY);
}

void FlintDebugger::responseExceptionInfo(void) {
    if(csr & DBG_STATUS_STOP) {
        if(
            exception &&
            (csr & DBG_STATUS_EXCP) &&
            flint.isInstanceof(exception, *(FlintConstUtf8 *)&throwableClassName)
        ) {
            FlintConstUtf8 &type = exception->type;
            FlintJavaString *str = exception->getDetailMessage();
            uint32_t responseSize = sizeof(FlintConstUtf8) * 2 + type.length + (str ? str->getUft8BuffSize() : 0) + 2;
            uint8_t coder = str ? str->getCoder() : 0;
            const char *text = str ? str->getText() : 0;
            uint32_t msgLen = str ? str->getLength() : 0;
            char utf8Buff[3];

            initDataFrame(DBG_CMD_READ_EXCP_INFO, DBG_RESP_OK, responseSize);
            if(!dataFrameAppend(type)) return;
            if(!dataFrameAppend((uint16_t)msgLen)) return;
            if(!dataFrameAppend((uint16_t)0)) return;
            if(msgLen) {
                if(coder == 0) for(uint32_t i = 0; i < msgLen; i++) {
                    uint8_t encodeSize = FlintJavaString::utf8Encode(text[i], utf8Buff);
                    for(uint8_t j = 0; j < encodeSize; j++)
                        if(!dataFrameAppend((uint8_t)utf8Buff[j])) return;
                }
                else for(uint32_t i = 0; i < msgLen; i++) {
                    uint8_t encodeSize = FlintJavaString::utf8Encode(((uint16_t *)text)[i], utf8Buff);
                    for(uint8_t j = 0; j < encodeSize; j++)
                        if(!dataFrameAppend((uint8_t)utf8Buff[j])) return;
                }
            }
            if(!dataFrameAppend((uint8_t)0)) return;
            dataFrameFinish();
        }
        else
            sendRespCode(DBG_CMD_READ_EXCP_INFO, DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_READ_EXCP_INFO, DBG_RESP_BUSY);
}

void FlintDebugger::responseLocalVariable(bool isU64, uint32_t stackIndex, uint32_t localIndex) {
    if(csr & DBG_STATUS_STOP) {
        if(!isU64) {
            uint32_t value;
            bool isObject;
            if(execution->readLocal(stackIndex, localIndex, value, isObject)) {
                uint32_t responseSize = 8;
                uint32_t valueSize = 4;
                if(isObject) {
                    FlintJavaObject &obj = *(FlintJavaObject *)value;
                    FlintConstUtf8 &type = obj.type;
                    uint8_t isPrim = obj.isPrimType(type);
                    valueSize = obj.size;
                    responseSize += sizeof(FlintConstUtf8) + obj.dimensions + (isPrim ? 0 : 2) + type.length + 1;
                }
                initDataFrame(DBG_CMD_READ_LOCAL, DBG_RESP_OK, responseSize);
                if(!dataFrameAppend((uint32_t)valueSize)) return;
                if(!dataFrameAppend((uint32_t)value)) return;

                if(isObject) {
                    FlintJavaObject &obj = *(FlintJavaObject *)value;
                    FlintConstUtf8 &type = obj.type;
                    uint8_t isPrim = obj.isPrimType(type);
                    uint16_t typeLength = obj.dimensions + (isPrim ? 0 : 2) + type.length;
                    if(!dataFrameAppend((uint16_t)typeLength)) return;
                    if(!dataFrameAppend((uint16_t)0)) return;
                    for(uint32_t i = 0; i < obj.dimensions; i++)
                        if(!dataFrameAppend((uint8_t)'[')) return;
                    if(!isPrim)
                        if(!dataFrameAppend((uint8_t)'L')) return;
                    if(!dataFrameAppend((uint8_t *)type.text, type.length)) return;
                    if(!isPrim)
                        if(!dataFrameAppend((uint8_t)';')) return;
                    if(!dataFrameAppend((uint8_t)0)) return;
                }
                dataFrameFinish();
            }
            else
                sendRespCode(DBG_CMD_READ_LOCAL, DBG_RESP_FAIL);
        }
        else {
            uint64_t value;
            if(execution->readLocal(stackIndex, localIndex, value)) {
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

void FlintDebugger::responseField(FlintJavaObject *obj, const FlintConstUtf8 &fieldName) {
    if(csr & DBG_STATUS_STOP) {
        if(!obj) {
            sendRespCode(DBG_CMD_READ_FIELD, DBG_RESP_FAIL);
            return;
        }
        uint8_t fieldType = 0;
        void *fieldData = (void *)&obj->getFields().getFieldData32(fieldName);
        if(!fieldData) {
            fieldType = 1;
            fieldData = (void *)&obj->getFields().getFieldData64(fieldName);
            if(!fieldData) {
                fieldType = 2;
                fieldData = (void *)&obj->getFields().getFieldObject(fieldName);
            }
        }
        if(fieldData) {
            if(fieldType == 0) {
                initDataFrame(DBG_CMD_READ_FIELD, DBG_RESP_OK, 8);
                if(!dataFrameAppend((uint32_t)4)) return;
                if(!dataFrameAppend((uint32_t)((FlintFieldData32 *)fieldData)->value)) return;
            }
            else if(fieldType == 1) {
                initDataFrame(DBG_CMD_READ_FIELD, DBG_RESP_OK, 12);
                if(!dataFrameAppend((uint32_t)8)) return;
                if(!dataFrameAppend((uint64_t)((FlintFieldData64 *)fieldData)->value)) return;
            }
            else {
                FlintJavaObject *subObj = ((FlintFieldObject *)fieldData)->object;
                if(subObj) {
                    FlintConstUtf8 &type = subObj->type;
                    uint8_t isPrim = subObj->isPrimType(type);
                    uint16_t typeLength = subObj->dimensions + (isPrim ? 0 : 2) + type.length;
                    uint32_t responseSize = 8 + sizeof(FlintConstUtf8) + typeLength + 1;

                    initDataFrame(DBG_CMD_READ_FIELD, DBG_RESP_OK, responseSize);
                    if(!dataFrameAppend((uint32_t)subObj->size)) return;
                    if(!dataFrameAppend((uint32_t)((FlintFieldObject *)fieldData)->object)) return;
                    if(!dataFrameAppend((uint16_t)typeLength)) return;
                    if(!dataFrameAppend((uint16_t)0)) return;
                    for(uint32_t i = 0; i < subObj->dimensions; i++)
                        if(!dataFrameAppend((uint8_t)'[')) return;
                    if(!isPrim)
                        if(!dataFrameAppend((uint8_t)'L')) return;
                    if(!dataFrameAppend((uint8_t *)type.text, type.length)) return;
                    if(!isPrim)
                        if(!dataFrameAppend((uint8_t)';')) return;
                    if(!dataFrameAppend((uint8_t)0)) return;
                }
                else {
                    initDataFrame(DBG_CMD_READ_FIELD, DBG_RESP_OK, 4);
                    if(!dataFrameAppend((uint32_t)0)) return;
                }
            }
            dataFrameFinish();
        }
    }
    else
        sendRespCode(DBG_CMD_READ_FIELD, DBG_RESP_BUSY);
}

void FlintDebugger::responseArray(FlintJavaObject *array, uint32_t index, uint32_t length) {
    if(csr & DBG_STATUS_STOP) {
        if(array && array->dimensions > 0) {
            uint8_t atype = FlintJavaObject::isPrimType(array->type);
            uint8_t elementSize = atype ? FlintJavaObject::getPrimitiveTypeSize(atype) : sizeof(FlintJavaObject *);
            uint32_t arrayLength = array->size / elementSize;
            uint32_t arrayEnd = index + length;
            arrayEnd = (arrayEnd < arrayLength) ? arrayEnd : arrayLength;
            if(index < arrayEnd) {
                initDataFrame(DBG_CMD_READ_ARRAY, DBG_RESP_OK, (arrayEnd - index) * elementSize);
                switch(elementSize) {
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

void FlintDebugger::responseObjSizeAndType(FlintJavaObject *obj) {
    if(csr & DBG_STATUS_STOP) {
        if(!obj) {
            sendRespCode(DBG_CMD_READ_SIZE_AND_TYPE, DBG_RESP_FAIL);
            return;
        }
        FlintConstUtf8 &type = obj->type;
        uint8_t isPrim = obj->isPrimType(type);
        uint16_t typeLength = obj->dimensions + (isPrim ? 0 : 2) + type.length;
        uint32_t responseSize = 4 + sizeof(FlintConstUtf8) + typeLength + 1;

        initDataFrame(DBG_CMD_READ_SIZE_AND_TYPE, DBG_RESP_OK, responseSize);
        if(!dataFrameAppend((uint32_t)obj->size)) return;
        if(!dataFrameAppend((uint16_t)typeLength)) return;
        if(!dataFrameAppend((uint16_t)0)) return;
        for(uint32_t i = 0; i < obj->dimensions; i++)
            if(!dataFrameAppend((uint8_t)'[')) return;
        if(!isPrim)
            if(!dataFrameAppend((uint8_t)'L')) return;
        if(!dataFrameAppend((uint8_t *)type.text, type.length)) return;
        if(!isPrim)
            if(!dataFrameAppend((uint8_t)';')) return;
        if(!dataFrameAppend((uint8_t)0)) return;
        dataFrameFinish();
    }
    else
        sendRespCode(DBG_CMD_READ_SIZE_AND_TYPE, DBG_RESP_BUSY);
}

void FlintDebugger::responseOpenFile(char *fileName, FlintFileMode mode) {
    if(csr & DBG_STATUS_RESET) {
        if(fileHandle)
            FlintAPI::IO::fclose(fileHandle);
        for(uint16_t i = 0; fileName[i]; i++) {
            if((fileName[i] == '/') || (fileName[i] == '\\')) {
                fileName[i] = 0;
                if(FlintAPI::IO::finfo(fileName, NULL, NULL) != FILE_RESULT_OK) {
                    if(FlintAPI::IO::mkdir(fileName) != FILE_RESULT_OK) {
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

void FlintDebugger::responseReadFile(uint32_t size) {
    if(csr & DBG_STATUS_RESET) {
        if(fileHandle) {
            uint32_t br = 0;
            size = (size < sizeof(fileBuff)) ? size : sizeof(fileBuff);
            if(FlintAPI::IO::fread(fileHandle, fileBuff, size, &br) == FILE_RESULT_OK) {
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

void FlintDebugger::responseWriteFile(uint8_t *data, uint32_t size) {
    if(csr & DBG_STATUS_RESET) {
        uint32_t bw = 0;
        if(
            size > 0 &&
            fileHandle &&
            FlintAPI::IO::fwrite(fileHandle, data, size, &bw) == FILE_RESULT_OK
        ) {
            sendRespCode(DBG_CMD_WRITE_FILE, DBG_RESP_OK);
        }
        else
            sendRespCode(DBG_CMD_WRITE_FILE, DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_WRITE_FILE, DBG_RESP_BUSY);
}

void FlintDebugger::responseSeekFile(uint32_t offset) {
    if(csr & DBG_STATUS_RESET) {
        if(fileHandle && FlintAPI::IO::fseek(fileHandle, offset) == FILE_RESULT_OK)
            sendRespCode(DBG_CMD_SEEK_FILE, DBG_RESP_OK);
        else
            sendRespCode(DBG_CMD_SEEK_FILE, DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_SEEK_FILE, DBG_RESP_BUSY);
}

void FlintDebugger::responseCloseFile(void) {
    if(csr & DBG_STATUS_RESET) {
        if(
            fileHandle &&
            FlintAPI::IO::fclose(fileHandle) == FILE_RESULT_OK
        ) {
            fileHandle = 0;
            sendRespCode(DBG_CMD_CLOSE_FILE, DBG_RESP_OK);
        }
        else
            sendRespCode(DBG_CMD_CLOSE_FILE, DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_CLOSE_FILE, DBG_RESP_BUSY);
}

void FlintDebugger::responseFileInfo(const char *fileName) {
    if(csr & DBG_STATUS_RESET) {
        uint32_t size;
        int64_t time;
        FlintFileResult ret = FlintAPI::IO::finfo(fileName, &size, &time);
        if(ret != FILE_RESULT_OK)
            sendRespCode(DBG_CMD_FILE_INFO, DBG_RESP_FAIL);
        else {
            initDataFrame(DBG_CMD_FILE_INFO, DBG_RESP_OK, sizeof(size) + sizeof(time));
            if(!dataFrameAppend(size)) return;
            if(!dataFrameAppend((uint64_t)time)) return;
            dataFrameFinish();
        }
    }
    else
        sendRespCode(DBG_CMD_FILE_INFO, DBG_RESP_BUSY);
}

void FlintDebugger::responseCreateDelete(FlintDbgCmd cmd, const char *path) {
    if(csr & DBG_STATUS_RESET) {
        FlintFileResult ret;
        if(cmd == DBG_CMD_DELETE_FILE)
            ret = FlintAPI::IO::fremove(path);
        else
            ret = FlintAPI::IO::mkdir(path);
        sendRespCode(cmd, (ret == FILE_RESULT_OK) ? DBG_RESP_OK : DBG_RESP_FAIL);
    }
    else
        sendRespCode(cmd, DBG_RESP_BUSY);
}

void FlintDebugger::responseOpenDir(const char *path) {
    if(csr & DBG_STATUS_RESET) {
        if(dirHandle)
            FlintAPI::IO::closedir(dirHandle);
        dirHandle = FlintAPI::IO::opendir(path);
        sendRespCode(DBG_CMD_OPEN_DIR, dirHandle ? DBG_RESP_OK : DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_OPEN_DIR, DBG_RESP_BUSY);
}

void FlintDebugger::responseReadDir(void) {
    if(csr & DBG_STATUS_RESET) {
        if(dirHandle) {
            uint8_t attribute;
            uint32_t size = 0;
            int64_t time = 0;
            if(FlintAPI::IO::readdir(dirHandle, &attribute, (char *)fileBuff, sizeof(fileBuff), &size, &time) == FILE_RESULT_OK) {
                uint16_t nameLength = strlen((char *)fileBuff);
                uint32_t respLength = 6 + nameLength + sizeof(size) + sizeof(time);
                initDataFrame(DBG_CMD_READ_DIR, DBG_RESP_OK, respLength);
                if(!dataFrameAppend((uint8_t)attribute)) return;
                if(!dataFrameAppend((uint16_t)nameLength)) return;
                if(!dataFrameAppend((uint16_t)0)) return;
                for(uint32_t i = 0; i < nameLength; i++)
                    if(!dataFrameAppend((uint8_t)fileBuff[i])) return;
                if(!dataFrameAppend((uint8_t)0)) return;
                if(!dataFrameAppend(size)) return;
                if(!dataFrameAppend((uint64_t)time)) return;
                dataFrameFinish();
            }
            else
                sendRespCode(DBG_CMD_READ_DIR, DBG_RESP_OK);
        }
    }
    else
        sendRespCode(DBG_CMD_READ_DIR, DBG_RESP_BUSY);
}

void FlintDebugger::responseCloseDir(void) {
    if(csr & DBG_STATUS_RESET) {
        if(
            dirHandle &&
            FlintAPI::IO::closedir(fileHandle) == FILE_RESULT_OK
        ) {
            dirHandle = 0;
            sendRespCode(DBG_CMD_CLOSE_DIR, DBG_RESP_OK);
        }
        else
            sendRespCode(DBG_CMD_CLOSE_DIR, DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_CLOSE_DIR, DBG_RESP_BUSY);
}

void FlintDebugger::responseConsoleBuffer(void) {
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

bool FlintDebugger::receivedDataHandler(uint8_t *data, uint32_t length) {
    FlintDbgCmd cmd = (FlintDbgCmd)data[0];
    uint32_t rxLength = data[1] | (data[2] << 8) | (data[3] << 16);
    if(rxLength != length) {
        sendRespCode(cmd, DBG_RESP_LENGTH_INVAILD);
        return true;
    }
    uint16_t crc = data[rxLength - 2] | (data[rxLength - 1] << 8);
    if(crc != Flint_CalcCrc(data, rxLength - 2)) {
        sendRespCode(cmd, DBG_RESP_CRC_FAIL);
        return true;
    }
    switch(cmd) {
        case DBG_CMD_READ_VM_INFO: {
            responseInfo();
            return true;
        }
        case DBG_CMD_ENTER_DEBUG: {
            flint.setDebugger(this);
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
            index += sizeof(uint32_t);
            FlintConstUtf8 &className = *(FlintConstUtf8 *)&data[index];
            index += sizeof(FlintConstUtf8) + className.length + 1;
            FlintConstUtf8 &methodName = *(FlintConstUtf8 *)&data[index];
            index += sizeof(FlintConstUtf8) + methodName.length + 1;
            FlintConstUtf8 &descriptor = *(FlintConstUtf8 *)&data[index];
            if(cmd == DBG_CMD_ADD_BKP)
                sendRespCode(DBG_CMD_ADD_BKP, addBreakPoint(pc, className, methodName, descriptor) ? DBG_RESP_OK : DBG_RESP_FAIL);
            else
                sendRespCode(DBG_CMD_REMOVE_BKP, removeBreakPoint(pc, className, methodName, descriptor) ? DBG_RESP_OK : DBG_RESP_FAIL);
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
            unlock();
            sendRespCode(DBG_CMD_RUN, DBG_RESP_OK);
            return true;
        }
        case DBG_CMD_STOP: {
            lock();
            csr = (csr & ~(DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT)) | DBG_CONTROL_STOP;
            unlock();
            sendRespCode(DBG_CMD_STOP, DBG_RESP_OK);
            return true;
        }
        case DBG_CMD_RESTART: {
            FlintConstUtf8 *mainClass = (FlintConstUtf8 *)&data[4];
            lock();
            csr &= DBG_CONTROL_EXCP_EN;
            unlock();
            flint.setDebugger(this);
            flint.terminate();
            flint.clearAllStaticFields();
            flint.freeAllExecution();
            flint.garbageCollection();
            flint.reset();
            try {
                flint.runToMain(mainClass->text);
                sendRespCode(DBG_CMD_RESTART, DBG_RESP_OK);
            }
            catch(...) {
                sendRespCode(DBG_CMD_RESTART, DBG_RESP_FAIL);
            }
            return true;
        }
        case DBG_CMD_TERMINATE: {
            bool endDbg = data[4] != 0;
            lock();
            csr = (csr & DBG_CONTROL_EXCP_EN) | DBG_STATUS_RESET;
            unlock();
            flint.terminate();
            flint.freeAll();
            flint.reset();
            sendRespCode(DBG_CMD_TERMINATE, DBG_RESP_OK);
            return !endDbg;
        }
        case DBG_CMD_STEP_IN:
        case DBG_CMD_STEP_OVER: {
            if(csr & DBG_STATUS_STOP && length == 10) {
                stepCodeLength = *(uint32_t *)&data[4];
                if(stepCodeLength && execution->getStackTrace(0, &startPoint, 0)) {
                    if(cmd == DBG_CMD_STEP_IN) {
                        lock();
                        csr = (csr & ~(DBG_STATUS_STOP_SET | DBG_STATUS_EXCP | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT)) | DBG_CONTROL_STEP_IN;
                        unlock();
                    }
                    else {
                        lock();
                        csr = (csr & ~(DBG_STATUS_STOP_SET | DBG_STATUS_EXCP | DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OUT)) | DBG_CONTROL_STEP_OVER;
                        unlock();
                    }
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
                if(execution->getStackTrace(0, &startPoint, 0)) {
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
                uint32_t stackIndex = (*(uint32_t *)&data[4]) & 0x7FFFFFFF;
                uint32_t localIndex = (*(uint32_t *)&data[8]);
                bool isU64 = (data[7] & 0x80) ? true : false;
                responseLocalVariable(isU64, stackIndex, localIndex);
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
            FlintJavaObject *obj = (FlintJavaObject *)*(uint32_t *)&data[4];
            FlintConstUtf8 &fieldName = *(FlintConstUtf8 *)&data[8];
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
                FlintJavaObject *array = (FlintJavaObject *)*(uint32_t *)&data[12];
                responseArray(array, index, length);
            }
            else
                sendRespCode(DBG_CMD_READ_ARRAY, DBG_RESP_FAIL);
            return true;
        }
        case DBG_CMD_READ_SIZE_AND_TYPE: {
            if(length == 10) {
                FlintJavaObject *obj = (FlintJavaObject *)*(uint32_t *)&data[4];
                responseObjSizeAndType(obj);
            }
            else
                sendRespCode(DBG_CMD_READ_SIZE_AND_TYPE, DBG_RESP_FAIL);
            return true;
        }
        case DBG_CMD_OPEN_FILE: {
            FlintFileMode fileMode = (FlintFileMode)data[4];
            char *fileName = (char *)((FlintConstUtf8 *)&data[5])->text;
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
                const char *path = (const char *)((FlintConstUtf8 *)&data[4])->text;
                responseFileInfo(path);
            }
            else
                sendRespCode(DBG_CMD_CREATE_DIR, DBG_RESP_FAIL);
            return true;
        }
        case DBG_CMD_DELETE_FILE:
        case DBG_CMD_CREATE_DIR: {
            if(length >= 12) {
                const char *path = (const char *)((FlintConstUtf8 *)&data[4])->text;
                responseCreateDelete(cmd, path);
            }
            else
                sendRespCode(DBG_CMD_CREATE_DIR, DBG_RESP_FAIL);
            return true;
        }
        case DBG_CMD_OPEN_DIR: {
            if(length >= 11) {
                char *path = (char *)((FlintConstUtf8 *)&data[4])->text;
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

bool FlintDebugger::addBreakPoint(uint32_t pc, const FlintConstUtf8 &className, const FlintConstUtf8 &methodName, const FlintConstUtf8 &descriptor) {
    try {
        if(breakPointCount < LENGTH(breakPoints)) {
            FlintClassLoader &loader = flint.load(className);
            FlintMethodInfo *method = &loader.getMethodInfo(methodName, descriptor);
            if(method) {
                for(uint8_t i = 0; i < breakPointCount; i++) {
                    if(method == breakPoints[i].method && pc == breakPoints[i].pc)
                        return true;
                }
                new (&breakPoints[breakPointCount])FlintBreakPoint(pc, *method);
                breakPointCount++;
                return true;
            }
        }
    }
    catch(FlintLoadFileError *file) {

    }
    return false;
}

bool FlintDebugger::removeBreakPoint(uint32_t pc, const FlintConstUtf8 &className, const FlintConstUtf8 &methodName, const FlintConstUtf8 &descriptor) {
    try {
        if(breakPointCount) {
            FlintClassLoader &loader = flint.load(className);
            FlintMethodInfo *method = &loader.getMethodInfo(methodName, descriptor);
            if(method) {
                for(uint8_t i = 0; i < breakPointCount; i++) {
                    if(method == breakPoints[i].method && pc == breakPoints[i].pc) {
                        breakPoints[i] = breakPoints[breakPointCount - 1];
                        breakPointCount--;
                    }
                }
                return true;
            }
        }
    }
    catch(FlintLoadFileError *file) {

    }
    return false;
}

bool FlintDebugger::exceptionIsEnabled(void) {
    return (csr & DBG_CONTROL_EXCP_EN) == DBG_CONTROL_EXCP_EN;
}

void FlintDebugger::caughtException(FlintExecution *exec, FlintJavaThrowable *excp) {
    execution = exec;
    exception = excp;
    lock();
    uint16_t tmp = csr & ~(DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT);
    tmp |= DBG_STATUS_STOP | DBG_STATUS_STOP_SET | DBG_STATUS_EXCP;
    csr = tmp;
    unlock();
    checkBreakPoint(exec);
}

void FlintDebugger::checkBreakPoint(FlintExecution *exec) {
    if(csr & DBG_CONTROL_STOP) {
        execution = exec;
        lock();
        csr = (csr & ~DBG_CONTROL_STOP) | DBG_STATUS_STOP | DBG_STATUS_STOP_SET;
        unlock();
    }
    else if(!(csr & DBG_STATUS_STOP)) {
        if(breakPointCount) {
            uint32_t pc = exec->pc;
            FlintMethodInfo *method = exec->method;
            for(uint8_t i = 0; i < breakPointCount; i++) {
                if(breakPoints[i].method == method && breakPoints[i].pc == pc) {
                    execution = exec;
                    lock();
                    csr |= DBG_STATUS_STOP | DBG_STATUS_STOP_SET;
                    unlock();
                    break;
                }
            }
        }
    }
    while(csr & (DBG_STATUS_STOP | DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT | DBG_STATUS_EXCP)) {
        if(execution == exec) {
            if(csr & DBG_CONTROL_STEP_IN) {
                if(csr & DBG_STATUS_STOP) {
                    lock();
                    csr &= ~(DBG_STATUS_STOP | DBG_STATUS_STOP_SET);
                    unlock();
                    return;
                }
                else if(&startPoint.method != exec->method || (exec->pc - startPoint.pc) >= stepCodeLength || exec->pc <= startPoint.pc) {
                    lock();
                    csr = (csr & ~DBG_CONTROL_STEP_IN) | DBG_STATUS_STOP | DBG_STATUS_STOP_SET;
                    unlock();
                }
                else
                    return;
            }
            else if(csr & DBG_CONTROL_STEP_OVER) {
                if(csr & DBG_STATUS_STOP) {
                    lock();
                    csr &= ~(DBG_STATUS_STOP | DBG_STATUS_STOP_SET);
                    unlock();
                    return;
                }
                else if(
                    (exec->startSp <= startPoint.baseSp) &&
                    (&startPoint.method != exec->method || (exec->pc - startPoint.pc) >= stepCodeLength || exec->pc <= startPoint.pc)
                ) {
                    lock();
                    csr = (csr & ~DBG_CONTROL_STEP_OVER) | DBG_STATUS_STOP | DBG_STATUS_STOP_SET;
                    unlock();
                }
                else
                    return;
            }
            else if(csr & DBG_CONTROL_STEP_OUT) {
                if(csr & DBG_STATUS_STOP) {
                    lock();
                    csr &= ~(DBG_STATUS_STOP | DBG_STATUS_STOP_SET);
                    unlock();
                    return;
                }
                else if(exec->startSp < startPoint.baseSp) {
                    lock();
                    csr = (csr & ~DBG_CONTROL_STEP_OUT) | DBG_STATUS_STOP | DBG_STATUS_STOP_SET;
                    unlock();
                }
                else
                    return;
            }
        }
    }
}

void FlintDebugger::lock(void) {
    FlintAPI::Thread::lock(dbgLockHandle);
}

void FlintDebugger::unlock(void) {
    FlintAPI::Thread::unlock(dbgLockHandle);
}
