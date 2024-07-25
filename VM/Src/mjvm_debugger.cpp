
#include <iostream>
#include <string.h>
#include "mjvm.h"
#include "mjvm_debugger.h"
#include "mjvm_system_api.h"

MjvmBreakPoint::MjvmBreakPoint(void) : pc(0), method(0) {

}

MjvmBreakPoint::MjvmBreakPoint(uint32_t pc, MjvmMethodInfo &method) : pc(pc), method(&method) {

}

MjvmStackFrame::MjvmStackFrame(void) : pc(0), baseSp(0), method(*(MjvmMethodInfo *)0) {

}

MjvmStackFrame::MjvmStackFrame(uint32_t pc, uint32_t baseSp, MjvmMethodInfo &method) : pc(pc), baseSp(baseSp), method(method) {

}

MjvmDebugger::MjvmDebugger(Mjvm &mjvm) : mjvm(mjvm) {
    execution = 0;
    exception = 0;
    installClassFileHandle = 0;
    stepCodeLength = 0;
    csr = DBG_STATUS_RESET;
    breakPointCount = 0;
    txDataLength = 0;
}

void MjvmDebugger::clearTxBuffer(void) {
    txDataLength = 0;
}

void MjvmDebugger::initDataFrame(MjvmDbgCmd cmd, MjvmDbgRespCode responseCode, uint32_t dataLength) {
    txBuff[0] = (uint8_t)(cmd | 0x80);
    txBuff[1] = (uint8_t)(dataLength >> 0);
    txBuff[2] = (uint8_t)(dataLength >> 8);
    txBuff[3] = (uint8_t)(dataLength >> 16);
    txBuff[4] = (uint8_t)responseCode;
    txDataLength = 5;
}

bool MjvmDebugger::dataFrameAppend(uint8_t data) {
    if(txDataLength == sizeof(txBuff)) {
        if(!sendData(txBuff, sizeof(txBuff)))
            return false;
        txDataLength = 0;
    }
    txBuff[txDataLength++] = data;
    return true;
}

bool MjvmDebugger::dataFrameAppend(uint16_t data) {
    if(!dataFrameAppend((uint8_t)data))
        return false;
    return dataFrameAppend((uint8_t)(data >> 8));
}

bool MjvmDebugger::dataFrameAppend(uint32_t data) {
    if(!dataFrameAppend((uint8_t)data))
        return false;
    else if(!dataFrameAppend((uint8_t)(data >> 8)))
        return false;
    else if(!dataFrameAppend((uint8_t)(data >> 16)))
        return false;
    return dataFrameAppend((uint8_t)(data >> 24));
}

bool MjvmDebugger::dataFrameAppend(uint64_t data) {
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

bool MjvmDebugger::dataFrameAppend(uint8_t *data, uint16_t length) {
    for(uint16_t i = 0; i < length; i++) {
        if(!dataFrameAppend(data[i]))
            return false;
    }
    return true;
}

bool MjvmDebugger::dataFrameAppend(MjvmConstUtf8 &utf8) {
    uint32_t size = sizeof(MjvmConstUtf8) + utf8.length + 1;
    uint8_t *buff = (uint8_t *)&utf8;
    for(uint32_t i = 0; i < size; i++) {
        if(!dataFrameAppend(buff[i]))
            return false;
    }
    return true;
}

bool MjvmDebugger::dataFrameFinish(void) {
    if(txDataLength) {
        uint32_t length = txDataLength;
        txDataLength = 0;
        return sendData(txBuff, length);
    }
    return true;
}

bool MjvmDebugger::sendRespCode(MjvmDbgCmd cmd, MjvmDbgRespCode responseCode) {
    initDataFrame(cmd, responseCode, 0);
    return dataFrameFinish();
}

void MjvmDebugger::responseStatus(void) {
    uint16_t tmp = csr;
    if(tmp & (DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT))
        tmp &= ~(DBG_STATUS_STOP_SET | DBG_STATUS_STOP);
    initDataFrame(DBG_CMD_READ_STATUS, DBG_RESP_OK, 1);
    dataFrameAppend((uint8_t)tmp);
    if(dataFrameFinish() && (tmp & DBG_STATUS_STOP_SET)) {
        Mjvm::lock();
        csr &= ~DBG_STATUS_STOP_SET;
        Mjvm::unlock();
    }
}

void MjvmDebugger::responseStackTrace(uint32_t stackIndex) {
    if(csr & DBG_STATUS_STOP) {
        MjvmStackFrame stackTrace;
        bool isEndStack = false;
        if(execution->getStackTrace(stackIndex, &stackTrace, &isEndStack)) {
            MjvmMethodInfo &method = stackTrace.method;
            MjvmConstUtf8 &className = method.classLoader.getThisClass();

            uint32_t responseSize = 8;
            responseSize += sizeof(MjvmConstUtf8) + className.length + 1;
            responseSize += sizeof(MjvmConstUtf8) + method.name.length + 1;
            responseSize += sizeof(MjvmConstUtf8) + method.descriptor.length + 1;

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

void MjvmDebugger::responseExceptionInfo(void) {
    if(csr & DBG_STATUS_STOP) {
        if(
            exception &&
            (csr & DBG_STATUS_EXCP) &&
            mjvm.isInstanceof(exception, throwableClassName.text, throwableClassName.length)
        ) {
            MjvmConstUtf8 &type = exception->type;
            MjvmString &str = exception->getDetailMessage();
            uint32_t responseSize = sizeof(MjvmConstUtf8) * 2 + type.length + MjvmString::getUft8BuffSize(str) + 2;
            uint8_t coder = str.getCoder();
            const char *text = str.getText();
            uint32_t msgLen = str.getLength();
            char utf8Buff[3];

            initDataFrame(DBG_CMD_READ_EXCP_INFO, DBG_RESP_OK, responseSize);
            if(!dataFrameAppend(type)) return;
            if(!dataFrameAppend((uint16_t)msgLen)) return;
            if(!dataFrameAppend((uint16_t)0)) return;
            if(coder == 0) for(uint32_t i = 0; i < msgLen; i++) {
                uint8_t encodeSize = MjvmString::utf8Encode(text[i], utf8Buff);
                for(uint8_t j = 0; j < encodeSize; j++)
                    if(!dataFrameAppend((uint8_t)utf8Buff[j])) return;
            }
            else for(uint32_t i = 0; i < msgLen; i++) {
                uint8_t encodeSize = MjvmString::utf8Encode(((uint16_t *)text)[i], utf8Buff);
                for(uint8_t j = 0; j < encodeSize; j++)
                    if(!dataFrameAppend((uint8_t)utf8Buff[j])) return;
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

void MjvmDebugger::responseLocalVariable(bool isU64, uint32_t stackIndex, uint32_t localIndex) {
    if(csr & DBG_STATUS_STOP) {
        if(!isU64) {
            uint32_t value;
            bool isObject;
            if(execution->readLocal(stackIndex, localIndex, value, isObject)) {
                uint32_t responseSize = 8;
                uint32_t valueSize = 4;
                if(isObject) {
                    MjvmObject &obj = *(MjvmObject *)value;
                    MjvmConstUtf8 &type = obj.type;
                    uint8_t isPrim = obj.isPrimType(type);
                    valueSize = obj.size;
                    responseSize += sizeof(MjvmConstUtf8) + obj.dimensions + (isPrim ? 0 : 2) + type.length + 1;
                }
                initDataFrame(DBG_CMD_READ_LOCAL, DBG_RESP_OK, responseSize);
                if(!dataFrameAppend((uint32_t)valueSize)) return;
                if(!dataFrameAppend((uint32_t)value)) return;

                if(isObject) {
                    MjvmObject &obj = *(MjvmObject *)value;
                    MjvmConstUtf8 &type = obj.type;
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

void MjvmDebugger::responseField(MjvmObject *obj, MjvmConstUtf8 &fieldName) {
    if(csr & DBG_STATUS_STOP) {
        if(obj) {
            MjvmFieldsData *fields = (MjvmFieldsData *)obj->data;
            uint8_t fieldType = 0;
            void *fieldData = (void *)&fields->getFieldData32(fieldName);
            if(!fieldData) {
                fieldType = 1;
                fieldData = (void *)&fields->getFieldData64(fieldName);
                if(!fieldData) {
                    fieldType = 2;
                    fieldData = (void *)&fields->getFieldObject(fieldName);
                }
            }
            if(fieldData) {
                if(fieldType == 0) {
                    initDataFrame(DBG_CMD_READ_FIELD, DBG_RESP_OK, 8);
                    if(!dataFrameAppend((uint32_t)4)) return;
                    if(!dataFrameAppend((uint32_t)((MjvmFieldData32 *)fieldData)->value)) return;
                }
                else if(fieldType == 1) {
                    initDataFrame(DBG_CMD_READ_FIELD, DBG_RESP_OK, 12);
                    if(!dataFrameAppend((uint32_t)8)) return;
                    if(!dataFrameAppend((uint64_t)((MjvmFieldData64 *)fieldData)->value)) return;
                }
                else {
                    MjvmObject *subObj = ((MjvmFieldObject *)fieldData)->object;
                    if(subObj) {
                        MjvmConstUtf8 &type = subObj->type;
                        uint8_t isPrim = subObj->isPrimType(type);
                        uint16_t typeLength = subObj->dimensions + (isPrim ? 0 : 2) + type.length;
                        uint32_t responseSize = 8 + sizeof(MjvmConstUtf8) + typeLength + 1;

                        initDataFrame(DBG_CMD_READ_FIELD, DBG_RESP_OK, responseSize);
                        if(!dataFrameAppend((uint32_t)subObj->size)) return;
                        if(!dataFrameAppend((uint32_t)((MjvmFieldObject *)fieldData)->object)) return;
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
            sendRespCode(DBG_CMD_READ_FIELD, DBG_RESP_FAIL);
    }
    else
        sendRespCode(DBG_CMD_READ_FIELD, DBG_RESP_BUSY);
}

void MjvmDebugger::responseArray(MjvmObject *array, uint32_t index, uint32_t length) {
    if(csr & DBG_STATUS_STOP) {
        if(array->dimensions > 0) {
            uint8_t atype = MjvmObject::isPrimType(array->type);
            uint8_t elementSize = atype ? MjvmObject::getPrimitiveTypeSize(atype) : sizeof(MjvmObject *);
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

void MjvmDebugger::responseObjSizeAndType(MjvmObject *obj) {
    if(csr & DBG_STATUS_STOP) {
        MjvmConstUtf8 &type = obj->type;
        uint8_t isPrim = obj->isPrimType(type);
        uint16_t typeLength = obj->dimensions + (isPrim ? 0 : 2) + type.length;
        uint32_t responseSize = 4 + sizeof(MjvmConstUtf8) + typeLength + 1;

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

bool MjvmDebugger::receivedDataHandler(uint8_t *data, uint32_t length) {
    switch((MjvmDbgCmd)data[0]) {
        case DBG_CMD_READ_STATUS: {
            responseStatus();
            return true;
        }
        case DBG_CMD_READ_STACK_TRACE: {
            uint32_t stackIndex = (*(uint32_t *)&data[1]) & 0x7FFFFFFF;
            responseStackTrace(stackIndex);
            return true;
        }
        case DBG_CMD_ADD_BKP:
        case DBG_CMD_REMOVE_BKP: {
            uint32_t index = sizeof(MjvmDbgCmd);
            uint32_t pc = *(uint32_t *)&data[index];
            index += sizeof(uint32_t);
            MjvmConstUtf8 &className = *(MjvmConstUtf8 *)&data[index];
            index += sizeof(MjvmConstUtf8) + className.length + 1;
            MjvmConstUtf8 &methodName = *(MjvmConstUtf8 *)&data[index];
            index += sizeof(MjvmConstUtf8) + methodName.length + 1;
            MjvmConstUtf8 &descriptor = *(MjvmConstUtf8 *)&data[index];
            if((MjvmDbgCmd)data[0] == DBG_CMD_ADD_BKP)
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
            Mjvm::lock();
            csr &= ~(DBG_STATUS_STOP | DBG_STATUS_STOP_SET | DBG_STATUS_EXCP | DBG_CONTROL_STOP | DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT);
            Mjvm::unlock();
            sendRespCode(DBG_CMD_RUN, DBG_RESP_OK);
            return true;
        }
        case DBG_CMD_STOP: {
            Mjvm::lock();
            csr = (csr & ~(DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT)) | DBG_CONTROL_STOP;
            Mjvm::unlock();
            sendRespCode(DBG_CMD_STOP, DBG_RESP_OK);
            return true;
        }
        case DBG_CMD_RESTART: {
            MjvmConstUtf8 *mainClass = (MjvmConstUtf8 *)&data[1];
            Mjvm::lock();
            csr &= DBG_CONTROL_EXCP_EN;
            Mjvm::unlock();
            mjvm.setDebugger(this);
            mjvm.terminateAll();
            try {
                mjvm.runToMain(mainClass->text);
                sendRespCode(DBG_CMD_RESTART, DBG_RESP_OK);
            }
            catch(...) {
                sendRespCode(DBG_CMD_RESTART, DBG_RESP_FAIL);
            }
            return true;
        }
        case DBG_CMD_TERMINATE: {
            bool endDbg = data[1] != 0;
            Mjvm::lock();
            csr = (csr & DBG_CONTROL_EXCP_EN) | DBG_STATUS_RESET;
            Mjvm::unlock();
            mjvm.terminateAll();
            sendRespCode(DBG_CMD_TERMINATE, DBG_RESP_OK);
            return !endDbg;
        }
        case DBG_CMD_STEP_IN:
        case DBG_CMD_STEP_OVER: {
            if(csr & DBG_STATUS_STOP && length == 5 && data[1] > 0) {
                if(execution->getStackTrace(0, &startPoint, 0)) {
                    stepCodeLength = *(uint32_t *)&data[1];
                    if((MjvmDbgCmd)data[0] == DBG_CMD_STEP_IN) {
                        Mjvm::lock();
                        csr = (csr & ~(DBG_STATUS_STOP_SET | DBG_STATUS_EXCP | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT)) | DBG_CONTROL_STEP_IN;
                        Mjvm::unlock();
                    }
                    else {
                        Mjvm::lock();
                        csr = (csr & ~(DBG_STATUS_STOP_SET | DBG_STATUS_EXCP | DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OUT)) | DBG_CONTROL_STEP_OVER;
                        Mjvm::unlock();
                    }
                    sendRespCode((MjvmDbgCmd)data[0], DBG_RESP_OK);
                }
                else
                    sendRespCode((MjvmDbgCmd)data[0], DBG_RESP_FAIL);
            }
            else
                sendRespCode((MjvmDbgCmd)data[0], DBG_RESP_BUSY);
            return true;
        }
        case DBG_CMD_STEP_OUT: {
            if(csr & DBG_STATUS_STOP) {
                if(execution->getStackTrace(0, &startPoint, 0)) {
                    Mjvm::lock();
                    csr = (csr & ~(DBG_STATUS_STOP_SET | DBG_STATUS_EXCP | DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER)) | DBG_CONTROL_STEP_OUT;
                    Mjvm::unlock();
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
            Mjvm::lock();
            if(data[1] & 0x01)
                csr |= DBG_CONTROL_EXCP_EN;
            else
                csr &= ~DBG_CONTROL_EXCP_EN;
            Mjvm::unlock();
            sendRespCode(DBG_CMD_SET_EXCP_MODE, DBG_RESP_OK);
            return true;
        }
        case DBG_CMD_READ_EXCP_INFO: {
            responseExceptionInfo();
            return true;
        }
        case DBG_CMD_READ_LOCAL: {
            bool isU64 = data[1];
            uint32_t stackIndex = (*(uint32_t *)&data[2]) & 0x7FFFFFFF;
            uint32_t localIndex = (*(uint32_t *)&data[6]);
            responseLocalVariable(isU64, stackIndex, localIndex);
            return true;
        }
        case DBG_CMD_WRITE_LOCAL: {
            // TODO
            return true;
        }
        case DBG_CMD_READ_FIELD: {
            MjvmObject *obj = (MjvmObject *)*(uint32_t *)&data[1];
            MjvmConstUtf8 &fieldName = *(MjvmConstUtf8 *)&data[5];
            responseField(obj, fieldName);
            return true;
        }
        case DBG_CMD_WRITE_FIELD: {
            // TODO
            return true;
        }
        case DBG_CMD_READ_ARRAY: {
            uint32_t length = (*(uint32_t *)&data[0]) >> 8;
            uint32_t index = *(uint32_t *)&data[4];
            MjvmObject *array = (MjvmObject *)*(uint32_t *)&data[8];
            responseArray(array, index, length);
            return true;
        }
        case DBG_CMD_READ_SIZE_AND_TYPE: {
            MjvmObject *obj = (MjvmObject *)*(uint32_t *)&data[1];
            responseObjSizeAndType(obj);
            return true;
        }
        case DBG_CMD_INSTALL_FILE: {
            if(csr & DBG_STATUS_RESET) {
                if(installClassFileHandle)
                    MjvmSystem_FileClose(installClassFileHandle);
                MjvmConstUtf8 *fileName = (MjvmConstUtf8 *)&data[1];
                installClassFileHandle = MjvmSystem_FileOpen(fileName->text, MJVM_FILE_CREATE_ALWAYS);
                if(installClassFileHandle)
                    sendRespCode(DBG_CMD_INSTALL_FILE, DBG_RESP_OK);
                else
                    sendRespCode(DBG_CMD_INSTALL_FILE, DBG_RESP_FAIL);
            }
            else
                sendRespCode(DBG_CMD_INSTALL_FILE, DBG_RESP_BUSY);
            return true;
        }
        case DBG_CMD_WRITE_FILE_DATA: {
            if(csr & DBG_STATUS_RESET) {
                uint32_t bw = 0;
                if(
                    installClassFileHandle &&
                    MjvmSystem_FileWrite(installClassFileHandle, &data[1], length - 1, &bw) == FILE_RESULT_OK
                ) {
                    sendRespCode(DBG_CMD_WRITE_FILE_DATA, DBG_RESP_OK);
                }
                else
                    sendRespCode(DBG_CMD_WRITE_FILE_DATA, DBG_RESP_FAIL);
            }
            else
                sendRespCode(DBG_CMD_WRITE_FILE_DATA, DBG_RESP_BUSY);
            return true;
        }
        case DBG_CMD_COMPLATE_INSTALL: {
            if(csr & DBG_STATUS_RESET) {
                if(
                    installClassFileHandle &&
                    MjvmSystem_FileClose(installClassFileHandle) == FILE_RESULT_OK
                ) {
                    sendRespCode(DBG_CMD_COMPLATE_INSTALL, DBG_RESP_OK);
                }
                else
                    sendRespCode(DBG_CMD_COMPLATE_INSTALL, DBG_RESP_FAIL);
            }
            else
                sendRespCode(DBG_CMD_COMPLATE_INSTALL, DBG_RESP_BUSY);
            return true;
        }
        default: {
            sendRespCode((MjvmDbgCmd)data[0], DBG_RESP_UNKNOW);
            return true;
        }
    }
}

bool MjvmDebugger::addBreakPoint(uint32_t pc, MjvmConstUtf8 &className, MjvmConstUtf8 &methodName, MjvmConstUtf8 &descriptor) {
    try {
        if(breakPointCount < LENGTH(breakPoints)) {
            MjvmClassLoader &loader = mjvm.load(className);
            MjvmMethodInfo *method = &loader.getMethodInfo(methodName, descriptor);
            if(method) {
                for(uint8_t i = 0; i < breakPointCount; i++) {
                    if(method == breakPoints[i].method && pc == breakPoints[i].pc)
                        return true;
                }
                new (&breakPoints[breakPointCount])MjvmBreakPoint(pc, *method);
                breakPointCount++;
                return true;
            }
        }
    }
    catch(MjvmLoadFileError *file) {

    }
    return false;
}

bool MjvmDebugger::removeBreakPoint(uint32_t pc, MjvmConstUtf8 &className, MjvmConstUtf8 &methodName, MjvmConstUtf8 &descriptor) {
    try {
        if(breakPointCount) {
            MjvmClassLoader &loader = mjvm.load(className);
            MjvmMethodInfo *method = &loader.getMethodInfo(methodName, descriptor);
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
    catch(MjvmLoadFileError *file) {

    }
    return false;
}

bool MjvmDebugger::exceptionIsEnabled(void) {
    return (csr & DBG_CONTROL_EXCP_EN) == DBG_CONTROL_EXCP_EN;
}

void MjvmDebugger::caughtException(MjvmExecution *exec, MjvmThrowable *excp) {
    execution = exec;
    exception = excp;
    Mjvm::lock();
    uint16_t tmp = csr & ~(DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT);
    tmp |= DBG_STATUS_STOP | DBG_STATUS_STOP_SET | DBG_STATUS_EXCP;
    csr = tmp;
    Mjvm::unlock();
    checkBreakPoint(exec);
}

void MjvmDebugger::checkBreakPoint(MjvmExecution *exec) {
    if(csr & DBG_CONTROL_STOP) {
        execution = exec;
        Mjvm::lock();
        csr = (csr & ~DBG_CONTROL_STOP) | DBG_STATUS_STOP | DBG_STATUS_STOP_SET;
        Mjvm::unlock();
    }
    else if(!(csr & DBG_STATUS_STOP)) {
        if(breakPointCount) {
            uint32_t pc = exec->pc;
            MjvmMethodInfo *method = exec->method;
            for(uint8_t i = 0; i < breakPointCount; i++) {
                if(breakPoints[i].method == method && breakPoints[i].pc == pc) {
                    execution = exec;
                    Mjvm::lock();
                    csr |= DBG_STATUS_STOP | DBG_STATUS_STOP_SET;
                    Mjvm::unlock();
                    break;
                }
            }
        }
    }
    while(csr & (DBG_STATUS_STOP | DBG_CONTROL_STEP_IN | DBG_CONTROL_STEP_OVER | DBG_CONTROL_STEP_OUT | DBG_STATUS_EXCP)) {
        if(execution == exec) {
            if(csr & DBG_CONTROL_STEP_IN) {
                if(csr & DBG_STATUS_STOP) {
                    Mjvm::lock();
                    csr &= ~(DBG_STATUS_STOP | DBG_STATUS_STOP_SET);
                    Mjvm::unlock();
                    return;
                }
                else if(&startPoint.method != exec->method || (exec->pc - startPoint.pc) >= stepCodeLength || exec->pc <= startPoint.pc) {
                    Mjvm::lock();
                    csr = (csr & ~DBG_CONTROL_STEP_IN) | DBG_STATUS_STOP | DBG_STATUS_STOP_SET;
                    Mjvm::unlock();
                }
                else
                    return;
            }
            else if(csr & DBG_CONTROL_STEP_OVER) {
                if(csr & DBG_STATUS_STOP) {
                    Mjvm::lock();
                    csr &= ~(DBG_STATUS_STOP | DBG_STATUS_STOP_SET);
                    Mjvm::unlock();
                    return;
                }
                else if(
                    (exec->startSp <= startPoint.baseSp) &&
                    (&startPoint.method != exec->method || (exec->pc - startPoint.pc) >= stepCodeLength || exec->pc <= startPoint.pc)
                ) {
                    Mjvm::lock();
                    csr = (csr & ~DBG_CONTROL_STEP_OVER) | DBG_STATUS_STOP | DBG_STATUS_STOP_SET;
                    Mjvm::unlock();
                }
                else
                    return;
            }
            else if(csr & DBG_CONTROL_STEP_OUT) {
                if(csr & DBG_STATUS_STOP) {
                    Mjvm::lock();
                    csr &= ~(DBG_STATUS_STOP | DBG_STATUS_STOP_SET);
                    Mjvm::unlock();
                    return;
                }
                else if(exec->startSp < startPoint.baseSp) {
                    Mjvm::lock();
                    csr = (csr & ~DBG_CONTROL_STEP_OUT) | DBG_STATUS_STOP | DBG_STATUS_STOP_SET;
                    Mjvm::unlock();
                }
                else
                    return;
            }
        }
    }
}

void MjvmDebugger::clearResetStatus(void) {
    Mjvm::lock();
    this->csr &= ~DBG_STATUS_RESET;
    Mjvm::unlock();
}
