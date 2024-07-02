
#include <iostream>
#include <string.h>
#include "mjvm.h"
#include "mjvm_debugger.h"

BreakPoint::BreakPoint(void) : pc(0), method(0) {

}

BreakPoint::BreakPoint(uint32_t pc, MethodInfo &method) : pc(pc), method(&method) {

}

StackTrace::StackTrace(void) : pc(0), method(*(MethodInfo *)0) {
    
}

StackTrace::StackTrace(uint32_t pc, MethodInfo &method) : pc(pc), method(method) {

}

Debugger::Debugger(Execution &execution) : execution(execution), status(DBG_STATUS_STOP), breakPointCount(0) {

}

void Debugger::checkBreakPoint(uint32_t pc, const MethodInfo *method) {
    if(!(status & DBG_STATUS_STOP)) {
        if(breakPointCount) {
            for(uint8_t i = 0; i < breakPointCount; i++) {
                if(breakPoints[i].method == method && breakPoints[i].pc == pc) {
                    Mjvm::lock();
                    status |= DBG_STATUS_STOP | DBG_STATUS_STOP_SET;
                    Mjvm::unlock();
                    break;
                }
            }
        }
    }
    while(status & (DBG_STATUS_STOP | DBG_STATUS_STEP_IN)) {
        if(status & DBG_STATUS_STEP_IN) {
            if(status & DBG_STATUS_STOP) {
                Mjvm::lock();
                status &= ~(DBG_STATUS_STOP | DBG_STATUS_STOP_SET);
                Mjvm::unlock();
                return;
            }
            else if(
                &startPoint.method != method ||
                (pc - startPoint.pc) >= stepCodeLength ||
                pc <= startPoint.pc
            ) {
                Mjvm::lock();
                status = (status & ~DBG_STATUS_STEP_IN) | DBG_STATUS_STOP | DBG_STATUS_STOP_SET;
                Mjvm::unlock();
            }
            else
                return;
        }
    }
}

void Debugger::receivedDataHandler(uint8_t *data, uint32_t length) {
    static uint8_t txBuff[1024];
    txBuff[0] = data[0];
    switch((DebuggerCmd)data[0]) {
        case DBG_READ_STATUS: {
            txBuff[1] = 0;
            txBuff[2] = status;
            if(sendData(txBuff, 3)) {
                Mjvm::lock();
                status &= ~DBG_STATUS_STOP_SET;
                Mjvm::unlock();
            }
            break;
        }
        case DBG_READ_STACK_TRACE: {
            if(status & DBG_STATUS_STOP && length == 5) {
                uint32_t index = sizeof(DebuggerCmd);
                uint32_t stackIndex = *(uint32_t *)&data[1];

                StackTrace stackTrace;
                execution.getStackTrace(stackIndex, &stackTrace);

                MethodInfo &method = stackTrace.method;
                ConstUtf8 &className = method.classLoader.getThisClass();

                uint32_t responseSize = 10;
                responseSize += sizeof(ConstUtf8) + className.length + 1;
                responseSize += sizeof(ConstUtf8) + method.name.length + 1;
                responseSize += sizeof(ConstUtf8) + method.descriptor.length + 1;

                if(responseSize <= sizeof(txBuff)) {
                    txBuff[1] = 0;
                    index += sizeof(uint8_t);
                    *(uint32_t *)&txBuff[index] = stackIndex;
                    index += sizeof(uint32_t);
                    *(uint32_t *)&txBuff[index] = stackTrace.pc;
                    index += sizeof(uint32_t);
                    memcpy(&txBuff[index], &className, sizeof(ConstUtf8) + className.length + 1);
                    index += sizeof(ConstUtf8) + className.length + 1;
                    memcpy(&txBuff[index], &method.name, sizeof(ConstUtf8) + method.name.length + 1);
                    index += sizeof(ConstUtf8) + method.name.length + 1;
                    memcpy(&txBuff[index], &method.descriptor, sizeof(ConstUtf8) + method.descriptor.length + 1);
                    sendData(txBuff, responseSize);
                }
                else {
                    txBuff[1] = 2;
                    sendData(txBuff, 2);
                }
            }
            else {
                txBuff[1] = 1;
                sendData(txBuff, 2);
            }
            break;
        }
        case DBG_ADD_BKP:
        case DBG_REMOVE_BKP: {
            uint32_t index = sizeof(DebuggerCmd);
            uint32_t pc = *(uint32_t *)&data[index];
            index += sizeof(uint32_t);
            ConstUtf8 &className = *(ConstUtf8 *)&data[index];
            index += sizeof(ConstUtf8) + className.length + 1;
            ConstUtf8 &methodName = *(ConstUtf8 *)&data[index];
            index += sizeof(ConstUtf8) + methodName.length + 1;
            ConstUtf8 &descriptor = *(ConstUtf8 *)&data[index];
            if((DebuggerCmd)data[0] == DBG_ADD_BKP)
                txBuff[1] = !addBreakPoint(pc, className, methodName, descriptor);
            else
                txBuff[1] = !removeBreakPoint(pc, className, methodName, descriptor);
            sendData(txBuff, 2);
            break;
        }
        case DBG_REMOVE_ALL_BKP: {
            breakPointCount = 0;
            txBuff[1] = 0;
            sendData(txBuff, 2);
            break;
        }
        case DBG_RUN: {
            Mjvm::lock();
            status &= ~DBG_STATUS_STOP;
            Mjvm::unlock();
            txBuff[1] = 0;
            sendData(txBuff, 2);
            break;
        }
        case DBG_STOP: {
            Mjvm::lock();
            status = (status & ~DBG_STATUS_STOP_SET) | DBG_STATUS_STOP;
            Mjvm::unlock();
            txBuff[1] = 0;
            sendData(txBuff, 2);
            break;
        }
        case DBG_STEP_IN: {
            if(status & DBG_STATUS_STOP && length == 5 && data[1] > 0) {
                execution.getStackTrace(0, &startPoint);
                stepCodeLength = *(uint32_t *)&data[1];
                Mjvm::lock();
                status |= DBG_STATUS_STEP_IN;
                Mjvm::unlock();
                txBuff[1] = 0;
            }
            else
                txBuff[1] = 1;
            sendData(txBuff, 2);
            break;
        }
        case DBG_READ_VARIABLE:
        case DBG_WRITE_VARIABLE: {
            if(status & DBG_STATUS_STOP) {
                // TODO
                if((DebuggerCmd)data[0] == DBG_READ_VARIABLE) {
                    // TODO
                }
                else {
                    // TODO
                }
            }
            else
                txBuff[1] = 1;
            sendData(txBuff, 2);
        }
        default: {
            txBuff[1] = 0xFF;
            sendData(txBuff, 2);
            break;
        }
    }
}

bool Debugger::addBreakPoint(uint32_t pc, ConstUtf8 &className, ConstUtf8 &methodName, ConstUtf8 &descriptor) {
    try {
        if(breakPointCount < LENGTH(breakPoints)) {
            ClassLoader &loader = execution.load(className);
            MethodInfo *method = &loader.getMethodInfo(methodName, descriptor);
            if(method) {
                for(uint8_t i = 0; i < breakPointCount; i++) {
                    if(method == breakPoints[i].method && pc == breakPoints[i].pc)
                        return true;
                }
                new (&breakPoints[breakPointCount])BreakPoint(pc, *method);
                breakPointCount++;
                return true;
            }
        }
    }
    catch(LoadFileError *file) {

    }
    return false;
}

bool Debugger::removeBreakPoint(uint32_t pc, ConstUtf8 &className, ConstUtf8 &methodName, ConstUtf8 &descriptor) {
    try {
        if(breakPointCount) {
            ClassLoader &loader = execution.load(className);
            MethodInfo *method = &loader.getMethodInfo(methodName, descriptor);
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
    catch(LoadFileError *file) {

    }
    return false;
}
