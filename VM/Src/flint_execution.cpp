
#include <iostream>
#include <string.h>
#include "flint.h"
#include "flint_opcodes.h"
#include "flint_execution.h"
#include "flint_const_name_base.h"
#include "flint_system_api.h"
#include "flint_throw_support.h"
#include "flint_default_conf.h"
#include "flint_default_compiler_define.h"

#define FLOAT_NAN                           0x7FC00000
#define DOUBLE_NAN                          0x7FF8000000000000

#define ARRAY_TO_INT16(array)               (int16_t)(((array)[0] << 8) | (array)[1])
#define ARRAY_TO_INT32(array)               (int32_t)(((array)[0] << 24) | ((array)[1] << 16) | ((array)[2] << 8) | (array)[3])

#define GET_STACK_VALUE(_index)             stack[_index]
#define SET_STACK_VALUE(_index, _value)     stack[_index] = _value

static const void **opcodeLabelsStop = 0;
static const void **opcodeLabelsExit = 0;

FlintExecution::FlintExecution(Flint &flint, FlintJavaThread *onwerThread) : flint(flint), stackLength(DEFAULT_STACK_SIZE / sizeof(int32_t)) {
    this->opcodes = 0;
    this->lr = -1;
    this->sp = -1;
    this->startSp = sp;
    this->peakSp = sp;
    this->stack = (int32_t *)Flint::malloc(DEFAULT_STACK_SIZE);
    this->onwerThread = onwerThread;
}

FlintExecution::FlintExecution(Flint &flint, FlintJavaThread *onwerThread, uint32_t stackSize) : flint(flint), stackLength(stackSize / sizeof(int32_t)) {
    this->opcodes = 0;
    this->lr = -1;
    this->sp = -1;
    this->startSp = sp;
    this->peakSp = sp;
    this->stack = (int32_t *)Flint::malloc(stackSize);
    this->onwerThread = onwerThread;
}

void FlintExecution::stackPushInt32(int32_t value) {
    stack[++sp] = value;
    peakSp = sp;
}

void FlintExecution::stackPushInt64(int64_t value) {
    stack[++sp] = ((uint32_t *)&value)[0];
    stack[++sp] = ((uint32_t *)&value)[1];
    peakSp = sp;
}

void FlintExecution::stackPushFloat(float value) {
    stack[++sp] = *(uint32_t *)&value;
    peakSp = sp;
}

void FlintExecution::stackPushDouble(double value) {
    stack[++sp] = ((uint32_t *)&value)[0];
    stack[++sp] = ((uint32_t *)&value)[1];
    peakSp = sp;
}

void FlintExecution::stackPushObject(FlintJavaObject *obj) {
    stack[++sp] = (int32_t)obj;
    peakSp = sp;
    if(obj && (obj->getProtected() & 0x02))
        flint.clearProtectObjectNew(*obj);
}

int32_t FlintExecution::stackPopInt32(void) {
    return stack[sp--];
}

int64_t FlintExecution::stackPopInt64(void) {
    uint64_t ret;
    ((uint32_t *)&ret)[1] = stack[sp--];
    ((uint32_t *)&ret)[0] = stack[sp--];
    return ret;
}

float FlintExecution::stackPopFloat(void) {
    return *(float *)&stack[sp--];
}

double FlintExecution::stackPopDouble(void) {
    uint64_t ret;
    sp -= 2;
    ((uint32_t *)&ret)[1] = stack[sp--];
    ((uint32_t *)&ret)[0] = stack[sp--];
    return *(double *)&ret;
}

FlintJavaObject *FlintExecution::stackPopObject(void) {
    return (FlintJavaObject *)stack[sp--];
}

bool FlintExecution::getStackTrace(uint32_t index, FlintStackFrame *stackTrace, bool *isEndStack) const {
    if(index == 0) {
        new (stackTrace)FlintStackFrame(pc, startSp, *method);
        if(isEndStack)
            *isEndStack = (startSp < 4);
        return true;
    }
    else {
        int32_t traceSp = startSp;
        if(traceSp < 4)
            return false;
        while(--index) {
            traceSp = stack[traceSp];
            if(traceSp < 4)
                return false;
        }
        uint32_t tracePc = stack[traceSp - 2];
        FlintMethodInfo &traceMethod = *(FlintMethodInfo *)stack[traceSp - 3];
        traceSp = stack[traceSp];
        new (stackTrace)FlintStackFrame(tracePc, traceSp, traceMethod);
        if(isEndStack)
            *isEndStack = (traceSp < 4);
        return true;
    }
}

bool FlintExecution::readLocal(uint32_t stackIndex, uint32_t localIndex, uint32_t &value, bool &isObject) const {
    FlintStackFrame stackTrace;
    if(!getStackTrace(stackIndex, &stackTrace, 0))
        return false;
    value = stack[stackTrace.baseSp + 1 + localIndex];
    if(isObject)
        isObject = flint.isObject(value);
    return true;
}

bool FlintExecution::readLocal(uint32_t stackIndex, uint32_t localIndex, uint64_t &value) const {
    FlintStackFrame stackTrace;
    if(!getStackTrace(stackIndex, &stackTrace, 0))
        return false;
    value = *(int64_t *)&stack[stackTrace.baseSp + 1 + localIndex];
    return true;
}

FlintError FlintExecution::initNewContext(FlintMethodInfo *methodInfo, uint16_t argc) {
    uint16_t maxLocals = methodInfo->getMaxLocals();
    uint16_t maxStack = methodInfo->getMaxStack();
    if((sp + maxLocals + maxStack + 4) >= stackLength)
        return ERR_STACK_OVERFLOW;

    /* Save current context */
    stack[++sp] = (int32_t)method;
    stack[++sp] = pc;
    stack[++sp] = lr;
    stack[++sp] = startSp;
    startSp = sp;

    method = methodInfo;
    code = methodInfo->getCode();
    if(code == NULL)
        return throwNoSuchMethodError(*this, methodInfo->classLoader.thisClass->text, methodInfo->getName().text);
    pc = 0;
    locals = &stack[sp + 1];
    for(uint32_t i = argc; i < maxLocals; i++) {
        uint32_t index = sp + i + 1;
        stack[index] = 0;
    }
    sp += maxLocals;
    return ERR_OK;
}

FlintError FlintExecution::stackInitExitPoint(uint32_t exitPc) {
    int32_t argc = sp + 1;
    for(uint32_t i = 0; i < argc; i++)
        SET_STACK_VALUE(sp - i + 4, GET_STACK_VALUE(sp - i));
    sp -= argc;
    pc = lr = exitPc;
    return initNewContext(method, argc);
}

void FlintExecution::stackRestoreContext(void) {
    if(method->accessFlag & METHOD_SYNCHRONIZED) {
        if(method->accessFlag & METHOD_STATIC) {
            if(&method->getName() == (FlintConstUtf8 *)staticConstructorName) {
                FlintClassData &classData = (FlintClassData &)method->classLoader;
                Flint::lock();
                classData.staticInitOwnId = 0;
                Flint::unlock();
            }
            else
                unlockClass((FlintClassData &)method->classLoader);
        }
        else
            unlockObject((FlintJavaObject *)locals[0]);
    }
    sp = startSp;
    startSp = stackPopInt32();
    lr = stackPopInt32();
    pc = stackPopInt32();
    method = (FlintMethodInfo *)stackPopInt32();
    code = method->getCode();
    locals = &stack[startSp + 1];
}

FlintError FlintExecution::lockClass(FlintClassData &cls) {
    Flint::lock();
    if(cls.monitorCount == 0 || cls.ownId == (int32_t)this) {
        cls.ownId = (int32_t)this;
        if(cls.monitorCount < 0xFFFFFFFF) {
            cls.monitorCount++;
            Flint::unlock();
            return ERR_OK;
        }
        Flint::unlock();
        return ERR_LOCK_LIMIT;
    }
    Flint::unlock();
    return ERR_LOCK_FAIL;
}

void FlintExecution::unlockClass(FlintClassData &cls) {
    Flint::lock();
    if(cls.monitorCount)
        cls.monitorCount--;
    Flint::unlock();
}

FlintError FlintExecution::lockObject(FlintJavaObject *obj) {
    Flint::lock();
    if(obj->monitorCount == 0 || obj->ownId == (int32_t)this) {
        obj->ownId = (int32_t)this;
        if(obj->monitorCount < 0xFFFFFF) {
            obj->monitorCount++;
            Flint::unlock();
            return ERR_OK;
        }
        Flint::unlock();
        return ERR_LOCK_LIMIT;
    }
    Flint::unlock();
    return ERR_LOCK_FAIL;
}

void FlintExecution::unlockObject(FlintJavaObject *obj) {
    Flint::lock();
    if(obj->monitorCount)
        obj->monitorCount--;
    Flint::unlock();
}

FlintError FlintExecution::invoke(FlintMethodInfo *methodInfo, uint8_t argc) {
    if(!(methodInfo->accessFlag & METHOD_NATIVE)) {
        peakSp = sp + 4;
        for(uint32_t i = 0; i < argc; i++)
            SET_STACK_VALUE(sp - i + 4, GET_STACK_VALUE(sp - i));
        sp -= argc;
        return initNewContext(methodInfo, argc);
    }
    else {
        int32_t retSp = sp - argc;
        FlintNativeMethodPtr nativeCode = (FlintNativeMethodPtr)methodInfo->getCode();
        if(!nativeCode)
            return throwNoSuchMethodError(*this, methodInfo->classLoader.thisClass->text, methodInfo->getName().text);
        FlintError err = nativeCode(*this);
        if(err != ERR_OK) {
            if(err == ERR_THROW) {
                FlintJavaObject *excp = stackPopObject();
                sp = retSp;
                stackPushObject(excp);
            }
            else
                sp = retSp;
            return err;
        }
        FlintConstUtf8 &methodDesc = methodInfo->getDescriptor();
        uint8_t retType = methodDesc.text[methodDesc.length - 1];
        if(retType != 'V') {
            if(retType == 'J' || retType == 'D') {
                int64_t ret = stackPopInt64();
                sp = retSp;
                stackPushInt64(ret);
            }
            else if(retType == 'L' || retType == '[') {
                FlintJavaObject *ret = stackPopObject();
                sp = retSp;
                stackPushObject(ret);
            }
            else {
                int32_t ret = stackPopInt32();
                sp = retSp;
                stackPushInt32(ret);
            }
        }
        else
            sp = retSp;
        pc = lr;
        return ERR_OK;
    }
}

FlintError FlintExecution::invokeStatic(FlintConstMethod &constMethod) {
    FlintMethodInfo *methodInfo = constMethod.methodInfo;
    if(methodInfo == NULL) {
        FlintError err = flint.findMethod(constMethod, methodInfo);
        if(err != ERR_OK) {
            if(err == ERR_METHOD_NOT_FOUND)
                return throwNoSuchMethodError(*this, constMethod.className.text, constMethod.nameAndType.name.text);
            return checkAndThrowForFlintError(*this, err, (FlintConstUtf8 *)methodInfo);
        }
        constMethod.methodInfo = methodInfo;
    }
    FlintClassData &classData = (FlintClassData &)methodInfo->classLoader;
    if(classData.hasStaticCtor()) {
        FlintInitStatus initStatus = classData.getInitStatus();
        if(initStatus == UNINITIALIZED)
            return invokeStaticCtor(classData);
        else if((initStatus == INITIALIZING) && (classData.staticInitOwnId != (uint32_t)this)) {
            FlintAPI::Thread::yield();
            return ERR_OK;
        }
    }
    if(methodInfo->accessFlag & METHOD_SYNCHRONIZED) {
        FlintError err = lockClass(classData);
        if(err == ERR_LOCK_FAIL) {
            FlintAPI::Thread::yield();
            return ERR_OK;
        }
        RETURN_IF_ERR(err);
    }
    lr = pc + 3;
    return invoke(methodInfo, constMethod.getArgc());
}

FlintError FlintExecution::invokeSpecial(FlintConstMethod &constMethod) {
    uint8_t argc = constMethod.getArgc() + 1;
    FlintMethodInfo *methodInfo = constMethod.methodInfo;
    if(methodInfo == NULL) {
        FlintError err = flint.findMethod(constMethod, methodInfo);
        if(err != ERR_OK) {
            if(err == ERR_METHOD_NOT_FOUND)
                return throwNoSuchMethodError(*this, constMethod.className.text, constMethod.nameAndType.name.text);
            return checkAndThrowForFlintError(*this, err, (FlintConstUtf8 *)methodInfo);
        }
        constMethod.methodInfo = methodInfo;
    }
    FlintClassData &classData = (FlintClassData &)methodInfo->classLoader;
    if(classData.hasStaticCtor()) {
        FlintInitStatus initStatus = classData.getInitStatus();
        if(initStatus == UNINITIALIZED)
            return invokeStaticCtor(classData);
        else if((initStatus == INITIALIZING) && (classData.staticInitOwnId != (uint32_t)this)) {
            FlintAPI::Thread::yield();
            return ERR_OK;
        }
    }
    if(methodInfo->accessFlag & METHOD_SYNCHRONIZED) {
        FlintError err = lockObject((FlintJavaObject *)stack[sp - argc - 1]);
        if(err == ERR_LOCK_FAIL) {
            FlintAPI::Thread::yield();
            return ERR_OK;
        }
        RETURN_IF_ERR(err);
    }
    lr = pc + 3;
    return invoke(methodInfo, argc);
}

FlintError FlintExecution::invokeVirtual(FlintConstMethod &constMethod) {
    uint8_t argc = constMethod.getArgc();
    FlintJavaObject *obj = (FlintJavaObject *)stack[sp - argc];
    if(obj == 0)
        return throwNullPointerException(*this, constMethod);
    FlintConstUtf8 &type = (obj->dimensions > 0 || FlintJavaObject::isPrimType(obj->type)) ? (FlintConstUtf8 &)objectClassName : obj->type;
    FlintMethodInfo *methodInfo = constMethod.methodInfo;
    if((!methodInfo) || (methodInfo->classLoader.thisClass != &type)) {
        FlintError err = flint.findMethod(type, constMethod.nameAndType, methodInfo);
        if(err != ERR_OK) {
            if(err == ERR_METHOD_NOT_FOUND)
                return throwNoSuchMethodError(*this, constMethod.className.text, constMethod.nameAndType.name.text);
            return checkAndThrowForFlintError(*this, err, (FlintConstUtf8 *)methodInfo);
        }
        constMethod.methodInfo = methodInfo;
    }
    if(methodInfo->accessFlag & METHOD_SYNCHRONIZED) {
        FlintError err = lockObject(obj);
        if(err == ERR_LOCK_FAIL) {
            FlintAPI::Thread::yield();
            return ERR_OK;
        }
        RETURN_IF_ERR(err);
    }
    argc++;
    lr = pc + 3;
    return invoke(methodInfo, argc);
}

FlintError FlintExecution::invokeInterface(FlintConstInterfaceMethod &interfaceMethod, uint8_t argc) {
    FlintJavaObject *obj = (FlintJavaObject *)stack[sp - argc + 1];
    if(obj == 0)
        return throwNullPointerException(*this, (FlintConstMethod &)interfaceMethod);
    FlintConstUtf8 &type = (obj->dimensions > 0 || FlintJavaObject::isPrimType(obj->type)) ? (FlintConstUtf8 &)objectClassName : obj->type;
    FlintMethodInfo *methodInfo = interfaceMethod.methodInfo;
    if((!methodInfo) || (methodInfo->classLoader.thisClass != &type)) {
        FlintError err = flint.findMethod(type, interfaceMethod.nameAndType, methodInfo);
        if(err != ERR_OK) {
            if(err == ERR_METHOD_NOT_FOUND)
                return throwNoSuchMethodError(*this, interfaceMethod.className.text, interfaceMethod.nameAndType.name.text);
            return checkAndThrowForFlintError(*this, err, (FlintConstUtf8 *)methodInfo);
        }
    }
    if(methodInfo->accessFlag & METHOD_SYNCHRONIZED) {
        FlintError err = lockObject(obj);
        if(err == ERR_LOCK_FAIL) {
            FlintAPI::Thread::yield();
            return ERR_OK;
        }
        RETURN_IF_ERR(err);
    }
    lr = pc + 5;
    return invoke(methodInfo, argc);
}

FlintError FlintExecution::invokeStaticCtor(FlintClassData &classData) {
    Flint::lock();
    if(classData.getInitStatus() != UNINITIALIZED) {
        Flint::unlock();
        return ERR_OK;
    }
    classData.staticInitOwnId = (uint32_t)this;
    RETURN_IF_ERR(flint.initStaticField(classData));
    Flint::unlock();
    FlintMethodInfo *ctorMethod;
    FlintError err = classData.getStaticCtor(ctorMethod);
    if(err == ERR_OK) {
        lr = pc;
        return invoke(ctorMethod, 0);
    }
    if(err == ERR_METHOD_NOT_FOUND)
        return throwNoSuchMethodError(*this, classData.thisClass->text, ((FlintConstUtf8 *)staticConstructorName)->text);
    return err;
}

FlintError FlintExecution::run(void) {
    #include "flint_instruction_label.h"

    ::opcodeLabelsStop = opcodeLabelsStop;
    ::opcodeLabelsExit = opcodeLabelsExit;

    uint32_t hitBkpPc = 0;
    FlintMethodInfo *hitBkpMethod = NULL;

    FlintDebugger *dbg = flint.getDebugger();
    opcodes = opcodeLabels;

    stackInitExitPoint(method->getCodeLength());

    if(method->classLoader.hasStaticCtor()) {
        FlintError err = invokeStaticCtor((FlintClassData &)method->classLoader);
        if(err != ERR_OK) {
            if(err == ERR_THROW)
                goto exception_handler;
            return err;
        }
        goto *opcodes[code[pc]];
    }

    goto *opcodes[code[pc]];
    dbg_stop: {
        if(dbg) {
            if(!dbg->checkStop(this)) {
                if(hasTerminateRequest())
                    return ERR_TERMINATE_REQUEST;
                opcodes = opcodeLabels;
            }
        }
        goto *opcodeLabels[code[pc]];
    }
    op_nop:
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_m1:
        stackPushInt32(-1);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_0:
    op_aconst_null:
        stackPushInt32(0);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_1:
        stackPushInt32(1);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_2:
        stackPushInt32(2);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_3:
        stackPushInt32(3);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_4:
        stackPushInt32(4);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_5:
        stackPushInt32(5);
        pc++;
        goto *opcodes[code[pc]];
    op_lconst_0:
        stackPushInt64(0);
        pc++;
        goto *opcodes[code[pc]];
    op_lconst_1:
        stackPushInt64(1);
        pc++;
        goto *opcodes[code[pc]];
    op_fconst_0:
        stackPushFloat(0);
        pc++;
        goto *opcodes[code[pc]];
    op_fconst_1:
        stackPushFloat(1);
        pc++;
        goto *opcodes[code[pc]];
    op_fconst_2:
        stackPushFloat(2);
        pc++;
        goto *opcodes[code[pc]];
    op_dconst_0:
        stackPushDouble(0);
        pc++;
        goto *opcodes[code[pc]];
    op_dconst_1:
        stackPushDouble(1);
        pc++;
        goto *opcodes[code[pc]];
    op_bipush:
        stackPushInt32((int8_t)code[pc + 1]);
        pc += 2;
        goto *opcodes[code[pc]];
    op_sipush:
        stackPushInt32(ARRAY_TO_INT16(&code[pc + 1]));
        pc += 3;
        goto *opcodes[code[pc]];
    op_ldc: {
        FlintConstPool &constPool = method->classLoader.getConstPool(code[pc + 1]);
        pc += 2;
        switch(constPool.tag & 0x7F) {
            case CONST_INTEGER:
                stackPushInt32(method->classLoader.getConstInteger(constPool));
                goto *opcodes[code[pc]];
            case CONST_FLOAT:
                stackPushFloat(method->classLoader.getConstFloat(constPool));
                goto *opcodes[code[pc]];
            case CONST_STRING: {
                FlintJavaString *str;
                RETURN_IF_ERR(method->classLoader.getConstString(constPool, str));
                stackPushObject(str);
                goto *opcodes[code[pc]];
            }
            case CONST_CLASS: {
                FlintJavaClass *cls;
                RETURN_IF_ERR(method->classLoader.getConstClass(constPool, cls));
                stackPushObject(cls);
                goto *opcodes[code[pc]];
            }
            case CONST_METHOD_TYPE:
                // TODO
                goto *opcodes[code[pc]];
            case CONST_METHOD_HANDLE:
                // TODO
                goto *opcodes[code[pc]];
            default:
                return ERR_VM_ERROR; // "unkown the const pool tag"
        }
    }
    op_ldc_w: {
        uint16_t index = ARRAY_TO_INT16(&code[pc + 1]);
        FlintConstPool &constPool = method->classLoader.getConstPool(index);
        pc += 3;
        switch(constPool.tag & 0x7F) {
            case CONST_INTEGER:
                stackPushInt32(method->classLoader.getConstInteger(constPool));
                goto *opcodes[code[pc]];
            case CONST_FLOAT:
                stackPushFloat(method->classLoader.getConstFloat(constPool));
                goto *opcodes[code[pc]];
            case CONST_STRING: {
                FlintJavaString *str;
                RETURN_IF_ERR(method->classLoader.getConstString(constPool, str));
                stackPushObject(str);
                goto *opcodes[code[pc]];
            }
            case CONST_CLASS: {
                FlintJavaClass *cls;
                RETURN_IF_ERR(method->classLoader.getConstClass(constPool, cls));
                stackPushObject(cls);
                goto *opcodes[code[pc]];
            }
            case CONST_METHOD_TYPE:
                // TODO
                goto *opcodes[code[pc]];
            case CONST_METHOD_HANDLE:
                // TODO
                goto *opcodes[code[pc]];
            default:
                return ERR_VM_ERROR; // "unkown the const pool tag"
        }
    }
    op_ldc2_w: {
        uint16_t index = ARRAY_TO_INT16(&code[pc + 1]);
        FlintConstPool &constPool = method->classLoader.getConstPool(index);
        pc += 3;
        switch(constPool.tag) {
            case CONST_LONG:
                stackPushInt64(method->classLoader.getConstLong(constPool));
                goto *opcodes[code[pc]];
            case CONST_DOUBLE:
                stackPushDouble(method->classLoader.getConstDouble(constPool));
                goto *opcodes[code[pc]];
            default:
                return ERR_VM_ERROR; // "unkown the const pool tag"
        }
    }
    op_iload:
    op_fload:
        stackPushInt32(locals[code[pc + 1]]);
        pc += 2;
        goto *opcodes[code[pc]];
    op_iload_0:
    op_fload_0:
        stackPushInt32(locals[0]);
        pc++;
        goto *opcodes[code[pc]];
    op_iload_1:
    op_fload_1:
        stackPushInt32(locals[1]);
        pc++;
        goto *opcodes[code[pc]];
    op_iload_2:
    op_fload_2:
        stackPushInt32(locals[2]);
        pc++;
        goto *opcodes[code[pc]];
    op_iload_3:
    op_fload_3:
        stackPushInt32(locals[3]);
        pc++;
        goto *opcodes[code[pc]];
    op_lload:
    op_dload:
        stackPushInt64(*(int64_t *)&locals[code[pc + 1]]);
        pc += 2;
        goto *opcodes[code[pc]];
    op_lload_0:
    op_dload_0:
        stackPushInt64(*(int64_t *)&locals[0]);
        pc++;
        goto *opcodes[code[pc]];
    op_lload_1:
    op_dload_1:
        stackPushInt64(*(int64_t *)&locals[1]);
        pc++;
        goto *opcodes[code[pc]];
    op_lload_2:
    op_dload_2:
        stackPushInt64(*(int64_t *)&locals[2]);
        pc++;
        goto *opcodes[code[pc]];
    op_lload_3:
    op_dload_3:
        stackPushInt64(*(int64_t *)&locals[3]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload:
        stackPushObject((FlintJavaObject *)locals[code[pc + 1]]);
        pc += 2;
        goto *opcodes[code[pc]];
    op_aload_0:
        stackPushObject((FlintJavaObject *)locals[0]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload_1:
        stackPushObject((FlintJavaObject *)locals[1]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload_2:
        stackPushObject((FlintJavaObject *)locals[2]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload_3:
        stackPushObject((FlintJavaObject *)locals[3]);
        pc++;
        goto *opcodes[code[pc]];
    op_iaload:
    op_faload: {
        int32_t index = stackPopInt32();
        FlintJavaObject *obj = stackPopObject();
        if(obj == 0)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj->size / sizeof(int32_t))) {
            RETURN_IF_NOT_THROW(throwArrayIndexOutOfBoundsException(*this, index, obj->size / sizeof(int32_t)));
            goto exception_handler;
        }
        stackPushInt32(((int32_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_laload:
    op_daload: {
        int32_t index = stackPopInt32();
        FlintJavaObject *obj = stackPopObject();
        if(obj == 0)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj->size / sizeof(int64_t))) {
            RETURN_IF_NOT_THROW(throwArrayIndexOutOfBoundsException(*this, index, obj->size / sizeof(int64_t)));
            goto exception_handler;
        }
        stackPushInt64(((int64_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_aaload: {
        int32_t index = stackPopInt32();
        FlintJavaObject *obj = stackPopObject();
        if(obj == 0)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj->size / sizeof(int32_t))) {
            RETURN_IF_NOT_THROW(throwArrayIndexOutOfBoundsException(*this, index, obj->size / sizeof(int32_t)));
            goto exception_handler;
        }
        stackPushObject(((FlintJavaObject **)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_baload: {
        int32_t index = stackPopInt32();
        FlintJavaObject *obj = stackPopObject();
        if(obj == 0)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj->size / sizeof(int8_t))) {
            RETURN_IF_NOT_THROW(throwArrayIndexOutOfBoundsException(*this, index, obj->size / sizeof(int8_t)));
            goto exception_handler;
        }
        stackPushInt32(((int8_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_caload:
    op_saload: {
        int32_t index = stackPopInt32();
        FlintJavaObject *obj = stackPopObject();
        if(obj == 0)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj->size / sizeof(int16_t))) {
            RETURN_IF_NOT_THROW(throwArrayIndexOutOfBoundsException(*this, index, obj->size / sizeof(int16_t)));
            goto exception_handler;
        }
        stackPushInt32(((int16_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_istore:
    op_fstore: {
        uint32_t index = code[pc + 1];
        locals[index] = stackPopInt32();
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_lstore:
    op_dstore: {
        uint32_t index = code[pc + 1];
        *(uint64_t *)&locals[index] = stackPopInt64();
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_astore: {
        uint32_t index = code[pc + 1];
        locals[index] = stackPopInt32();
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_istore_0:
    op_fstore_0: {
        locals[0] = stackPopInt32();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_istore_1:
    op_fstore_1: {
        locals[1] = stackPopInt32();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_istore_2:
    op_fstore_2: {
        locals[2] = stackPopInt32();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_istore_3:
    op_fstore_3: {
        locals[3] = stackPopInt32();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lstore_0:
    op_dstore_0: {
        *(uint64_t *)&locals[0] = stackPopInt64();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lstore_1:
    op_dstore_1: {
        *(uint64_t *)&locals[1] = stackPopInt64();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lstore_2:
    op_dstore_2: {
        *(uint64_t *)&locals[2] = stackPopInt64();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lstore_3:
    op_dstore_3: {
        *(uint64_t *)&locals[3] = stackPopInt64();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_astore_0: {
        locals[0] = stackPopInt32();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_astore_1: {
        locals[1] = stackPopInt32();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_astore_2: {
        locals[2] = stackPopInt32();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_astore_3: {
        locals[3] = stackPopInt32();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iastore:
    op_fastore:
    op_aastore: {
        int32_t value = stackPopInt32();
        int32_t index = stackPopInt32();
        FlintJavaObject *obj = stackPopObject();
        if(obj == 0)
            goto store_null_array_excp;
        else if((index < 0) || (index >= (obj->size / sizeof(int32_t)))) {
            RETURN_IF_NOT_THROW(throwArrayIndexOutOfBoundsException(*this, index, obj->size / sizeof(int32_t)));
            goto exception_handler;
        }
        ((int32_t *)obj->data)[index] = value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lastore:
    op_dastore: {
        int64_t value = stackPopInt64();
        int32_t index = stackPopInt32();
        FlintJavaObject *obj = stackPopObject();
        if(obj == 0)
            goto store_null_array_excp;
        else if((index < 0) || (index >= (obj->size / sizeof(int64_t)))) {
            RETURN_IF_NOT_THROW(throwArrayIndexOutOfBoundsException(*this, index, obj->size / sizeof(int64_t)));
            goto exception_handler;
        }
        ((int64_t *)obj->data)[index] = value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_bastore: {
        int32_t value = stackPopInt32();
        int32_t index = stackPopInt32();
        FlintJavaObject *obj = stackPopObject();
        if(obj == 0)
            goto store_null_array_excp;
        else if((index < 0) || (index >= (obj->size / sizeof(int8_t)))) {
            RETURN_IF_NOT_THROW(throwArrayIndexOutOfBoundsException(*this, index, obj->size / sizeof(int8_t)));
            goto exception_handler;
        }
        ((int8_t *)obj->data)[index] = value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_castore:
    op_sastore: {
        int32_t value = stackPopInt32();
        int32_t index = stackPopInt32();
        FlintJavaObject *obj = stackPopObject();
        if(obj == 0)
            goto store_null_array_excp;
        else if((index < 0) || (index >= (obj->size / sizeof(int16_t)))) {
            RETURN_IF_NOT_THROW(throwArrayIndexOutOfBoundsException(*this, index, obj->size / sizeof(int16_t)));
            goto exception_handler;
        }
        ((int16_t *)obj->data)[index] = value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_pop:
        stackPopInt32();
        pc++;
        goto *opcodes[code[pc]];
    op_pop2:
        stackPopInt64();
        pc++;
        goto *opcodes[code[pc]];
    op_dup: {
        int32_t value = GET_STACK_VALUE(sp);
        stackPushInt32(value);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dup_x1: {
        int32_t value2 = GET_STACK_VALUE(sp - 1);
        int32_t value1 = GET_STACK_VALUE(sp - 0);
        stackPushInt32(value1);
        SET_STACK_VALUE(sp - 1, value2);
        SET_STACK_VALUE(sp - 2, value1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dup_x2: {
        int32_t value3 = GET_STACK_VALUE(sp - 2);
        int32_t value2 = GET_STACK_VALUE(sp - 1);
        int32_t value1 = GET_STACK_VALUE(sp - 0);
        stackPushInt32(value1);
        SET_STACK_VALUE(sp - 1, value2);
        SET_STACK_VALUE(sp - 2, value3);
        SET_STACK_VALUE(sp - 3, value1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dup2: {
        int32_t value2 = GET_STACK_VALUE(sp - 1);
        int32_t value1 = GET_STACK_VALUE(sp - 0);
        stackPushInt32(value2);
        stackPushInt32(value1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dup2_x1: {
        int32_t value3 = GET_STACK_VALUE(sp - 2);
        int32_t value2 = GET_STACK_VALUE(sp - 1);
        int32_t value1 = GET_STACK_VALUE(sp - 0);
        stackPushInt32(value2);
        stackPushInt32(value1);
        SET_STACK_VALUE(sp - 2, value3);
        SET_STACK_VALUE(sp - 3, value1);
        SET_STACK_VALUE(sp - 4, value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dup2_x2: {
        int32_t value4 = GET_STACK_VALUE(sp - 3);
        int32_t value3 = GET_STACK_VALUE(sp - 2);
        int32_t value2 = GET_STACK_VALUE(sp - 1);
        int32_t value1 = GET_STACK_VALUE(sp - 0);
        stackPushInt32(value2);
        stackPushInt32(value1);
        SET_STACK_VALUE(sp - 2, value3);
        SET_STACK_VALUE(sp - 3, value4);
        SET_STACK_VALUE(sp - 4, value1);
        SET_STACK_VALUE(sp - 5, value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iadd: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 + value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ladd: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 + value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fadd: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        stackPushFloat(value1 + value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dadd: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        stackPushDouble(value1 + value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_isub: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 - value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lsub: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 - value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fsub: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        stackPushFloat(value1 - value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dsub: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        stackPushDouble(value1 - value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_imul: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 * value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lmul: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 * value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fmul: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        stackPushFloat(value1 * value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dmul: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        stackPushDouble(value1 * value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_idiv: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        if(value2 == 0)
            goto divided_by_zero_excp;
        stackPushInt32(value1 / value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ldiv: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        if(value2 == 0)
            goto divided_by_zero_excp;
        stackPushInt64(value1 / value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fdiv: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        stackPushFloat(value1 / value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ddiv: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        stackPushDouble(value1 / value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_irem: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        if(value2 == 0)
            goto divided_by_zero_excp;
        stackPushInt32(value1 % value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lrem: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        if(value2 == 0)
            goto divided_by_zero_excp;
        stackPushInt64(value1 % value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_frem: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        int32_t temp = (int32_t)(value1 / value2);
        stackPushFloat(value1 - (temp * value2));
        pc++;
        goto *opcodes[code[pc]];
    }
    op_drem: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        int64_t temp = (int64_t)(value1 / value2);
        stackPushDouble(value1 - (temp * value2));
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ineg:
        stack[sp] = -stack[sp];
        pc++;
        goto *opcodes[code[pc]];
    op_lneg:
        stackPushInt64(-stackPopInt64());
        pc++;
        goto *opcodes[code[pc]];
    op_fneg:
        stackPushFloat(-stackPopFloat());
        pc++;
        goto *opcodes[code[pc]];
    op_dneg:
        stackPushDouble(-stackPopDouble());
        pc++;
        goto *opcodes[code[pc]];
    op_ishl: {
        int32_t position = stackPopInt32();
        int32_t value = stackPopInt32();
        stackPushInt32(value << position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lshl: {
        int32_t position = stackPopInt32();
        int64_t value = stackPopInt64();
        stackPushInt64(value << position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ishr: {
        int32_t position = stackPopInt32();
        int32_t value = stackPopInt32();
        stackPushInt32(value >> position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lshr: {
        int32_t position = stackPopInt32();
        int64_t value = stackPopInt64();
        stackPushInt64(value >> position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iushr: {
        int32_t position = stackPopInt32();
        uint32_t value = stackPopInt32();
        stackPushInt32(value >> position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lushr: {
        int32_t position = stackPopInt32();
        uint64_t value = stackPopInt64();
        stackPushInt64(value >> position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iand: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 & value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_land: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 & value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ior: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 | value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lor: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 | value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ixor: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 ^ value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lxor: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 ^ value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iinc:
        locals[code[pc + 1]] += (int8_t)code[pc + 2];
        pc += 3;
        goto *opcodes[code[pc]];
    op_i2l:
        stackPushInt64(stackPopInt32());
        pc++;
        goto *opcodes[code[pc]];
    op_i2f: {
        float value = stack[sp];
        stack[sp] = *(int32_t *)&value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_i2d:
        stackPushDouble(stackPopInt32());
        pc++;
        goto *opcodes[code[pc]];
    op_l2i:
        stackPushInt32(stackPopInt64());
        pc++;
        goto *opcodes[code[pc]];
    op_l2f:
        stackPushFloat(stackPopInt64());
        pc++;
        goto *opcodes[code[pc]];
    op_l2d:
        stackPushDouble(stackPopInt64());
        pc++;
        goto *opcodes[code[pc]];
    op_f2i:
        stack[sp] = *(float *)&stack[sp];
        pc++;
        goto *opcodes[code[pc]];
    op_f2l:
        stackPushInt64(stackPopFloat());
        pc++;
        goto *opcodes[code[pc]];
    op_f2d:
        stackPushDouble(stackPopFloat());
        pc++;
        goto *opcodes[code[pc]];
    op_d2i:
        stackPushInt32(stackPopDouble());
        pc++;
        goto *opcodes[code[pc]];
    op_d2l:
        stackPushInt64(stackPopDouble());
        pc++;
        goto *opcodes[code[pc]];
    op_d2f:
        stackPushFloat(stackPopDouble());
        pc++;
        goto *opcodes[code[pc]];
    op_i2b:
        stack[sp] = (int8_t)stack[sp];
        pc++;
        goto *opcodes[code[pc]];
    op_i2c:
    op_i2s:
        stack[sp] = (int16_t)stack[sp];
        pc++;
        goto *opcodes[code[pc]];
    op_lcmp: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt32((value1 == value2) ? 0 : ((value1 < value2) ? -1 : 1));
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fcmpl:
    op_fcmpg: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        if((*(uint32_t *)&value1 == FLOAT_NAN) || (*(uint32_t *)&value2 == FLOAT_NAN))
            stackPushInt32((code[pc] == OP_FCMPL) ? -1 : 1);
        else if(value1 > value2)
            stackPushInt32(1);
        else if(value1 == value2)
            stackPushInt32(0);
        else
            stackPushInt32(-1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dcmpl:
    op_dcmpg: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        if((*(uint64_t *)&value1 == DOUBLE_NAN) || (*(uint64_t *)&value2 == DOUBLE_NAN))
            stackPushInt32((code[pc] == OP_DCMPL) ? -1 : 1);
        else if(value1 > value2)
            stackPushInt32(1);
        else if(value1 == value2)
            stackPushInt32(0);
        else
            stackPushInt32(-1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ifeq:
    op_ifnull:
        pc += (!stackPopInt32()) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_ifne:
    op_ifnonnull:
        pc += stackPopInt32() ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_iflt:
        pc += (stackPopInt32() < 0) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_ifge:
        pc += (stackPopInt32() >= 0) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_ifgt:
        pc += (stackPopInt32() > 0) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_ifle:
        pc += (stackPopInt32() <= 0) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_if_icmpeq:
    op_if_acmpeq: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 == value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmpne:
    op_if_acmpne: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 != value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmplt: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 < value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmpge: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 >= value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmpgt: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 > value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmple: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 <= value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_goto:
        pc += ARRAY_TO_INT16(&code[pc + 1]);
        goto *opcodes[code[pc]];
    op_goto_w:
        pc += ARRAY_TO_INT32(&code[pc + 1]);
        goto *opcodes[code[pc]];
    op_jsr:
        stackPushInt32(pc + 3);
        pc += ARRAY_TO_INT16(&code[pc + 1]);
        goto *opcodes[code[pc]];
    op_jsrw:
        stackPushInt32(pc + 5);
        pc += ARRAY_TO_INT32(&code[pc + 1]);
        goto *opcodes[code[pc]];
    op_ret:
        pc = locals[code[pc + 1]];
        goto *opcodes[code[pc]];
    op_tableswitch: {
        int32_t index = stackPopInt32();
        uint8_t padding = (4 - ((pc + 1) % 4)) % 4;
        const uint8_t *table = &code[pc + padding + 1];
        int32_t low = ARRAY_TO_INT32(&table[4]);
        int32_t height = ARRAY_TO_INT32(&table[8]);
        if(index < low || index > height) {
            int32_t defaultOffset = ARRAY_TO_INT32(table);
            pc += defaultOffset;
            goto *opcodes[code[pc]];
        }
        table = &table[12 + (index - low) * 4];
        pc += ARRAY_TO_INT32(table);
        goto *opcodes[code[pc]];
    }
    op_lookupswitch: {
        int32_t key = stackPopInt32();
        uint8_t padding = (4 - ((pc + 1) % 4)) % 4;
        const uint8_t *table = &code[pc + padding + 1];
        int32_t defaultPc = ARRAY_TO_INT32(table);
        int32_t npairs = ARRAY_TO_INT32(&table[4]);
        table = &table[8];
        while(npairs--) {
            int32_t pairs = ARRAY_TO_INT32(table);
            if(key == pairs) {
                pc += ARRAY_TO_INT32(&table[4]);
                goto *opcodes[code[pc]];
            }
            table = &table[8];
        }
        pc += defaultPc;
        goto *opcodes[code[pc]];
    }
    op_ireturn:
    op_freturn: {
        int32_t retVal = stackPopInt32();
        stackRestoreContext();
        stackPushInt32(retVal);
        pc = lr;
        goto *opcodes[code[pc]];
    }
    op_lreturn:
    op_dreturn: {
        int64_t retVal = stackPopInt64();
        stackRestoreContext();
        stackPushInt64(retVal);
        pc = lr;
        goto *opcodes[code[pc]];
    }
    op_areturn: {
        int32_t retVal = (int32_t)stackPopObject();
        stackRestoreContext();
        stackPushObject((FlintJavaObject *)retVal);
        pc = lr;
        goto *opcodes[code[pc]];
    }
    op_return: {
        stackRestoreContext();
        peakSp = sp;
        pc = lr;
        goto *opcodes[code[pc]];
    }
    op_getstatic: {
        FlintConstField *constField;
        RETURN_IF_ERR(method->classLoader.getConstField(ARRAY_TO_INT16(&code[pc + 1]), constField));
        FlintClassData *classData;
        FlintError err = flint.load(constField->className, (FlintClassLoader *&)classData);
        if(err != ERR_OK) {
            RETURN_IF_NOT_THROW(checkAndThrowForFlintError(*this, err, &constField->className));
            goto exception_handler;
        }
        FlintInitStatus initStatus = classData->getInitStatus();
        if(initStatus == INITIALIZED || (initStatus == INITIALIZING && classData->staticInitOwnId == (uint32_t)this)) {
            FlintFieldsData *fields = classData->staticFieldsData;
            switch(constField->nameAndType.descriptor.text[0]) {
                case 'J':
                case 'D': {
                    FlintFieldData64 *fieldData = fields->getFieldData64(*constField);
                    if(fieldData == NULL) {
                        RETURN_IF_NOT_THROW(throwNoSuchFieldError(*this, constField->className.text, constField->nameAndType.name.text));
                        goto exception_handler;
                    }
                    stackPushInt64(fieldData->value);
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                case 'L':
                case '[': {
                    FlintFieldObject *fieldData = fields->getFieldObject(*constField);
                    if(fieldData == NULL) {
                        RETURN_IF_NOT_THROW(throwNoSuchFieldError(*this, constField->className.text, constField->nameAndType.name.text));
                        goto exception_handler;
                    }
                    stackPushObject(fieldData->object);
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                default: {
                    FlintFieldData32 *fieldData = fields->getFieldData32(*constField);
                    if(fieldData == NULL) {
                        RETURN_IF_NOT_THROW(throwNoSuchFieldError(*this, constField->className.text, constField->nameAndType.name.text));
                        goto exception_handler;
                    }
                    stackPushInt32(fieldData->value);
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
            }
        }
        else if(initStatus == UNINITIALIZED) {
            FlintError err = invokeStaticCtor(*classData);
            if(err != ERR_OK) {
                if(err == ERR_THROW)
                    goto exception_handler;
                return err;
            }
        }
        goto *opcodes[code[pc]];
    }
    op_putstatic: {
        FlintConstField *constField;
        RETURN_IF_ERR(method->classLoader.getConstField(ARRAY_TO_INT16(&code[pc + 1]), constField));
        FlintClassData *classData;
        FlintError err = flint.load(constField->className, (FlintClassLoader *&)classData);
        if(err != ERR_OK) {
            RETURN_IF_NOT_THROW(checkAndThrowForFlintError(*this, err, &constField->className));
            goto exception_handler;
        }
        FlintInitStatus initStatus = classData->getInitStatus();
        if(initStatus == INITIALIZED || (initStatus == INITIALIZING && classData->staticInitOwnId == (uint32_t)this)) {
            FlintFieldsData *fields = classData->staticFieldsData;
            switch(constField->nameAndType.descriptor.text[0]) {
                case 'Z':
                case 'B': {
                    FlintFieldData32 *fieldData = fields->getFieldData32(*constField);
                    if(fieldData == NULL) {
                        RETURN_IF_NOT_THROW(throwNoSuchFieldError(*this, constField->className.text, constField->nameAndType.name.text));
                        goto exception_handler;
                    }
                    fieldData->value = (int8_t)stackPopInt32();
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                case 'C':
                case 'S': {
                    FlintFieldData32 *fieldData = fields->getFieldData32(*constField);
                    if(fieldData == NULL) {
                        RETURN_IF_NOT_THROW(throwNoSuchFieldError(*this, constField->className.text, constField->nameAndType.name.text));
                        goto exception_handler;
                    }
                    fieldData->value = (int16_t)stackPopInt32();
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                case 'J':
                case 'D': {
                    FlintFieldData64 *fieldData = fields->getFieldData64(*constField);
                    if(fieldData == NULL) {
                        RETURN_IF_NOT_THROW(throwNoSuchFieldError(*this, constField->className.text, constField->nameAndType.name.text));
                        goto exception_handler;
                    }
                    fieldData->value = stackPopInt64();
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                case 'L':
                case '[': {
                    FlintFieldObject *fieldData = fields->getFieldObject(*constField);
                    if(fieldData == NULL) {
                        RETURN_IF_NOT_THROW(throwNoSuchFieldError(*this, constField->className.text, constField->nameAndType.name.text));
                        goto exception_handler;
                    }
                    fieldData->object = stackPopObject();
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                default: {
                    FlintFieldData32 *fieldData = fields->getFieldData32(*constField);
                    if(fieldData == NULL) {
                        RETURN_IF_NOT_THROW(throwNoSuchFieldError(*this, constField->className.text, constField->nameAndType.name.text));
                        goto exception_handler;
                    }
                    fieldData->value = stackPopInt32();
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
            }
        }
        else if(initStatus == UNINITIALIZED) {
            FlintError err = invokeStaticCtor(*classData);
            if(err != ERR_OK) {
                if(err == ERR_THROW)
                    goto exception_handler;
                return err;
            }
        }
        goto *opcodes[code[pc]];
    }
    op_getfield: {
        FlintConstField *constField;
        RETURN_IF_ERR(method->classLoader.getConstField(ARRAY_TO_INT16(&code[pc + 1]), constField));
        switch(constField->nameAndType.descriptor.text[0]) {
            case 'J':
            case 'D': {
                FlintJavaObject *obj = stackPopObject();
                if(obj == 0) {
                    RETURN_IF_NOT_THROW(throwNullPointerException(*this, *constField));
                    goto exception_handler;
                }
                FlintFieldData64 *fieldData = obj->getFields().getFieldData64(*constField);
                if(fieldData == NULL) {
                    RETURN_IF_NOT_THROW(throwNoSuchFieldError(*this, constField->className.text, constField->nameAndType.name.text));
                    goto exception_handler;
                }
                stackPushInt64(fieldData->value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'L':
            case '[': {
                FlintJavaObject *obj = stackPopObject();
                if(obj == 0) {
                    RETURN_IF_NOT_THROW(throwNullPointerException(*this, *constField));
                    goto exception_handler;
                }
                FlintFieldObject *fieldData = obj->getFields().getFieldObject(*constField);
                if(fieldData == NULL) {
                    RETURN_IF_NOT_THROW(throwNoSuchFieldError(*this, constField->className.text, constField->nameAndType.name.text));
                    goto exception_handler;
                }
                stackPushObject(fieldData->object);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            default: {
                FlintJavaObject *obj = stackPopObject();
                if(obj == 0) {
                    RETURN_IF_NOT_THROW(throwNullPointerException(*this, *constField));
                }
                FlintFieldData32 *fieldData = obj->getFields().getFieldData32(*constField);
                if(fieldData == NULL) {
                    RETURN_IF_NOT_THROW(throwNoSuchFieldError(*this, constField->className.text, constField->nameAndType.name.text));
                    goto exception_handler;
                }
                stackPushInt32(fieldData->value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
        }
    }
    op_putfield: {
        FlintConstField *constField;
        RETURN_IF_ERR(method->classLoader.getConstField(ARRAY_TO_INT16(&code[pc + 1]), constField));
        switch(constField->nameAndType.descriptor.text[0]) {
            case 'Z':
            case 'B': {
                int32_t value = stackPopInt32();
                FlintJavaObject *obj = stackPopObject();
                if(obj == 0) {
                    RETURN_IF_NOT_THROW(throwNullPointerException(*this, *constField));
                    goto exception_handler;
                }
                FlintFieldData32 *fieldData = obj->getFields().getFieldData32(*constField);
                if(fieldData == NULL) {
                    RETURN_IF_NOT_THROW(throwNoSuchFieldError(*this, constField->className.text, constField->nameAndType.name.text));
                    goto exception_handler;
                }
                fieldData->value = (int8_t)value;
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'C':
            case 'S': {
                int32_t value = stackPopInt32();
                FlintJavaObject *obj = stackPopObject();
                if(obj == 0) {
                    RETURN_IF_NOT_THROW(throwNullPointerException(*this, *constField));
                    goto exception_handler;
                }
                FlintFieldData32 *fieldData = obj->getFields().getFieldData32(*constField);
                if(fieldData == NULL) {
                    RETURN_IF_NOT_THROW(throwNoSuchFieldError(*this, constField->className.text, constField->nameAndType.name.text));
                    goto exception_handler;
                }
                fieldData->value = (int16_t)value;
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'J':
            case 'D': {
                int64_t value = stackPopInt64();
                FlintJavaObject *obj = stackPopObject();
                if(obj == 0) {
                    RETURN_IF_NOT_THROW(throwNullPointerException(*this, *constField));
                    goto exception_handler;
                }
                FlintFieldData64 *fieldData = obj->getFields().getFieldData64(*constField);
                if(fieldData == NULL) {
                    RETURN_IF_NOT_THROW(throwNoSuchFieldError(*this, constField->className.text, constField->nameAndType.name.text));
                    goto exception_handler;
                }
                fieldData->value = value;
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'L':
            case '[': {
                FlintJavaObject *value = stackPopObject();
                FlintJavaObject *obj = stackPopObject();
                if(obj == 0) {
                    RETURN_IF_NOT_THROW(throwNullPointerException(*this, *constField));
                    goto exception_handler;
                }
                FlintFieldObject *fieldData = obj->getFields().getFieldObject(*constField);
                if(fieldData == NULL) {
                    RETURN_IF_NOT_THROW(throwNoSuchFieldError(*this, constField->className.text, constField->nameAndType.name.text));
                    goto exception_handler;
                }
                fieldData->object = value;
                pc += 3;
                goto *opcodes[code[pc]];
            }
            default: {
                int32_t value = stackPopInt32();
                FlintJavaObject *obj = stackPopObject();
                if(obj == 0) {
                    RETURN_IF_NOT_THROW(throwNullPointerException(*this, *constField));
                    goto exception_handler;
                }
                FlintFieldData32 *fieldData = obj->getFields().getFieldData32(*constField);
                if(fieldData == NULL) {
                    RETURN_IF_NOT_THROW(throwNoSuchFieldError(*this, constField->className.text, constField->nameAndType.name.text));
                    goto exception_handler;
                }
                fieldData->value = value;
                pc += 3;
                goto *opcodes[code[pc]];
            }
        }
    }
    op_invokevirtual: {
        FlintConstMethod *constMethod;
        RETURN_IF_ERR(method->classLoader.getConstMethod(ARRAY_TO_INT16(&code[pc + 1]), constMethod));
        FlintError err = invokeVirtual(*constMethod);
        if(err != ERR_OK) {
            if(err == ERR_THROW)
                goto exception_handler;
            return err;
        }
        goto *opcodes[code[pc]];
    }
    op_invokespecial: {
        FlintConstMethod *constMethod;
        RETURN_IF_ERR(method->classLoader.getConstMethod(ARRAY_TO_INT16(&code[pc + 1]), constMethod));
        FlintError err = invokeSpecial(*constMethod);
        if(err != ERR_OK) {
            if(err == ERR_THROW)
                goto exception_handler;
            return err;
        }
        goto *opcodes[code[pc]];
    }
    op_invokestatic: {
        FlintConstMethod *constMethod;
        RETURN_IF_ERR(method->classLoader.getConstMethod(ARRAY_TO_INT16(&code[pc + 1]), constMethod));
        FlintError err = invokeStatic(*constMethod);
        if(err != ERR_OK) {
            if(err == ERR_THROW)
                goto exception_handler;
            return err;
        }
        goto *opcodes[code[pc]];
    }
    op_invokeinterface: {
        FlintConstInterfaceMethod *interfaceMethod;
        RETURN_IF_ERR(method->classLoader.getConstInterfaceMethod(ARRAY_TO_INT16(&code[pc + 1]), interfaceMethod));
        uint8_t count = code[pc + 3];
        FlintError err = invokeInterface(*interfaceMethod, count);
        if(err != ERR_OK) {
            if(err == ERR_THROW)
                goto exception_handler;
            return err;
        }
        goto *opcodes[code[pc]];
    }
    op_invokedynamic: {
        // TODO
        // goto *opcodes[code[pc]];
        RETURN_IF_NOT_THROW(throwUnsupportedOperationException(*this, "Invokedynamic instructions are not supported"));
        goto exception_handler;
    }
    op_new: {
        uint16_t poolIndex = ARRAY_TO_INT16(&code[pc + 1]);
        FlintConstUtf8 &constClass = method->classLoader.getConstUtf8Class(poolIndex);
        FlintJavaObject *obj;
        RETURN_IF_ERR(flint.newObject(sizeof(FlintFieldsData), constClass, 0, obj));
        memset(obj->data, 0, sizeof(FlintFieldsData));
        FlintClassData *classData;
        FlintError err = flint.load(constClass, (FlintClassLoader *&)classData);
        if(err != ERR_OK) {
            RETURN_IF_NOT_THROW(checkAndThrowForFlintError(*this, err, &constClass));
            goto exception_handler;
        }
        new ((FlintFieldsData *)obj->data)FlintFieldsData();
        FlintConstUtf8 *classError;
        err = ((FlintFieldsData *)obj->data)->loadNonStatic(flint, *classData, classError);
        if(err != ERR_OK)
            return checkAndThrowForFlintError(*this, err, classError);
        stackPushObject(obj);
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_newarray: {
        int32_t count = stackPopInt32();
        if(count < 0)
            goto negative_array_size_excp;
        uint8_t atype = code[pc + 1];
        uint8_t typeSize = FlintJavaObject::getPrimitiveTypeSize(atype);
        FlintJavaObject *obj;
        RETURN_IF_ERR(flint.newObject(typeSize * count, *primTypeConstUtf8List[atype - 4], 1, obj));
        memset(obj->data, 0, obj->size);
        stackPushObject(obj);
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_anewarray: {
        int32_t count = stackPopInt32();
        if(count < 0)
            goto negative_array_size_excp;
        uint16_t poolIndex = ARRAY_TO_INT16(&code[pc + 1]);
        FlintConstUtf8 &constClass = method->classLoader.getConstUtf8Class(poolIndex);
        FlintObjectArray *array;
        RETURN_IF_ERR(flint.newObjectArray(constClass, count, array));
        memset(array->data, 0, array->size);
        stackPushObject(array);
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_arraylength: {
        FlintJavaObject *obj = stackPopObject();
        if(obj == 0) {
            RETURN_IF_NOT_THROW(throwNullPointerException(*this, "Cannot read the array length from null object"));
            goto exception_handler;
        }
        stackPushInt32(obj->size / obj->parseTypeSize());
        pc++;
        goto *opcodes[code[pc]];
    }
    op_athrow: {
        FlintJavaObject *obj = (FlintJavaObject *)stack[sp];
        if(obj == 0) {
            stackPopObject();
            RETURN_IF_NOT_THROW(throwNullPointerException(*this, "Cannot throw exception by null object"));
        }
        goto exception_handler;
    }
    exception_handler: {
        uint32_t tracePc = pc;
        int32_t traceStartSp = startSp;
        FlintMethodInfo *traceMethod = method;
        FlintJavaObject *obj = stackPopObject();
        if(dbg && dbg->exceptionIsEnabled())
            dbg->caughtException(this, (FlintJavaThrowable *)obj);
        while(1) {
            uint16_t exceptionLength = traceMethod->getExceptionLength();
            for(uint16_t i = 0; i < exceptionLength; i++) {
                FlintExceptionTable *exception = traceMethod->getException(i);
                if(exception->startPc <= tracePc && tracePc < exception->endPc) {
                    bool isMatch = false;
                    if(exception->catchType == 0)
                        isMatch = true;
                    else {
                        FlintConstUtf8 *classError;
                        FlintError err = flint.isInstanceof(obj, traceMethod->classLoader.getConstUtf8Class(exception->catchType), &classError);
                        if(err == ERR_OK)
                            isMatch = true;
                        else if(err != ERR_IS_INSTANCE_FALSE)
                            return checkAndThrowForFlintError(*this, err, classError);
                    }
                    if(isMatch) {
                        while(startSp > traceStartSp)
                            stackRestoreContext();
                        sp = startSp + traceMethod->getMaxLocals();
                        stackPushObject(obj);
                        pc = exception->handlerPc;
                        goto *opcodes[code[pc]];
                    }
                }
            }
            if(traceStartSp < 4) {
                if(dbg && !dbg->exceptionIsEnabled())
                    dbg->caughtException(this, (FlintJavaThrowable *)obj);
                stackPushObject(obj);
                return ERR_THROW;
            }
            traceMethod = (FlintMethodInfo *)stack[traceStartSp - 3];
            tracePc = stack[traceStartSp - 2];
            traceStartSp = stack[traceStartSp];
        }
    }
    op_checkcast: {
        FlintJavaObject *obj = (FlintJavaObject *)stack[sp];
        FlintConstUtf8 &type = method->classLoader.getConstUtf8Class(ARRAY_TO_INT16(&code[pc + 1]));
        if(obj != 0) {
            FlintConstUtf8 *classError;
            FlintError err = flint.isInstanceof(obj, type, &classError);
            if(err != ERR_OK) {
                if(err == ERR_IS_INSTANCE_FALSE) {
                    RETURN_IF_NOT_THROW(throwClassCastException(*this, obj, type));
                    goto exception_handler;
                }
                RETURN_IF_NOT_THROW(checkAndThrowForFlintError(*this, err, classError));
                goto exception_handler;
            }
        }
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_instanceof: {
        FlintJavaObject *obj = stackPopObject();
        FlintConstUtf8 &type = method->classLoader.getConstUtf8Class(ARRAY_TO_INT16(&code[pc + 1]));
        FlintConstUtf8 *classError;
        FlintError err = flint.isInstanceof(obj, type, &classError);
        if(err == ERR_OK)
            stackPushInt32(1);
        else if(err == ERR_IS_INSTANCE_FALSE)
            stackPushInt32(0);
        else {
            RETURN_IF_NOT_THROW(checkAndThrowForFlintError(*this, err, classError));
            goto exception_handler;
        }
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_monitorenter: {
        FlintJavaObject *obj = stackPopObject();
        if(obj == 0) {
            RETURN_IF_NOT_THROW(throwNullPointerException(*this, "Cannot enter synchronized block by null object"));
            goto exception_handler;
        }
        FlintError err = lockObject(obj);
        if(err == ERR_LOCK_FAIL) {
            FlintAPI::Thread::yield();
            goto *opcodes[code[pc]];
        }
        RETURN_IF_ERR(err);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_monitorexit: {
        FlintJavaObject *obj = stackPopObject();
        unlockObject(obj);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_wide: {
        switch((FlintOpCode)code[pc + 1]) {
            case OP_IINC: {
                uint16_t index = ARRAY_TO_INT16(&code[pc + 2]);
                locals[index] += ARRAY_TO_INT16(&code[pc + 4]);
                pc += 6;
                goto *opcodes[code[pc]];
            }
            case OP_ALOAD: {
                uint16_t index = ARRAY_TO_INT16(&code[pc + 2]);
                stackPushObject((FlintJavaObject *)locals[index]);
                pc += 4;
                goto *opcodes[code[pc]];
            }
            case OP_FLOAD:
            case OP_ILOAD: {
                uint16_t index = ARRAY_TO_INT16(&code[pc + 2]);
                stackPushInt32(locals[index]);
                pc += 4;
                goto *opcodes[code[pc]];
            }
            case OP_LLOAD:
            case OP_DLOAD: {
                uint16_t index = ARRAY_TO_INT16(&code[pc + 2]);
                stackPushInt64(*(int64_t *)&locals[index]);
                pc += 4;
                goto *opcodes[code[pc]];
            }
            case OP_ASTORE: {
                uint16_t index = ARRAY_TO_INT16(&code[pc + 2]);
                locals[index] = stackPopInt32();
                pc += 4;
                goto *opcodes[code[pc]];
            }
            case OP_FSTORE:
            case OP_ISTORE: {
                uint16_t index = ARRAY_TO_INT16(&code[pc + 2]);
                locals[index] = stackPopInt32();
                pc += 4;
                goto *opcodes[code[pc]];
            }
            case OP_LSTORE:
            case OP_DSTORE: {
                uint16_t index = ARRAY_TO_INT16(&code[pc + 2]);
                *(uint64_t *)&locals[index] = stackPopInt64();
                pc += 4;
                goto *opcodes[code[pc]];
            }
            case OP_RET: {
                uint16_t index = ARRAY_TO_INT16(&code[pc + 2]);
                pc = locals[index];
                goto *opcodes[code[pc]];
            }
            default:
                goto op_unknow;
        }
    }
    op_multianewarray: {
        FlintConstUtf8 *typeName = &method->classLoader.getConstUtf8Class(ARRAY_TO_INT16(&code[pc + 1]));
        uint8_t dimensions = code[pc + 3];
        const char *typeNameText = typeName->text;
        uint32_t length = typeName->length - dimensions;
        if(typeNameText[dimensions] != 'L') {
            uint8_t atype = FlintJavaObject::convertToAType(typeNameText[dimensions]);
            if(atype == 0)
                return throwIllegalArgumentException(*this, "invalid primative type");
            typeName = (FlintConstUtf8 *)primTypeConstUtf8List[atype - 4];
        }
        else {
            FlintClassLoader *loader;
            FlintError err = flint.load(&typeNameText[dimensions + 1], length - 2, loader);
            if(err != ERR_OK) {
                RETURN_IF_NOT_THROW(checkAndThrowForFlintError(*this, err, &typeNameText[dimensions + 1], length - 2));
                goto exception_handler;
            }
            typeName = loader->thisClass;
        }
        sp -= dimensions - 1;
        for(int32_t i = 0; i < dimensions; i++) {
            if(stack[sp + i] < 0)
                goto negative_array_size_excp;
        }
        FlintJavaObject *array;
        RETURN_IF_ERR(flint.newMultiArray(*typeName, &stack[sp], dimensions, 1, array));
        stackPushObject(array);
        pc += 4;
        goto *opcodes[code[pc]];
    }
    op_breakpoint: {
        if(!dbg) {
            pc++;
            goto *opcodes[code[pc]];
        }
        dbg->hitBreakpoint(this);
        if(hasTerminateRequest())
            return ERR_TERMINATE_REQUEST;

        if(hitBkpMethod != NULL)
            dbg->restoreBreakPoint(hitBkpPc, hitBkpMethod);
        hitBkpPc = this->pc;
        hitBkpMethod = this->method;
        dbg->restoreOriginalOpcode(hitBkpPc, hitBkpMethod);
        opcodes = opcodeRestoreBkpLabels;
        goto *opcodeLabels[code[pc]];
    }
    restore_bkp: {
        dbg->restoreBreakPoint(hitBkpPc, hitBkpMethod);
        hitBkpMethod = NULL;
        opcodes = opcodeLabelsStop;
        goto *opcodes[code[pc]];
    }
    op_unknow:
        return ERR_VM_ERROR; // "unknow opcode"
    divided_by_zero_excp: {
        RETURN_IF_NOT_THROW(throwArithmeticException(*this, "Divided by zero"));
        goto exception_handler;
    }
    negative_array_size_excp: {
        RETURN_IF_NOT_THROW(throwNegativeArraySizeException(*this, "Size of the array is a negative number"));
        goto exception_handler;
    }
    load_null_array_excp: {
        RETURN_IF_NOT_THROW(throwNullPointerException(*this, "Cannot load from null array object"));
        goto exception_handler;
    }
    store_null_array_excp: {
        RETURN_IF_NOT_THROW(throwNullPointerException(*this, "Cannot store to null array object"));
        goto exception_handler;
    }
    op_exit:
        return hasTerminateRequest() ? ERR_TERMINATE_REQUEST : ERR_OK;
}

void FlintExecution::runTask(FlintExecution *execution) {
    FlintError err = execution->run();
    switch(err) {
        case ERR_OK:
        case ERR_TERMINATE_REQUEST:
            break;
        case ERR_THROW: {
            FlintJavaThrowable *ex = (FlintJavaThrowable *)execution->stackPopObject();
            FlintJavaString *str = ex->getDetailMessage();
            if(str)
                execution->flint.println(str);
            else
                execution->flint.println(ex->type);
            break;
        }
        case ERR_OUT_OF_MEMORY:
            execution->flint.println("Out of memory");
            break;
        case ERR_STACK_OVERFLOW:
            execution->flint.println("Stack overflow");
            break;
        case ERR_CLASS_LOAD_FAIL:
            execution->flint.println("Class load fail");
            break;
        case ERR_CLASS_NOT_FOUND:
            execution->flint.println("Class not found");
            break;
        case ERR_FIELD_NOT_FOUND:
            execution->flint.println("Field not found");
            break;
        case ERR_METHOD_NOT_FOUND:
            execution->flint.println("Method not found");
            break;
        case ERR_VM_ERROR:
            execution->flint.println("VM error");
            break;
        default:
            break;
    }
    while(execution->startSp > 3)
        execution->stackRestoreContext();
    execution->peakSp = -1;
    execution->flint.freeExecution(*execution);
    FlintAPI::Thread::terminate(0);
}

bool FlintExecution::run(FlintMethodInfo *method) {
    this->method = method;
    if(!opcodes)
        return (FlintAPI::Thread::create((void (*)(void *))runTask, (void *)this) != 0);
    return false;
}

void FlintExecution::stopRequest(void) {
    opcodes = opcodeLabelsStop;
}

void FlintExecution::terminateRequest(void) {
    opcodes = opcodeLabelsExit;
}

bool FlintExecution::hasTerminateRequest(void) const {
    return (opcodes == opcodeLabelsExit);
}

FlintError FlintExecution::getOnwerThread(FlintJavaThread *&thread) {
    FlintError err = ERR_OK;
    if(onwerThread == NULL) {
        Flint::lock();
        err = flint.newObject(*(FlintConstUtf8 *)threadClassName, (FlintJavaObject *&)onwerThread);
        Flint::unlock();
    }
    thread = onwerThread;
    return err;
}

FlintExecution::~FlintExecution(void) {
    Flint::free(stack);
}
