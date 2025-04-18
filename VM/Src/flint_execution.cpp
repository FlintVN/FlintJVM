
#include <iostream>
#include <string.h>
#include "flint.h"
#include "flint_opcodes.h"
#include "flint_execution.h"
#include "flint_const_name_base.h"
#include "flint_system_api.h"

#if __has_include("flint_conf.h")
#include "flint_conf.h"
#endif
#include "flint_default_conf.h"

#define FLOAT_NAN                           0x7FC00000
#define DOUBLE_NAN                          0x7FF8000000000000

#define ARRAY_TO_INT16(array)               (int16_t)(((array)[0] << 8) | (array)[1])
#define ARRAY_TO_INT32(array)               (int32_t)(((array)[0] << 24) | ((array)[1] << 16) | ((array)[2] << 8) | (array)[3])

#define GET_STACK_VALUE(_index)             stack[_index]
#define SET_STACK_VALUE(_index, _value)     stack[_index] = _value

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

void FlintExecution::initNewContext(FlintMethodInfo &methodInfo, uint16_t argc) {
    FlintCodeAttribute &attributeCode = methodInfo.getAttributeCode();
    if((sp + attributeCode.maxLocals + attributeCode.maxStack + 4) >= stackLength)
        throw (FlintOutOfMemoryError *)"Stack overflow";

    /* Save current context */
    stack[++sp] = (int32_t)method;
    stack[++sp] = pc;
    stack[++sp] = lr;
    stack[++sp] = startSp;
    startSp = sp;

    method = &methodInfo;
    code = attributeCode.code;
    pc = 0;
    locals = &stack[sp + 1];
    for(uint32_t i = argc; i < attributeCode.maxLocals; i++) {
        uint32_t index = sp + i + 1;
        stack[index] = 0;
    }
    sp += attributeCode.maxLocals;
}

void FlintExecution::stackInitExitPoint(uint32_t exitPc) {
    int32_t argc = sp + 1;
    for(uint32_t i = 0; i < argc; i++)
        SET_STACK_VALUE(sp - i + 4, GET_STACK_VALUE(sp - i));
    sp -= argc;
    pc = lr = exitPc;
    initNewContext(*method, argc);
}

void FlintExecution::stackRestoreContext(void) {
    if(method->accessFlag & METHOD_SYNCHRONIZED) {
        if(method->accessFlag & METHOD_STATIC) {
            if(&method->name == &staticConstructorName) {
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
    code = method->getAttributeCode().code;
    locals = &stack[startSp + 1];
}

bool FlintExecution::lockClass(FlintClassData &cls) {
    Flint::lock();
    if(cls.monitorCount == 0 || cls.ownId == (int32_t)this) {
        cls.ownId = (int32_t)this;
        if(cls.monitorCount < 0xFFFFFFFF) {
            cls.monitorCount++;
            Flint::unlock();
            return true;
        }
        Flint::unlock();
        throw "monitorCount limit has been reached";
    }
    Flint::unlock();
    return false;
}

void FlintExecution::unlockClass(FlintClassData &cls) {
    Flint::lock();
    if(cls.monitorCount)
        cls.monitorCount--;
    Flint::unlock();
}

bool FlintExecution::lockObject(FlintJavaObject *obj) {
    Flint::lock();
    if(obj->monitorCount == 0 || obj->ownId == (int32_t)this) {
        obj->ownId = (int32_t)this;
        if(obj->monitorCount < 0xFFFFFF) {
            obj->monitorCount++;
            Flint::unlock();
            return true;
        }
        Flint::unlock();
        throw "monitorCount limit has been reached";
    }
    Flint::unlock();
    return false;
}

void FlintExecution::unlockObject(FlintJavaObject *obj) {
    Flint::lock();
    if(obj->monitorCount)
        obj->monitorCount--;
    Flint::unlock();
}

void FlintExecution::invoke(FlintMethodInfo &methodInfo, uint8_t argc) {
    if(!(methodInfo.accessFlag & METHOD_NATIVE)) {
        peakSp = sp + 4;
        for(uint32_t i = 0; i < argc; i++)
            SET_STACK_VALUE(sp - i + 4, GET_STACK_VALUE(sp - i));
        sp -= argc;
        initNewContext(methodInfo, argc);
    }
    else {
        int32_t retSp = sp - argc;
        FlintNativeAttribute &attrNative = methodInfo.getAttributeNative();
        attrNative.nativeMethod(*this);
        uint8_t retType = methodInfo.descriptor.text[methodInfo.descriptor.length - 1];
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
    }
}

void FlintExecution::invokeStatic(FlintConstMethod &constMethod) {
    if(constMethod.methodInfo == 0)
        constMethod.methodInfo = &flint.findMethod(constMethod);
    FlintMethodInfo &methodInfo = *constMethod.methodInfo;
    FlintClassData &classData = (FlintClassData &)methodInfo.classLoader;
    if(classData.hasStaticCtor()) {
        FlintInitStatus initStatus = classData.getInitStatus();
        if(initStatus == UNINITIALIZED)
            return invokeStaticCtor(classData);
        else if((initStatus == INITIALIZING) && (classData.staticInitOwnId != (uint32_t)this))
            return FlintAPI::Thread::yield();
    }
    if((methodInfo.accessFlag & METHOD_SYNCHRONIZED) && !lockClass(classData)) {
        FlintAPI::Thread::yield();
        return;
    }
    lr = pc + 3;
    invoke(methodInfo, constMethod.getArgc());
}

void FlintExecution::invokeSpecial(FlintConstMethod &constMethod) {
    uint8_t argc = constMethod.getArgc() + 1;
    if(constMethod.methodInfo == 0)
        constMethod.methodInfo = &flint.findMethod(constMethod);
    FlintMethodInfo &methodInfo = *constMethod.methodInfo;
    FlintClassData &classData = (FlintClassData &)methodInfo.classLoader;
    if(classData.hasStaticCtor()) {
        FlintInitStatus initStatus = classData.getInitStatus();
        if(initStatus == UNINITIALIZED)
            return invokeStaticCtor(classData);
        else if((initStatus == INITIALIZING) && (classData.staticInitOwnId != (uint32_t)this))
            return FlintAPI::Thread::yield();
    }
    if((methodInfo.accessFlag & METHOD_SYNCHRONIZED) && !lockObject((FlintJavaObject *)stack[sp - argc - 1])) {
        FlintAPI::Thread::yield();
        return;
    }
    lr = pc + 3;
    invoke(methodInfo, argc);
}

void FlintExecution::invokeVirtual(FlintConstMethod &constMethod) {
    uint8_t argc = constMethod.getArgc();
    FlintJavaObject *obj = (FlintJavaObject *)stack[sp - argc];
    if(obj == 0) {
        const char *msg[] = {"Cannot invoke ", constMethod.className.text, ".", constMethod.nameAndType.name.text, " by null object"};
        throw &flint.newNullPointerException(&flint.newString(msg, LENGTH(msg)));
    }
    FlintConstUtf8 &type = (obj->dimensions > 0 || FlintJavaObject::isPrimType(obj->type)) ? (FlintConstUtf8 &)objectClassName : obj->type;
    FlintMethodInfo *methodInfo;
    if(constMethod.methodInfo && constMethod.methodInfo->classLoader.getThisClass() == type)
        methodInfo = constMethod.methodInfo;
    else {
        constMethod.methodInfo = &flint.findMethod(type, constMethod.nameAndType);
        methodInfo = constMethod.methodInfo;
    }
    if((methodInfo->accessFlag & METHOD_SYNCHRONIZED) && !lockObject(obj)) {
        FlintAPI::Thread::yield();
        return;
    }
    argc++;
    lr = pc + 3;
    invoke(*methodInfo, argc);
}

void FlintExecution::invokeInterface(FlintConstInterfaceMethod &interfaceMethod, uint8_t argc) {
    FlintJavaObject *obj = (FlintJavaObject *)stack[sp - argc + 1];
    if(obj == 0) {
        const char *msg[] = {"Cannot invoke ", interfaceMethod.className.text, ".", interfaceMethod.nameAndType.name.text, " by null object"};
        throw &flint.newNullPointerException(&flint.newString(msg, LENGTH(msg)));
    }
    FlintConstUtf8 &type = (obj->dimensions > 0 || FlintJavaObject::isPrimType(obj->type)) ? (FlintConstUtf8 &)objectClassName : obj->type;
    FlintMethodInfo *methodInfo;
    if(interfaceMethod.methodInfo && interfaceMethod.methodInfo->classLoader.getThisClass() == type)
        methodInfo = interfaceMethod.methodInfo;
    else {
        interfaceMethod.methodInfo = &flint.findMethod(type, interfaceMethod.nameAndType);
        methodInfo = interfaceMethod.methodInfo;
    }
    if((methodInfo->accessFlag & METHOD_SYNCHRONIZED) && !lockObject(obj)) {
        FlintAPI::Thread::yield();
        return;
    }
    lr = pc + 5;
    invoke(*methodInfo, argc);
}

void FlintExecution::invokeStaticCtor(FlintClassData &classData) {
    Flint::lock();
    if(classData.getInitStatus() != UNINITIALIZED)
        return Flint::unlock();
    classData.staticInitOwnId = (uint32_t)this;
    flint.initStaticField(classData);
    Flint::unlock();
    FlintMethodInfo &ctorMethod = classData.getStaticCtor();
    lr = pc;
    invoke(ctorMethod, 0);
}

void FlintExecution::run(void) {
    static const void *opcodeLabels[256] = {
        &&op_nop, &&op_aconst_null, &&op_iconst_m1, &&op_iconst_0, &&op_iconst_1, &&op_iconst_2, &&op_iconst_3, &&op_iconst_4, &&op_iconst_5,
        &&op_lconst_0, &&op_lconst_1, &&op_fconst_0, &&op_fconst_1, &&op_fconst_2, &&op_dconst_0, &&op_dconst_1, &&op_bipush, &&op_sipush,
        &&op_ldc, &&op_ldc_w, &&op_ldc2_w, &&op_iload, &&op_lload, &&op_fload, &&op_dload, &&op_aload, &&op_iload_0, &&op_iload_1, &&op_iload_2,
        &&op_iload_3, &&op_lload_0, &&op_lload_1, &&op_lload_2, &&op_lload_3, &&op_fload_0, &&op_fload_1, &&op_fload_2, &&op_fload_3,
        &&op_dload_0, &&op_dload_1, &&op_dload_2, &&op_dload_3, &&op_aload_0, &&op_aload_1, &&op_aload_2, &&op_aload_3, &&op_iaload, &&op_laload,
        &&op_faload, &&op_daload, &&op_aaload, &&op_baload, &&op_caload, &&op_saload, &&op_istore, &&op_lstore, &&op_fstore, &&op_dstore,
        &&op_astore, &&op_istore_0, &&op_istore_1, &&op_istore_2, &&op_istore_3, &&op_lstore_0, &&op_lstore_1, &&op_lstore_2, &&op_lstore_3,
        &&op_fstore_0, &&op_fstore_1, &&op_fstore_2, &&op_fstore_3, &&op_dstore_0, &&op_dstore_1, &&op_dstore_2, &&op_dstore_3, &&op_astore_0,
        &&op_astore_1, &&op_astore_2, &&op_astore_3, &&op_iastore, &&op_lastore, &&op_fastore, &&op_dastore, &&op_aastore, &&op_bastore,
        &&op_castore, &&op_sastore, &&op_pop, &&op_pop2, &&op_dup, &&op_dup_x1, &&op_dup_x2, &&op_dup2, &&op_dup2_x1, &&op_dup2_x2, &&op_unknow,
        &&op_iadd, &&op_ladd, &&op_fadd, &&op_dadd, &&op_isub, &&op_lsub, &&op_fsub, &&op_dsub, &&op_imul, &&op_lmul, &&op_fmul, &&op_dmul,
        &&op_idiv, &&op_ldiv, &&op_fdiv, &&op_ddiv, &&op_irem, &&op_lrem, &&op_frem, &&op_drem, &&op_ineg, &&op_lneg, &&op_fneg, &&op_dneg,
        &&op_ishl, &&op_lshl, &&op_ishr, &&op_lshr, &&op_iushr, &&op_lushr, &&op_iand, &&op_land, &&op_ior, &&op_lor, &&op_ixor, &&op_lxor,
        &&op_iinc, &&op_i2l, &&op_i2f, &&op_i2d, &&op_l2i, &&op_l2f, &&op_l2d, &&op_f2i, &&op_f2l, &&op_f2d, &&op_d2i, &&op_d2l, &&op_d2f,
        &&op_i2b, &&op_i2c, &&op_i2s, &&op_lcmp, &&op_fcmpl, &&op_fcmpg, &&op_dcmpl, &&op_dcmpg, &&op_ifeq, &&op_ifne, &&op_iflt, &&op_ifge,
        &&op_ifgt, &&op_ifle, &&op_if_icmpeq, &&op_if_icmpne, &&op_if_icmplt, &&op_if_icmpge, &&op_if_icmpgt, &&op_if_icmple, &&op_if_acmpeq,
        &&op_if_acmpne, &&op_goto, &&op_jsr, &&op_ret, &&op_tableswitch, &&op_lookupswitch, &&op_ireturn, &&op_lreturn, &&op_freturn,
        &&op_dreturn, &&op_areturn, &&op_return, &&op_getstatic, &&op_putstatic, &&op_getfield, &&op_putfield, &&op_invokevirtual,
        &&op_invokespecial, &&op_invokestatic, &&op_invokeinterface, &&op_invokedynamic, &&op_new, &&op_newarray, &&op_anewarray,
        &&op_arraylength, &&op_athrow, &&op_checkcast, &&op_instanceof, &&op_monitorenter, &&op_monitorexit, &&op_wide, &&op_multianewarray,
        &&op_ifnull, &&op_ifnonnull, &&op_goto_w, &&op_jsrw, &&op_breakpoint, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_exit,
    };

    static const void *opcodeLabelsDebug[256] = {
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&op_unknow, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp, &&check_bkp,
        &&check_bkp, &&check_bkp, &&check_bkp, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_exit,
    };

    static const void *opcodeLabelsExit[256] = {
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
        &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit, &&op_exit,
    };

    ::opcodeLabelsExit = opcodeLabelsExit;
    FlintDebugger *dbg = flint.getDebugger();
    opcodes = dbg ? opcodeLabelsDebug : opcodeLabels;

    FlintLoadFileError *fileNotFound = 0;

    stackInitExitPoint(method->getAttributeCode().codeLength);

    if(method->classLoader.hasStaticCtor()) {
        invokeStaticCtor((FlintClassData &)method->classLoader);
        goto *opcodes[code[pc]];
    }

    goto *opcodes[code[pc]];
    check_bkp: {
        dbg->checkBreakPoint(this);
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
            case CONST_STRING:
                stackPushObject(&method->classLoader.getConstString(flint, constPool));
                goto *opcodes[code[pc]];
            case CONST_CLASS:
                stackPushObject(&method->classLoader.getConstClass(flint, constPool));
                goto *opcodes[code[pc]];
            case CONST_METHOD_TYPE:
                // TODO
                goto *opcodes[code[pc]];
            case CONST_METHOD_HANDLE:
                // TODO
                goto *opcodes[code[pc]];
            default:
                throw "unkown the const pool tag";
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
            case CONST_STRING:
                stackPushObject(&method->classLoader.getConstString(flint, constPool));
                goto *opcodes[code[pc]];
            case CONST_CLASS:
                stackPushObject(&method->classLoader.getConstClass(flint, constPool));
                goto *opcodes[code[pc]];
            case CONST_METHOD_TYPE:
                // TODO
                goto *opcodes[code[pc]];
            case CONST_METHOD_HANDLE:
                // TODO
                goto *opcodes[code[pc]];
            default:
                throw "unkown the const pool tag";
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
                throw "unkown the const pool tag";
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
            char indexStrBuff[11];
            char lengthStrBuff[11];
            sprintf(indexStrBuff, "%d", (int)index);
            sprintf(lengthStrBuff, "%d", obj->size / sizeof(int32_t));
            const char *msg[] = {"Index ", indexStrBuff, " out of bounds for length ", lengthStrBuff};
            FlintJavaString &strObj = flint.newString(msg, LENGTH(msg));
            try {
                FlintJavaThrowable &excpObj = flint.newArrayIndexOutOfBoundsException(&strObj);
                stackPushObject(&excpObj);
            }
            catch(FlintLoadFileError *file) {
                fileNotFound = file;
                goto file_not_found_excp;
            }
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
            char indexStrBuff[11];
            char lengthStrBuff[11];
            sprintf(indexStrBuff, "%d", (int)index);
            sprintf(lengthStrBuff, "%d", obj->size / sizeof(int64_t));
            const char *msg[] = {"Index ", indexStrBuff, " out of bounds for length ", lengthStrBuff};
            FlintJavaString &strObj = flint.newString(msg, LENGTH(msg));
            try {
                FlintJavaThrowable &excpObj = flint.newArrayIndexOutOfBoundsException(&strObj);
                stackPushObject(&excpObj);
            }
            catch(FlintLoadFileError *file) {
                fileNotFound = file;
                goto file_not_found_excp;
            }
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
            char indexStrBuff[11];
            char lengthStrBuff[11];
            sprintf(indexStrBuff, "%d", (int)index);
            sprintf(lengthStrBuff, "%d", obj->size / sizeof(int32_t));
            const char *msg[] = {"Index ", indexStrBuff, " out of bounds for length ", lengthStrBuff};
            FlintJavaString &strObj = flint.newString(msg, LENGTH(msg));
            try {
                FlintJavaThrowable &excpObj = flint.newArrayIndexOutOfBoundsException(&strObj);
                stackPushObject(&excpObj);
            }
            catch(FlintLoadFileError *file) {
                fileNotFound = file;
                goto file_not_found_excp;
            }
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
            char indexStrBuff[11];
            char lengthStrBuff[11];
            sprintf(indexStrBuff, "%d", (int)index);
            sprintf(lengthStrBuff, "%d", obj->size / sizeof(int8_t));
            const char *msg[] = {"Index ", indexStrBuff, " out of bounds for length ", lengthStrBuff};
            FlintJavaString &strObj = flint.newString(msg, LENGTH(msg));
            try {
                FlintJavaThrowable &excpObj = flint.newArrayIndexOutOfBoundsException(&strObj);
                stackPushObject(&excpObj);
            }
            catch(FlintLoadFileError *file) {
                fileNotFound = file;
                goto file_not_found_excp;
            }
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
            char indexStrBuff[11];
            char lengthStrBuff[11];
            sprintf(indexStrBuff, "%d", (int)index);
            sprintf(lengthStrBuff, "%d", obj->size / sizeof(int16_t));
            const char *msg[] = {"Index ", indexStrBuff, " out of bounds for length ", lengthStrBuff};
            FlintJavaString &strObj = flint.newString(msg, LENGTH(msg));
            try {
                FlintJavaThrowable &excpObj = flint.newArrayIndexOutOfBoundsException(&strObj);
                stackPushObject(&excpObj);
            }
            catch(FlintLoadFileError *file) {
                fileNotFound = file;
                goto file_not_found_excp;
            }
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
            char indexStrBuff[11];
            char lengthStrBuff[11];
            sprintf(indexStrBuff, "%d", (int)index);
            sprintf(lengthStrBuff, "%d", obj->size / sizeof(int32_t));
            const char *msg[] = {"Index ", indexStrBuff, " out of bounds for length ", lengthStrBuff};
            FlintJavaString &strObj = flint.newString(msg, LENGTH(msg));
            try {
                FlintJavaThrowable &excpObj = flint.newArrayIndexOutOfBoundsException(&strObj);
                stackPushObject(&excpObj);
            }
            catch(FlintLoadFileError *file) {
                fileNotFound = file;
                goto file_not_found_excp;
            }
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
            char indexStrBuff[11];
            char lengthStrBuff[11];
            sprintf(indexStrBuff, "%d", (int)index);
            sprintf(lengthStrBuff, "%d", obj->size / sizeof(int64_t));
            const char *msg[] = {"Index ", indexStrBuff, " out of bounds for length ", lengthStrBuff};
            FlintJavaString &strObj = flint.newString(msg, LENGTH(msg));
            try {
                FlintJavaThrowable &excpObj = flint.newArrayIndexOutOfBoundsException(&strObj);
                stackPushObject(&excpObj);
            }
            catch(FlintLoadFileError *file) {
                fileNotFound = file;
                goto file_not_found_excp;
            }
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
            char indexStrBuff[11];
            char lengthStrBuff[11];
            sprintf(indexStrBuff, "%d", (int)index);
            sprintf(lengthStrBuff, "%d", obj->size / sizeof(int8_t));
            const char *msg[] = {"Index ", indexStrBuff, " out of bounds for length ", lengthStrBuff};
            FlintJavaString &strObj = flint.newString(msg, LENGTH(msg));
            try {
                FlintJavaThrowable &excpObj = flint.newArrayIndexOutOfBoundsException(&strObj);
                stackPushObject(&excpObj);
            }
            catch(FlintLoadFileError *file) {
                fileNotFound = file;
                goto file_not_found_excp;
            }
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
            char indexStrBuff[11];
            char lengthStrBuff[11];
            sprintf(indexStrBuff, "%d", (int)index);
            sprintf(lengthStrBuff, "%d", obj->size / sizeof(int16_t));
            const char *msg[] = {"Index ", indexStrBuff, " out of bounds for length ", lengthStrBuff};
            FlintJavaString &strObj = flint.newString(msg, LENGTH(msg));
            try {
                FlintJavaThrowable &excpObj = flint.newArrayIndexOutOfBoundsException(&strObj);
                stackPushObject(&excpObj);
            }
            catch(FlintLoadFileError *file) {
                fileNotFound = file;
                goto file_not_found_excp;
            }
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
        FlintConstField &constField = method->classLoader.getConstField(ARRAY_TO_INT16(&code[pc + 1]));
        FlintClassData *classData;
        try {
            classData = (FlintClassData *)&flint.load(constField.className);
        }
        catch(FlintLoadFileError *file) {
            fileNotFound = file;
            goto file_not_found_excp;
        }
        FlintInitStatus initStatus = classData->getInitStatus();
        if(initStatus == INITIALIZED || (initStatus == INITIALIZING && classData->staticInitOwnId == (uint32_t)this)) {
            FlintFieldsData *fields = classData->staticFieldsData;
            pc += 3;
            switch(constField.nameAndType.descriptor.text[0]) {
                case 'J':
                case 'D': {
                    stackPushInt64(fields->getFieldData64(constField).value);
                    goto *opcodes[code[pc]];
                }
                case 'L':
                case '[': {
                    stackPushObject(fields->getFieldObject(constField).object);
                    goto *opcodes[code[pc]];
                }
                default: {
                    stackPushInt32(fields->getFieldData32(constField).value);
                    goto *opcodes[code[pc]];
                }
            }
        }
        else if(initStatus == UNINITIALIZED)
            invokeStaticCtor(*classData);
        goto *opcodes[code[pc]];
    }
    op_putstatic: {
        FlintConstField &constField = method->classLoader.getConstField(ARRAY_TO_INT16(&code[pc + 1]));
        FlintClassData *classData;
        try {
            classData = (FlintClassData *)&flint.load(constField.className);
        }
        catch(FlintLoadFileError *file) {
            fileNotFound = file;
            goto file_not_found_excp;
        }
        FlintInitStatus initStatus = classData->getInitStatus();
        if(initStatus == INITIALIZED || (initStatus == INITIALIZING && classData->staticInitOwnId == (uint32_t)this)) {
            FlintFieldsData *fields = classData->staticFieldsData;
            pc += 3;
            switch(constField.nameAndType.descriptor.text[0]) {
                case 'Z':
                case 'B': {
                    fields->getFieldData32(constField).value = (int8_t)stackPopInt32();
                    goto *opcodes[code[pc]];
                }
                case 'C':
                case 'S': {
                    fields->getFieldData32(constField).value = (int16_t)stackPopInt32();
                    goto *opcodes[code[pc]];
                }
                case 'J':
                case 'D': {
                    fields->getFieldData64(constField).value = stackPopInt64();
                    goto *opcodes[code[pc]];
                }
                case 'L':
                case '[': {
                    fields->getFieldObject(constField).object = stackPopObject();
                    goto *opcodes[code[pc]];
                }
                default: {
                    fields->getFieldData32(constField).value = stackPopInt32();
                    goto *opcodes[code[pc]];
                }
            }
        }
        else if(initStatus == UNINITIALIZED)
            invokeStaticCtor(*classData);
        goto *opcodes[code[pc]];
    }
    op_getfield: {
        FlintConstField &constField = method->classLoader.getConstField(ARRAY_TO_INT16(&code[pc + 1]));
        pc += 3;
        switch(constField.nameAndType.descriptor.text[0]) {
            case 'J':
            case 'D': {
                FlintJavaObject *obj = stackPopObject();
                if(obj == 0)
                    goto getfield_null_excp;
                stackPushInt64(obj->getFields().getFieldData64(constField).value);
                goto *opcodes[code[pc]];
            }
            case 'L':
            case '[': {
                FlintJavaObject *obj = stackPopObject();
                if(obj == 0)
                    goto getfield_null_excp;
                stackPushObject(obj->getFields().getFieldObject(constField).object);
                goto *opcodes[code[pc]];
            }
            default: {
                FlintJavaObject *obj = stackPopObject();
                if(obj == 0)
                    goto getfield_null_excp;
                stackPushInt32(obj->getFields().getFieldData32(constField).value);
                goto *opcodes[code[pc]];
            }
        }
        getfield_null_excp: {
            const char *msg[] = {"Cannot read field '", constField.nameAndType.name.text, "' from null object"};
            FlintJavaString &strObj = flint.newString(msg, LENGTH(msg));
            try {
                FlintJavaThrowable &excpObj = flint.newNullPointerException(&strObj);
                stackPushObject(&excpObj);
            }
            catch(FlintLoadFileError *file) {
                fileNotFound = file;
                goto file_not_found_excp;
            }
            goto exception_handler;
        }
    }
    op_putfield: {
        FlintConstField &constField = method->classLoader.getConstField(ARRAY_TO_INT16(&code[pc + 1]));
        pc += 3;
        switch(constField.nameAndType.descriptor.text[0]) {
            case 'Z':
            case 'B': {
                int32_t value = stackPopInt32();
                FlintJavaObject *obj = stackPopObject();
                if(obj == 0)
                    goto putfield_null_excp;
                obj->getFields().getFieldData32(constField).value = (int8_t)value;
                goto *opcodes[code[pc]];
            }
            case 'C':
            case 'S': {
                int32_t value = stackPopInt32();
                FlintJavaObject *obj = stackPopObject();
                if(obj == 0)
                    goto putfield_null_excp;
                obj->getFields().getFieldData32(constField).value = (int16_t)value;
                goto *opcodes[code[pc]];
            }
            case 'J':
            case 'D': {
                int64_t value = stackPopInt64();
                FlintJavaObject *obj = stackPopObject();
                if(obj == 0)
                    goto putfield_null_excp;
                obj->getFields().getFieldData64(constField).value = value;
                goto *opcodes[code[pc]];
            }
            case 'L':
            case '[': {
                FlintJavaObject *value = stackPopObject();
                FlintJavaObject *obj = stackPopObject();
                if(obj == 0)
                    goto putfield_null_excp;
                obj->getFields().getFieldObject(constField).object = value;
                goto *opcodes[code[pc]];
            }
            default: {
                int32_t value = stackPopInt32();
                FlintJavaObject *obj = stackPopObject();
                if(obj == 0)
                    goto putfield_null_excp;
                obj->getFields().getFieldData32(constField).value = value;
                goto *opcodes[code[pc]];
            }
        }
        putfield_null_excp: {
            const char *msg[] = {"Cannot assign field '", constField.nameAndType.name.text, "' for null object"};
            FlintJavaString &strObj = flint.newString(msg, LENGTH(msg));
            try {
                FlintJavaThrowable &excpObj = flint.newNullPointerException(&strObj);
                stackPushObject(&excpObj);
            }
            catch(FlintLoadFileError *file) {
                fileNotFound = file;
                goto file_not_found_excp;
            }
            goto exception_handler;
        }
    }
    op_invokevirtual: {
        FlintConstMethod &constMethod = method->classLoader.getConstMethod(ARRAY_TO_INT16(&code[pc + 1]));
        try {
            invokeVirtual(constMethod);
        }
        catch(FlintJavaThrowable *ex) {
            stackPushObject(ex);
            goto exception_handler;
        }
        catch(FlintLoadFileError *file) {
            fileNotFound = file;
            goto file_not_found_excp;
        }
        catch(FlintFindNativeError *err) {
            const char *msg[] = {err->getMessage(), " ", constMethod.nameAndType.name.text};
            FlintJavaString &strObj = flint.newString(msg, LENGTH(msg));
            FlintJavaThrowable &excpObj = flint.newUnsatisfiedLinkErrorException(&strObj);
            stackPushObject(&excpObj);
            goto exception_handler;
        }
        catch(const char *msg) {
            FlintJavaThrowable &excpObj = flint.newException(&flint.newString(msg, strlen(msg)));
            stackPushObject(&excpObj);
            goto exception_handler;
        }
        goto *opcodes[code[pc]];
    }
    op_invokespecial: {
        FlintConstMethod &constMethod = method->classLoader.getConstMethod(ARRAY_TO_INT16(&code[pc + 1]));
        try {
            invokeSpecial(constMethod);
        }
        catch(FlintJavaThrowable *ex) {
            stackPushObject(ex);
            goto exception_handler;
        }
        catch(FlintLoadFileError *file) {
            fileNotFound = file;
            goto file_not_found_excp;
        }
        catch(FlintFindNativeError *err) {
            const char *msg[] = {err->getMessage(), " ", constMethod.nameAndType.name.text};
            FlintJavaString &strObj = flint.newString(msg, LENGTH(msg));
            FlintJavaThrowable &excpObj = flint.newUnsatisfiedLinkErrorException(&strObj);
            stackPushObject(&excpObj);
            goto exception_handler;
        }
        catch(const char *msg) {
            FlintJavaThrowable &excpObj = flint.newException(&flint.newString(msg, strlen(msg)));
            stackPushObject(&excpObj);
            goto exception_handler;
        }
        goto *opcodes[code[pc]];
    }
    op_invokestatic: {
        FlintConstMethod &constMethod = method->classLoader.getConstMethod(ARRAY_TO_INT16(&code[pc + 1]));
        try {
            invokeStatic(constMethod);
        }
        catch(FlintJavaThrowable *ex) {
            stackPushObject(ex);
            goto exception_handler;
        }
        catch(FlintLoadFileError *file) {
            fileNotFound = file;
            goto file_not_found_excp;
        }
        catch(FlintFindNativeError *err) {
            const char *msg[] = {err->getMessage(), " ", constMethod.nameAndType.name.text};
            FlintJavaThrowable &excpObj = flint.newUnsatisfiedLinkErrorException(&flint.newString(msg, LENGTH(msg)));
            stackPushObject(&excpObj);
            goto exception_handler;
        }
        catch(const char *msg) {
            FlintJavaThrowable &excpObj = flint.newException(&flint.newString(msg, strlen(msg)));
            stackPushObject(&excpObj);
            goto exception_handler;
        }
        goto *opcodes[code[pc]];
    }
    op_invokeinterface: {
        FlintConstInterfaceMethod &interfaceMethod = method->classLoader.getConstInterfaceMethod(ARRAY_TO_INT16(&code[pc + 1]));
        uint8_t count = code[pc + 3];
        try {
            invokeInterface(interfaceMethod, count);
        }
        catch(FlintJavaThrowable *ex) {
            stackPushObject(ex);
            goto exception_handler;
        }
        catch(FlintLoadFileError *file) {
            fileNotFound = file;
            goto file_not_found_excp;
        }
        catch(FlintFindNativeError *err) {
            const char *msg[] = {err->getMessage(), " ", interfaceMethod.nameAndType.name.text};
            FlintJavaString &strObj = flint.newString(msg, LENGTH(msg));
            FlintJavaThrowable &excpObj = flint.newUnsatisfiedLinkErrorException(&strObj);
            stackPushObject(&excpObj);
            goto exception_handler;
        }
        catch(const char *msg) {
            FlintJavaThrowable &excpObj = flint.newException(&flint.newString(msg, strlen(msg)));
            stackPushObject(&excpObj);
            goto exception_handler;
        }
        goto *opcodes[code[pc]];
    }
    op_invokedynamic: {
        // TODO
        // goto *opcodes[code[pc]];
        FlintJavaString &strObj = flint.newString(STR_AND_SIZE("Invokedynamic instructions are not supported"));
        try {
            FlintJavaThrowable &excpObj = flint.newUnsupportedOperationException(&strObj);
            stackPushObject(&excpObj);
        }
        catch(FlintLoadFileError *file) {
            fileNotFound = file;
            goto file_not_found_excp;
        }
        goto exception_handler;
    }
    op_new: {
        uint16_t poolIndex = ARRAY_TO_INT16(&code[pc + 1]);
        FlintConstUtf8 &constClass = method->classLoader.getConstUtf8Class(poolIndex);
        FlintJavaObject &obj = flint.newObject(sizeof(FlintFieldsData), constClass);
        memset(obj.data, 0, sizeof(FlintFieldsData));
        try {
            FlintClassData &classData = (FlintClassData &)flint.load(constClass);
            new ((FlintFieldsData *)obj.data)FlintFieldsData(flint, classData, false);
            stackPushObject(&obj);
            pc += 3;
            goto *opcodes[code[pc]];
        }
        catch(FlintLoadFileError *file) {
            fileNotFound = file;
            goto file_not_found_excp;
        }
    }
    op_newarray: {
        int32_t count = stackPopInt32();
        if(count < 0)
            goto negative_array_size_excp;
        uint8_t atype = code[pc + 1];
        uint8_t typeSize = FlintJavaObject::getPrimitiveTypeSize(atype);
        FlintJavaObject &obj = flint.newObject(typeSize * count, *primTypeConstUtf8List[atype - 4], 1);
        memset(obj.data, 0, obj.size);
        stackPushObject(&obj);
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_anewarray: {
        int32_t count = stackPopInt32();
        if(count < 0)
            goto negative_array_size_excp;
        uint16_t poolIndex = ARRAY_TO_INT16(&code[pc + 1]);
        FlintConstUtf8 &constClass = method->classLoader.getConstUtf8Class(poolIndex);
        FlintJavaObject &obj = flint.newObjectArray(constClass, count);
        memset(obj.data, 0, obj.size);
        stackPushObject(&obj);
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_arraylength: {
        FlintJavaObject *obj = stackPopObject();
        if(obj == 0) {
            FlintJavaString &strObj = flint.newString(STR_AND_SIZE("Cannot read the array length from null object"));
            try {
                FlintJavaThrowable &excpObj = flint.newNullPointerException(&strObj);
                stackPushObject(&excpObj);
            }
            catch(FlintLoadFileError *file) {
                fileNotFound = file;
                goto file_not_found_excp;
            }
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
            FlintJavaString &strObj = flint.newString(STR_AND_SIZE("Cannot throw exception by null object"));
            try {
                FlintJavaThrowable &excpObj = flint.newNullPointerException(&strObj);
                stackPushObject(&excpObj);
            }
            catch(FlintLoadFileError *file) {
                fileNotFound = file;
                goto file_not_found_excp;
            }
            goto exception_handler;
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
            FlintCodeAttribute &attributeCode = traceMethod->getAttributeCode();
            for(uint16_t i = 0; i < attributeCode.exceptionTableLength; i++) {
                FlintExceptionTable &exceptionTable = attributeCode.getException(i);
                if(exceptionTable.startPc <= tracePc && tracePc < exceptionTable.endPc) {
                    if(exceptionTable.catchType == 0 || flint.isInstanceof(obj, traceMethod->classLoader.getConstUtf8Class(exceptionTable.catchType))) {
                        while(startSp > traceStartSp)
                            stackRestoreContext();
                        sp = startSp + attributeCode.maxLocals;
                        stackPushObject(obj);
                        pc = exceptionTable.handlerPc;
                        goto *opcodes[code[pc]];
                    }
                }
            }
            if(traceStartSp < 0) {
                if(dbg && !dbg->exceptionIsEnabled())
                    dbg->caughtException(this, (FlintJavaThrowable *)obj);
                throw (FlintJavaThrowable *)obj;
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
            bool isInsOf;
            try {
                isInsOf = flint.isInstanceof(obj, type);
            }
            catch(FlintLoadFileError *file) {
                fileNotFound = file;
                goto file_not_found_excp;
            }
            if(!isInsOf) {
                uint32_t len = 7 + obj->type.length + 27 + type.length + 1;
                bool isPrimType = FlintJavaObject::isPrimType(obj->type);
                len += obj->dimensions;
                if(!isPrimType)
                    len += 2;
                FlintJavaString &strObj = flint.newString(len, 0);
                int8_t *strBuff = strObj.getValue()->getData();
                memcpy(&strBuff[0], "Class '", 7);
                uint32_t index = 7;
                for(uint32_t i = 0; i < obj->dimensions; i++)
                    strBuff[index++] = '[';
                if(!isPrimType)
                    strBuff[index++] = 'L';
                memcpy(&strBuff[index], obj->type.text, obj->type.length);
                index += obj->type.length;
                if(!isPrimType)
                    strBuff[index++] = ';';
                memcpy(&strBuff[index], "' cannot be cast to class '", 27);
                index += 27;
                memcpy(&strBuff[index], type.text, type.length);
                index += type.length;
                strBuff[index] = '\'';
                try {
                    FlintJavaThrowable &excpObj = flint.newClassCastException(&strObj);
                    stackPushObject(&excpObj);
                }
                catch(FlintLoadFileError *file) {
                    fileNotFound = file;
                    goto file_not_found_excp;
                }
                goto exception_handler;
            }
        }
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_instanceof: {
        FlintJavaObject *obj = stackPopObject();
        FlintConstUtf8 &type = method->classLoader.getConstUtf8Class(ARRAY_TO_INT16(&code[pc + 1]));
        try {
            stackPushInt32(flint.isInstanceof(obj, type));
        }
        catch(FlintLoadFileError *file) {
            fileNotFound = file;
            goto file_not_found_excp;
        }
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_monitorenter: {
        FlintJavaObject *obj = stackPopObject();
        if(obj == 0) {
            FlintJavaString &strObj = flint.newString(STR_AND_SIZE("Cannot enter synchronized block by null object"));
            try {
                FlintJavaThrowable &excpObj = flint.newNullPointerException(&strObj);
                stackPushObject(&excpObj);
            }
            catch(FlintLoadFileError *file) {
                fileNotFound = file;
                goto file_not_found_excp;
            }
            goto exception_handler;
        }
        if(!lockObject(obj)) {
            FlintAPI::Thread::yield();
            goto *opcodes[code[pc]];
        }
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
        try {
            if(typeNameText[dimensions] != 'L') {
                uint8_t atype = FlintJavaObject::convertToAType(typeNameText[dimensions]);
                if(atype == 0)
                    throw "invalid primative type";
                typeName = (FlintConstUtf8 *)primTypeConstUtf8List[atype - 4];
            }
            else
                typeName = &flint.load(&typeNameText[dimensions + 1], length - 2).getThisClass();
            sp -= dimensions - 1;
            for(int32_t i = 0; i < dimensions; i++) {
                if(stack[sp + i] < 0)
                    goto negative_array_size_excp;
            }
            FlintJavaObject &array = flint.newMultiArray(*typeName, &stack[sp], dimensions);
            stackPushObject(&array);
        }
        catch(FlintLoadFileError *file) {
            fileNotFound = file;
            goto file_not_found_excp;
        }
        pc += 4;
        goto *opcodes[code[pc]];
    }
    op_breakpoint:
        pc++;
        goto *opcodes[code[pc]];
    op_unknow:
        throw "unknow opcode";
    divided_by_zero_excp: {
        FlintJavaString &strObj = flint.newString(STR_AND_SIZE("Divided by zero"));
        try {
            FlintJavaThrowable &excpObj = flint.newArithmeticException(&strObj);
            stackPushObject(&excpObj);
        }
        catch(FlintLoadFileError *file) {
            fileNotFound = file;
            goto file_not_found_excp;
        }
        goto exception_handler;
    }
    negative_array_size_excp: {
        FlintJavaString &strObj = flint.newString(STR_AND_SIZE("Size of the array is a negative number"));
        try {
            FlintJavaThrowable &excpObj = flint.newNegativeArraySizeException(&strObj);
            stackPushObject(&excpObj);
        }
        catch(FlintLoadFileError *file) {
            fileNotFound = file;
            goto file_not_found_excp;
        }
        goto exception_handler;
    }
    load_null_array_excp: {
        FlintJavaString &strObj = flint.newString(STR_AND_SIZE("Cannot load from null array object"));
        try {
            FlintJavaThrowable &excpObj = flint.newNullPointerException(&strObj);
            stackPushObject(&excpObj);
        }
        catch(FlintLoadFileError *file) {
            fileNotFound = file;
            goto file_not_found_excp;
        }
        goto exception_handler;
    }
    store_null_array_excp: {
        FlintJavaString &strObj = flint.newString(STR_AND_SIZE("Cannot store to null array object"));
        try {
            FlintJavaThrowable &excpObj = flint.newNullPointerException(&strObj);
            stackPushObject(&excpObj);
        }
        catch(FlintLoadFileError *file) {
            fileNotFound = file;
            goto file_not_found_excp;
        }
        goto exception_handler;
    }
    file_not_found_excp: {
        const char *msg[] = {"Could not find or load class ", fileNotFound->getFileName(), ".class"};
        FlintJavaString &strObj = flint.newString(msg, LENGTH(msg));
        FlintJavaThrowable &excpObj = flint.newClassNotFoundException(&strObj);
        stackPushObject(&excpObj);
        goto exception_handler;
    }
    op_exit:
        return;
}

void FlintExecution::innerRunTask(FlintExecution *execution) {
    try {
        execution->run();
    }
    catch(FlintJavaThrowable *ex) {
        FlintJavaString *str = ex->getDetailMessage();
        if(str)
            execution->flint.print(str->getText(), str->getLength(), str->getCoder());
        else
            execution->flint.print(ex->type.text, ex->type.length, 0);
        execution->flint.print("\n", 1, 0);
    }
    catch(FlintOutOfMemoryError *err) {
        const char *msg = err->getMessage();
        execution->flint.print(msg, strlen(msg), 0);
        execution->flint.print("\n", 1, 0);
    }
    catch(FlintLoadFileError *file) {
        const char *fileName = file->getFileName();
        execution->flint.print("Could not find or load class ", 29, 0);
        while(*fileName) {
            execution->flint.print((*fileName == '/') ? "." : fileName, 1, 0);
            fileName++;
        }
        execution->flint.print("\n", 1, 0);
    }
    catch(const char *msg) {
        execution->flint.print(msg, strlen(msg), 0);
        execution->flint.print("\n", 1, 0);
    }
    while(execution->startSp > 3)
        execution->stackRestoreContext();
    execution->peakSp = -1;
    execution->flint.freeExecution(*execution);
    FlintAPI::Thread::terminate(0);
}

void FlintExecution::runTask(FlintExecution *execution) {
    innerRunTask(execution);
}

bool FlintExecution::run(FlintMethodInfo &method) {
    this->method = &method;
    if(!opcodes)
        return (FlintAPI::Thread::create((void (*)(void *))runTask, (void *)this) != 0);
    return false;
}

void FlintExecution::terminateRequest(void) {
    opcodes = opcodeLabelsExit;
}

bool FlintExecution::hasTerminateRequest(void) const {
    return (opcodes == opcodeLabelsExit);
}

FlintJavaThread &FlintExecution::getOnwerThread(void) {
    if(onwerThread == NULL) {
        Flint::lock();
        onwerThread = (FlintJavaThread *)&flint.newObject(threadClassName);
        Flint::unlock();
    }
    return *onwerThread;
}

FlintExecution::~FlintExecution(void) {
    Flint::free(stack);
}
