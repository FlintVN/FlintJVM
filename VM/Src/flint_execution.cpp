
#include <iostream>
#include <string.h>
#include "flint.h"
#include "flint_opcodes.h"
#include "flint_execution.h"
#include "flint_system_api.h"
#include "flint_default_conf.h"

#define FLOAT_NAN                           0x7FC00000
#define DOUBLE_NAN                          0x7FF8000000000000

#define ARRAY_TO_INT16(array)               (int16_t)(((array)[0] << 8) | (array)[1])
#define ARRAY_TO_INT32(array)               (int32_t)(((array)[0] << 24) | ((array)[1] << 16) | ((array)[2] << 8) | (array)[3])

#define GET_STACK_VALUE(_index)             stack[_index]
#define SET_STACK_VALUE(_index, _value)     stack[_index] = _value

static const void **opcodeLabelsStop = NULL;
static const void **opcodeLabelsExit = NULL;

FExec::FExec(JThread *onwer, uint32_t stackSize) : ListNode(), stackLength(stackSize / sizeof(int32_t)) {
    this->opcodes = 0;
    this->lr = -1;
    this->sp = -1;
    this->startSp = sp;
    this->peakSp = sp;
    this->onwerThread = onwerThread;
    this->excp = NULL;
}

void FExec::stackPushInt32(int32_t value) {
    stack[++sp] = value;
    peakSp = sp;
}

void FExec::stackPushInt64(int64_t value) {
    stack[++sp] = ((uint32_t *)&value)[0];
    stack[++sp] = ((uint32_t *)&value)[1];
    peakSp = sp;
}

void FExec::stackPushFloat(float value) {
    stack[++sp] = *(uint32_t *)&value;
    peakSp = sp;
}

void FExec::stackPushDouble(double value) {
    stack[++sp] = ((uint32_t *)&value)[0];
    stack[++sp] = ((uint32_t *)&value)[1];
    peakSp = sp;
}

void FExec::stackPushObject(JObject *obj) {
    stack[++sp] = (int32_t)obj;
    peakSp = sp;
    if(obj && (obj->getProtected() & 0x02))
        Flint::clearProtLv2(obj);
}

int32_t FExec::stackPopInt32(void) {
    return stack[sp--];
}

int64_t FExec::stackPopInt64(void) {
    uint64_t ret;
    ((uint32_t *)&ret)[1] = stack[sp--];
    ((uint32_t *)&ret)[0] = stack[sp--];
    return ret;
}

float FExec::stackPopFloat(void) {
    return *(float *)&stack[sp--];
}

double FExec::stackPopDouble(void) {
    uint64_t ret;
    sp -= 2;
    ((uint32_t *)&ret)[1] = stack[sp--];
    ((uint32_t *)&ret)[0] = stack[sp--];
    return *(double *)&ret;
}

JObject *FExec::stackPopObject(void) {
    return (JObject *)stack[sp--];
}

int32_t FExec::getStackTrace(StackFrame *stackTrace, int32_t traceSp) const {
    if(traceSp < 4) return -1;
    uint32_t tracePc = stack[traceSp - 2];
    MethodInfo *traceMethod = (MethodInfo *)stack[traceSp - 3];
    new (stackTrace)StackFrame(tracePc, stack[traceSp], traceMethod);
    return stack[traceSp];
}

bool FExec::getStackTrace(uint32_t index, StackFrame *stackTrace, bool *isEndStack) const {
    if(index == 0) {
        new (stackTrace)StackFrame(pc, startSp, method);
        if(isEndStack) *isEndStack = (startSp < 4);
        return true;
    }
    else {
        int32_t traceSp = startSp;
        do {
            traceSp = getStackTrace(stackTrace, traceSp);
            if(traceSp < 0) return false;
            /* Check if pc == -1 or not to skip exit point */
            if(stackTrace->pc != 0xFFFFFFFF) index--;
        } while(stackTrace->pc == 0xFFFFFFFF || index);
        if(isEndStack) *isEndStack = (traceSp < 4);
        return true;
    }
}

bool FExec::readLocal(uint32_t stackIndex, uint32_t localIndex, uint32_t *value, bool *isObject) const {
    StackFrame stackTrace;
    if(!getStackTrace(stackIndex, &stackTrace, 0)) return false;
    *value = stack[stackTrace.baseSp + 1 + localIndex];
    if(*isObject) *isObject = Flint::isObject((void *)*value);
    return true;
}

bool FExec::readLocal(uint32_t stackIndex, uint32_t localIndex, uint64_t *value) const {
    StackFrame stackTrace;
    if(!getStackTrace(stackIndex, &stackTrace, 0)) return false;
    *value = *(int64_t *)&stack[stackTrace.baseSp + 1 + localIndex];
    return true;
}

void FExec::stackPushArgs(uint32_t argc, va_list args) {
    for(uint32_t i = 0; i < argc; i++) {
        int32_t val = va_arg(args, int32_t);
        if(Flint::isObject((void *)val)) stackPushObject((JObject *)val);
        else stackPushInt32(val);
    }
}

void FExec::stackSaveContext(void) {
    stack[++sp] = (int32_t)method;
    stack[++sp] = pc;
    stack[++sp] = lr;
    stack[++sp] = startSp;
    startSp = sp;
}

void FExec::stackRestoreContext(void) {
    sp = startSp;
    startSp = stackPopInt32();
    lr = stackPopInt32();
    pc = stackPopInt32();
    method = (MethodInfo *)stackPopInt32();
    code = method->getCode();
    locals = &stack[startSp + 1];
}

void FExec::restoreContext(void) {
    if(method->accessFlag & (METHOD_SYNCHRONIZED | METHOD_CLINIT) && pc != 0xFFFFFFFF) {
        if(method->accessFlag & METHOD_STATIC) {
            if(method->accessFlag & METHOD_CLINIT)
                method->loader->staticInitialized();
            unlockClass(method->loader);
        }
        else
            unlockObject((JObject *)locals[0]);
    }
    stackRestoreContext();
}

void FExec::initNewContext(MethodInfo *methodInfo, uint16_t argc) {
    uint16_t maxLocals = methodInfo->getMaxLocals();
    method = methodInfo;
    code = methodInfo->getCode();
    if(code == NULL)
        return throwNew(Flint::findClass(this, "java/lang/LinkageError"), methodInfo->loader->getName(), methodInfo->name);
    pc = 0;
    locals = &stack[sp + 1];
    for(uint32_t i = argc; i < maxLocals; i++) {
        uint32_t index = sp + i + 1;
        stack[index] = 0;
    }
    sp += maxLocals;
}

void FExec::initExitPoint(MethodInfo *methodInfo) {
    this->method = methodInfo;
    this->pc = -1;
    this->lr = methodInfo->getCodeLength();  /* OP_EXIT - Initialize exit point */
}

bool FExec::lockClass(ClassLoader *cls) {
    Flint::lock();
    if(cls->monitorCount == 0 || cls->monitorOwnId == (int32_t)this) {
        cls->monitorOwnId = (int32_t)this;
        if(cls->monitorCount < 0xFFFFFFFF) {
            cls->monitorCount++;
            Flint::unlock();
            return true;
        }
        Flint::unlock();
        throwNew(Flint::findClass(this, "java/lang/IllegalMonitorStateException"));
        return false;
    }
    Flint::unlock();
    return false;
}

void FExec::unlockClass(ClassLoader *cls) {
    Flint::lock();
    if(cls->monitorCount)
        cls->monitorCount--;
    Flint::unlock();
}

bool FExec::lockObject(JObject *obj) {
    Flint::lock();
    if(obj->monitorCount == 0 || obj->ownId == (int32_t)this) {
        obj->ownId = (int32_t)this;
        if(obj->monitorCount < 0xFFFFFFFF) {
            obj->monitorCount++;
            Flint::unlock();
            return true;
        }
        Flint::unlock();
        throwNew(Flint::findClass(this, "java/lang/IllegalMonitorStateException"));
        return false;
    }
    Flint::unlock();
    return false;
}

void FExec::unlockObject(JObject *obj) {
    Flint::lock();
    if(obj->monitorCount)
        obj->monitorCount--;
    Flint::unlock();
}

static bool checkInvokeArgs(FExec *ctx, JObject *obj, MethodInfo *methodInfo) {
    if(obj == NULL) {
        JClass *excpCls = Flint::findClass(ctx, "java/lang/NullPointerException");
        ctx->throwNew(excpCls, "Can not invoke \"%s.%s\" by null object", methodInfo->loader->getName(), methodInfo->name);
        return false;
    }
    JClass *cls = methodInfo->loader->getThisClass(ctx);
    if(cls == NULL) return 0;
    if(!Flint::isInstanceof(ctx, obj, cls)) {
        JClass *excpCls = Flint::findClass(ctx, "java/lang/IncompatibleClassChangeError");
        ctx->throwNew(excpCls, "object type %s cannot be used as the \"this\" parameter for the \"%s.%s\" method", obj->getTypeName(), methodInfo->loader->getName(), methodInfo->name);
        return false;
    }
    return true;
}

uint64_t FExec::callMethod(MethodInfo *methodInfo, uint8_t argc) {
    MethodAccessFlag flag = methodInfo->accessFlag;
    if(!(flag & METHOD_NATIVE)) {
        peakSp = sp + 4;
        sp -= argc;
        if(peakSp >= stackLength) {
            throwNew(Flint::findClass(this, "java/lang/StackOverflowError"));
            return 0;
        }
        memmove(&stack[sp + 1 + 4], &stack[sp + 1], argc * sizeof(uint32_t));
        stackSaveContext();
        sp += argc;
        initExitPoint(methodInfo);
    }
    if(!(flag & METHOD_STATIC) && !checkInvokeArgs(this, (JObject *)stack[sp - argc + 1], methodInfo)) return 0;

    /* Lock Class/Object if method is SYNCHRONIZED */
    if(flag & (METHOD_SYNCHRONIZED | METHOD_CLINIT)) {
        if(flag & METHOD_NATIVE) while(lockClass(methodInfo->loader) == false)
            FlintAPI::Thread::yield();
        else while(lockObject((JObject *)stack[sp - argc - 1]) == false)
            FlintAPI::Thread::yield();
    }

    invoke(methodInfo, argc);
    if(hasException() || hasTerminateRequest()) return 0;
    if(!(flag & METHOD_NATIVE)) {
        exec(false);
        if(hasException() || hasTerminateRequest()) return 0;
    }
    uint64_t ret = 0;
    switch(methodInfo->getReturnType()[0]) {
        case 'V': break;
        case 'J':
        case 'D': ret = stackPopInt64(); break;
        case 'L': ret = (uint64_t)stackPopObject(); break;
        default: ret = stackPopInt32(); break;
    }
    if(!(flag & METHOD_NATIVE)) stackRestoreContext();
    return ret;
}

template <typename T>
static T callToNative(FNIEnv *env, T (*nmtptr)(FNIEnv *, ...), int32_t *args, uint8_t argc) {
    static constexpr const void *callWithArgs[] = {
        &&args0, &&args1,  &&args2,  &&args3,  &&args4,  &&args5,  &&args6,  &&args7, &&args8,
        &&args9, &&args10, &&args11, &&args12, &&args13, &&args14, &&args15, &&args16, &&args17
    };
    goto *callWithArgs[argc];
    args0:  return nmtptr(env);
    args1:  return nmtptr(env, args[0]);
    args2:  return nmtptr(env, args[0], args[1]);
    args3:  return nmtptr(env, args[0], args[1], args[2]);
    args4:  return nmtptr(env, args[0], args[1], args[2], args[3]);
    args5:  return nmtptr(env, args[0], args[1], args[2], args[3], args[4]);
    args6:  return nmtptr(env, args[0], args[1], args[2], args[3], args[4], args[5]);
    args7:  return nmtptr(env, args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
    args8:  return nmtptr(env, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
    args9:  return nmtptr(env, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]);
    args10: return nmtptr(env, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);
    args11: return nmtptr(env, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10]);
    args12: return nmtptr(env, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11]);
    args13: return nmtptr(env, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11], args[12]);
    args14: return nmtptr(env, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11], args[12], args[13]);
    args15: return nmtptr(env, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11], args[12], args[13], args[14]);
    args16: return nmtptr(env, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11], args[12], args[13], args[14], args[15]);
    args17: return nmtptr(env, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11], args[12], args[13], args[14], args[15], args[16]);
}

void FExec::invokeNativeMethod(MethodInfo *methodInfo, uint8_t argc) {
    JNMPtr nmtptr = (JNMPtr)methodInfo->getCode();
    if(!nmtptr) {
        throwNew(Flint::findClass(this, "java/lang/LinkageError"), "%s.%s", methodInfo->loader->getName(), methodInfo->name);
        return;
    }
    FNIEnv env(this);
    switch(methodInfo->getReturnType()[0]) {
        case 'V': {
            callToNative(&env, (void (*)(FNIEnv *, ...))nmtptr, &stack[sp - argc + 1], argc);
            sp = sp - argc;
            if(excp != NULL) return;
            pc = lr;
            return;
        }
        case 'F': {
            float val = callToNative(&env, (float (*)(FNIEnv *, ...))nmtptr, &stack[sp - argc + 1], argc);
            sp = sp - argc;
            if(excp != NULL) return;
            stackPushFloat(val);
            pc = lr;
            return;
        }
        case 'J': {
            int64_t val = callToNative(&env, (int64_t (*)(FNIEnv *, ...))nmtptr, &stack[sp - argc + 1], argc);
            sp = sp - argc;
            if(excp != NULL) return;
            stackPushInt64(val);
            pc = lr;
            return;
        }
        case 'D': {
            double val = callToNative(&env, (double (*)(FNIEnv *, ...))nmtptr, &stack[sp - argc + 1], argc);
            sp = sp - argc;
            if(excp != NULL) return;
            stackPushDouble(val);
            pc = lr;
            return;
        }
        case 'L':
        case '[': {
            JObject *val = callToNative(&env, (JObject *(*)(FNIEnv *, ...))nmtptr, &stack[sp - argc + 1], argc);
            sp = sp - argc;
            if(excp != NULL) return;
            stackPushObject(val);
            pc = lr;
            return;
        }
        default: {
            int32_t val = callToNative(&env, (int32_t (*)(FNIEnv *, ...))nmtptr, &stack[sp - argc + 1], argc);
            sp = sp - argc;
            if(excp != NULL) return;
            stackPushInt32(val);
            pc = lr;
            return;
        }
    }
}

void FExec::invoke(MethodInfo *methodInfo, uint8_t argc) {
    if(!(methodInfo->accessFlag & METHOD_NATIVE)) {
        peakSp = sp + 4;
        sp -= argc;
        if((sp + methodInfo->getMaxLocals() + methodInfo->getMaxStack() + 4) >= stackLength)
            return throwNew(Flint::findClass(this, "java/lang/StackOverflowError"));
        memmove(&stack[sp + 1 + 4], &stack[sp + 1], argc * sizeof(uint32_t));
        stackSaveContext();
        initNewContext(methodInfo, argc);
    }
    else
        invokeNativeMethod(methodInfo, argc);
}

void FExec::invokeStatic(ConstMethod *constMethod) {
    MethodInfo *methodInfo = constMethod->methodInfo;
    if(methodInfo == NULL) {
        methodInfo = Flint::findMethod(this, Flint::findClass(this, constMethod->className), constMethod->nameAndType);
        if(methodInfo == NULL) return;
        constMethod->methodInfo = methodInfo;
        if(methodInfo->loader->getStaticInitStatus() == UNINITIALIZED)
            return invokeStaticCtor(methodInfo->loader);
    }
    if(methodInfo->accessFlag & (METHOD_SYNCHRONIZED | METHOD_CLINIT)) {
        if(lockClass(methodInfo->loader) == false) {
            FlintAPI::Thread::yield();
            return;
        }
    }
    lr = pc + 3;
    return invoke(methodInfo, constMethod->getArgc());
}

void FExec::invokeSpecial(ConstMethod *constMethod) {
    uint8_t argc = constMethod->getArgc() + 1;
    MethodInfo *methodInfo = constMethod->methodInfo;
    if(methodInfo == NULL) {
        methodInfo = Flint::findMethod(this, Flint::findClass(this, constMethod->className), constMethod->nameAndType);
        if(methodInfo == NULL) return;
        constMethod->methodInfo = methodInfo;
        if(methodInfo->loader->getStaticInitStatus() == UNINITIALIZED)
            return invokeStaticCtor(methodInfo->loader);
    }
    if(methodInfo->accessFlag & (METHOD_SYNCHRONIZED | METHOD_CLINIT)) {
        if(lockObject((JObject *)stack[sp - argc - 1]) == false) {
            FlintAPI::Thread::yield();
            return;
        }
    }
    lr = pc + 3;
    return invoke(methodInfo, argc);
}

void FExec::invokeVirtual(ConstMethod *constMethod) {
    uint8_t argc = constMethod->getArgc();
    JObject *obj = (JObject *)stack[sp - argc];
    if(obj == NULL) {
        JClass *excpCls = Flint::findClass(this, "java/lang/NullPointerException");
        throwNew(excpCls, "Cannot invoke \"%s.%s\" by null object", constMethod->className, constMethod->nameAndType->name);
        return;
    }
    MethodInfo *methodInfo = constMethod->methodInfo;
    JClass *objType;
    if(obj->type != NULL) objType = obj->type;
    else {
        objType = Flint::getClassOfClass(this);
        if(objType == NULL) return;
    }
    if(methodInfo == NULL || methodInfo->loader != objType->getClassLoader()) {
        methodInfo = Flint::findMethod(this, objType, constMethod->nameAndType);
        if(methodInfo == NULL) return;
        constMethod->methodInfo = methodInfo;
        if(methodInfo->loader->getStaticInitStatus() == UNINITIALIZED)
            return invokeStaticCtor(methodInfo->loader);
    }
    if(methodInfo->accessFlag & (METHOD_SYNCHRONIZED | METHOD_CLINIT)) {
        if(lockObject(obj) == false) {
            FlintAPI::Thread::yield();
            return;
        }
    }
    argc++;
    lr = pc + 3;
    return invoke(methodInfo, argc);
}

void FExec::invokeInterface(ConstInterfaceMethod *interfaceMethod, uint8_t argc) {
    JObject *obj = (JObject *)stack[sp - argc + 1];
    if(obj == NULL) {
        JClass *excpCls = Flint::findClass(this, "java/lang/NullPointerException");
        throwNew(excpCls, "Cannot invoke \"%s.%s\" by null object", interfaceMethod->className, interfaceMethod->nameAndType->name);
        return;
    }
    MethodInfo *methodInfo = interfaceMethod->methodInfo;
    JClass *objType;
    if(obj->type != NULL) objType = obj->type;
    else {
        objType = Flint::getClassOfClass(this);
        if(objType == NULL) return;
    }
    if(methodInfo == NULL || methodInfo->loader != objType->getClassLoader()) {
        methodInfo = Flint::findMethod(this, objType, interfaceMethod->nameAndType);
        if(methodInfo == NULL) return;
        interfaceMethod->methodInfo = methodInfo;
        if(methodInfo->loader->getStaticInitStatus() == UNINITIALIZED)
            return invokeStaticCtor(methodInfo->loader);
    }
    if(methodInfo->accessFlag & (METHOD_SYNCHRONIZED | METHOD_CLINIT)) {
        if(lockObject(obj) == false) {
            FlintAPI::Thread::yield();
            return;
        }
    }
    lr = pc + 5;
    return invoke(methodInfo, argc);
}

void FExec::invokeDynamic(ConstInvokeDynamic *constInvokeDynamic) {
    if(!constInvokeDynamic->isLinked()) {
        ClassLoader *ld = method->loader;
        BootstrapMethod *bootstapMethod = ld->getBootstapMethod(constInvokeDynamic->getBootstrapMethodAttrIndex());
        ConstNameAndType *nameAndType = ld->getConstNameAndType(this, constInvokeDynamic->getNameAndTypeIndex());
        if(nameAndType == NULL) return;

        JString *name = Flint::newString(this, nameAndType->name);
        if(name == NULL) return;

        JObject *type = Flint::newMethodType(this, nameAndType->desc);
        if(type == NULL) { Flint::freeObject(name); return; }

        ConstMethodHandle *methodHandle = ld->getConstMethodHandle(bootstapMethod->bootstrapMethodRefIndex);
        ConstMethod *constMethod = ld->getConstMethod(this, methodHandle->refIndex);
        if(constMethod == NULL) { Flint::freeObject(name); Flint::freeObject(type); return; }

        MethodInfo *bootstapMethodInfo = constMethod->methodInfo;
        if(bootstapMethodInfo == NULL) {
            bootstapMethodInfo = Flint::findMethod(this, Flint::findClass(this, constMethod->className), constMethod->nameAndType);
            if(bootstapMethodInfo == NULL) { Flint::freeObject(name); Flint::freeObject(type); return; }
            constMethod->methodInfo = bootstapMethodInfo;
        }

        JClass *methodHandles = Flint::findClass(this, "java/lang/invoke/MethodHandles");
        if(methodHandles == NULL) { Flint::freeObject(name); Flint::freeObject(type); return; }

        MethodInfo *lookupMethod = methodHandles->getClassLoader()->getMethodInfo(this, "lookup", "()Ljava/lang/invoke/MethodHandles$Lookup;");
        if(lookupMethod == NULL) { Flint::freeObject(name); Flint::freeObject(type); return; }

        JObject *caller = (JObject *)callMethod(lookupMethod, 0);
        if(caller == NULL) { Flint::freeObject(name); Flint::freeObject(type); return; }

        stackPushObject(caller);
        stackPushObject(name);
        stackPushObject(type);
        for(uint8_t i = 0; i < bootstapMethod->numBootstrapMethodArgs; i++) {
            uint16_t poolIndex = bootstapMethod->args[i];
            ConstPoolTag tag = ld->getConstPoolTag(poolIndex);
            switch(tag) {
                case CONST_INTEGER: stackPushInt32(ld->getConstInteger(poolIndex)); break;
                case CONST_FLOAT: stackPushFloat(ld->getConstFloat(poolIndex)); break;
                case CONST_LONG: stackPushInt64(ld->getConstLong(poolIndex)); break;
                case CONST_DOUBLE: stackPushDouble(ld->getConstDouble(poolIndex)); break;
                case CONST_CLASS: {
                    JClass *cls = ld->getConstClass(this, poolIndex);
                    if(cls == NULL) return;
                    stackPushObject(cls);
                    break;
                }
                case CONST_STRING: {
                    JString *str = ld->getConstString(this, poolIndex);
                    if(str == NULL) return;
                    stackPushObject(str);
                    break;
                }
                case CONST_METHOD_HANDLE: {
                    ConstMethodHandle *constMethodHandle = ld->getConstMethodHandle(poolIndex);
                    ConstMethod *constMethod = ld->getConstMethod(this, constMethodHandle->refIndex);
                    if(constMethod == NULL) return;
                    JMethodHandle *methodHandle = Flint::newMethodHandle(this, constMethod, constMethodHandle->refKind);
                    if(methodHandle == NULL) return;
                    stackPushObject(methodHandle);
                    break;
                }
                case CONST_METHOD_TYPE: {
                    JObject *methodType = Flint::newMethodType(this, ld->getConstMethodType(poolIndex));
                    if(methodType == NULL) return;
                    stackPushObject(methodType);
                    break;
                }
                default: {
                    JClass *excpCls = Flint::findClass(this, "java/lang/ClassFormatError");
                    throwNew(excpCls, "Constant pool tag value (%u) is invalid in class %s", tag, ld->getName());
                    return;
                }
            }
        }
        for(uint8_t i = 3 + bootstapMethod->numBootstrapMethodArgs; i < constMethod->argc; i++)
            stackPushObject(NULL);

        uint64_t ret;
        switch(methodHandle->refKind) {
            case REF_GETFIELD:
                break;
            case REF_GETSTATIC:
                break;
            case REF_PUTFIELD:
                break;
            case REF_PUTSTATIC:
                break;
            case REF_INVOKEVIRTUAL:
                break;
            case REF_INVOKESTATIC: {
                ret = callMethod(bootstapMethodInfo, constMethod->argc);
                break;
            }
            case REF_INVOKESPECIAL:
                break;
            case REF_NEWINVOKESPECIAL:
                break;
            case REF_INVOKEINTERFACE:
                break;
        }
        if(hasException() || hasTerminateRequest()) return;
        /* Link CallSite */
        constInvokeDynamic->linkTo((JObject *)ret);
        Flint::makeToGlobal((JObject *)ret);
    }
    JObject *callSite = constInvokeDynamic->getCallSite();
    JMethodHandle *target = (JMethodHandle *)callSite->getFieldObjByIndex(0)->value;
    stackPushObject(target);    /* this */
    stackPushObject(NULL);      /* args */
    invokeVirtual(target->getConstMethod());
}

void FExec::invokeStaticCtor(ClassLoader *loader) {
    if(lockClass(loader) == false) { FlintAPI::Thread::yield(); return; }
    if(loader->getStaticInitStatus() != UNINITIALIZED) { unlockClass(loader); return; }
    if(loader->initStaticFields(this) == false) { unlockClass(loader); return; }
    MethodInfo *ctorMethod = loader->getStaticCtor(this);
    if(ctorMethod == NULL) {
        if(excp == NULL) throwNew(Flint::findClass(this, "java/lang/LinkageError"), "<clinit>()");
        unlockClass(loader);
        return;
    }
    if(code[pc] == OP_BREAKPOINT)
        ((uint8_t *)code)[pc] = OP_BREAKPOINT_DUMMY;
    lr = pc;
    return invoke(ctorMethod, 0);
}

void FExec::exec(bool initOpcodeLabels) {
    #include "flint_instruction_label.h"

    ::opcodeLabelsStop = (const void **)opcodeLabelsStop;
    ::opcodeLabelsExit = (const void **)opcodeLabelsExit;

    FDbg *dbg = Flint::getDebugger();
    if(initOpcodeLabels) opcodes = (const void ** volatile)opcodeLabels;

    const uint8_t *code = this->code;

    if(method->loader->getStaticInitStatus() == UNINITIALIZED) {
        invokeStaticCtor(method->loader);
        if(excp != NULL) goto exception_handler;
        code = this->code;
        goto *opcodes[code[pc]];
    }

    goto *opcodes[code[pc]];
    dbg_stop: {
        if(dbg && !dbg->checkStop(this)) {
            if(hasTerminateRequest()) {
                // TODO - ERROR
                return;
            }
            opcodes = (const void ** volatile)opcodeLabels;
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
        uint8_t poolIndex = code[pc + 1];
        ClassLoader *loader = method->loader;
        switch(loader->getConstPoolTag(poolIndex)) {
            case CONST_INTEGER:
                stackPushInt32(loader->getConstInteger(poolIndex));
                pc += 2;
                goto *opcodes[code[pc]];
            case CONST_FLOAT:
                stackPushFloat(loader->getConstFloat(poolIndex));
                pc += 2;
                goto *opcodes[code[pc]];
            case CONST_STRING: {
                JString *str = loader->getConstString(this, poolIndex);
                if(str == NULL) goto exception_handler;
                stackPushObject(str);
                pc += 2;
                goto *opcodes[code[pc]];
            }
            case CONST_CLASS: {
                JClass *cls = loader->getConstClass(this, poolIndex);
                if(cls == NULL) goto exception_handler;
                stackPushObject(cls);
                pc += 2;
                goto *opcodes[code[pc]];
            }
            case CONST_METHOD_TYPE:
                // TODO
                pc += 2;
                goto *opcodes[code[pc]];
            case CONST_METHOD_HANDLE:
                // TODO
                pc += 2;
                goto *opcodes[code[pc]];
            default: {
                JClass *excpCls = Flint::findClass(this, "java/lang/ClassFormatError");
                throwNew(excpCls, "Constant pool tag value (%u) is invalid in class %s", loader->getConstPoolTag(poolIndex), loader->getName());
                return;
            }
        }
    }
    op_ldc_w: {
        uint16_t poolIndex = ARRAY_TO_INT16(&code[pc + 1]);
        ClassLoader *loader = method->loader;
        switch(loader->getConstPoolTag(poolIndex)) {
            case CONST_INTEGER:
                stackPushInt32(loader->getConstInteger(poolIndex));
                pc += 3;
                goto *opcodes[code[pc]];
            case CONST_FLOAT:
                stackPushFloat(loader->getConstFloat(poolIndex));
                pc += 3;
                goto *opcodes[code[pc]];
            case CONST_STRING: {
                JString *str = loader->getConstString(this, poolIndex);
                if(str == NULL) goto exception_handler;
                stackPushObject(str);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case CONST_CLASS: {
                JClass *cls = loader->getConstClass(this, poolIndex);
                if(cls == NULL) goto exception_handler;
                stackPushObject(cls);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case CONST_METHOD_TYPE:
                // TODO
                pc += 3;
                goto *opcodes[code[pc]];
            case CONST_METHOD_HANDLE:
                // TODO
                pc += 3;
                goto *opcodes[code[pc]];
            default: {
                JClass *excpCls = Flint::findClass(this, "java/lang/ClassFormatError");
                throwNew(excpCls, "Constant pool tag value (%u) is invalid in class %s", loader->getConstPoolTag(poolIndex), loader->getName());
                return;
            }
        }
    }
    op_ldc2_w: {
        uint16_t poolIndex = ARRAY_TO_INT16(&code[pc + 1]);
        ClassLoader *loader = method->loader;
        switch(loader->getConstPoolTag(poolIndex)) {
            case CONST_LONG:
                stackPushInt64(loader->getConstLong(poolIndex));
                pc += 3;
                goto *opcodes[code[pc]];
            case CONST_DOUBLE:
                stackPushDouble(loader->getConstDouble(poolIndex));
                pc += 3;
                goto *opcodes[code[pc]];
            default: {
                JClass *excpCls = Flint::findClass(this, "java/lang/ClassFormatError");
                throwNew(excpCls, "Constant pool tag value (%u) is invalid in class %s", loader->getConstPoolTag(poolIndex), loader->getName());
                return;
            }
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
        stackPushObject((JObject *)locals[code[pc + 1]]);
        pc += 2;
        goto *opcodes[code[pc]];
    op_aload_0:
        stackPushObject((JObject *)locals[0]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload_1:
        stackPushObject((JObject *)locals[1]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload_2:
        stackPushObject((JObject *)locals[2]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload_3:
        stackPushObject((JObject *)locals[3]);
        pc++;
        goto *opcodes[code[pc]];
    op_iaload:
    op_faload: {
        int32_t index = stackPopInt32();
        JObject *obj = stackPopObject();
        if(obj == 0)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj->size / sizeof(int32_t))) {
            JClass *excpCls = Flint::findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int32_t)));
            goto exception_handler;
        }
        stackPushInt32(((int32_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_laload:
    op_daload: {
        int32_t index = stackPopInt32();
        JObject *obj = stackPopObject();
        if(obj == 0)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj->size / sizeof(int64_t))) {
            JClass *excpCls = Flint::findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int64_t)));
            goto exception_handler;
        }
        stackPushInt64(((int64_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_aaload: {
        int32_t index = stackPopInt32();
        JObject *obj = stackPopObject();
        if(obj == 0)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj->size / sizeof(int32_t))) {
            JClass *excpCls = Flint::findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int32_t)));
            goto exception_handler;
        }
        stackPushObject(((JObject **)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_baload: {
        int32_t index = stackPopInt32();
        JObject *obj = stackPopObject();
        if(obj == 0)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj->size / sizeof(int8_t))) {
            JClass *excpCls = Flint::findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int8_t)));
            goto exception_handler;
        }
        stackPushInt32(((int8_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_caload:
    op_saload: {
        int32_t index = stackPopInt32();
        JObject *obj = stackPopObject();
        if(obj == 0)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj->size / sizeof(int16_t))) {
            JClass *excpCls = Flint::findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int16_t)));
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
        JObject *obj = stackPopObject();
        if(obj == 0)
            goto store_null_array_excp;
        else if((index < 0) || (index >= (obj->size / sizeof(int32_t)))) {
            JClass *excpCls = Flint::findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int32_t)));
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
        JObject *obj = stackPopObject();
        if(obj == 0)
            goto store_null_array_excp;
        else if((index < 0) || (index >= (obj->size / sizeof(int64_t)))) {
            JClass *excpCls = Flint::findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int64_t)));
            goto exception_handler;
        }
        ((int64_t *)obj->data)[index] = value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_bastore: {
        int32_t value = stackPopInt32();
        int32_t index = stackPopInt32();
        JObject *obj = stackPopObject();
        if(obj == 0)
            goto store_null_array_excp;
        else if((index < 0) || (index >= (obj->size / sizeof(int8_t)))) {
            JClass *excpCls = Flint::findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int8_t)));
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
        JObject *obj = stackPopObject();
        if(obj == 0)
            goto store_null_array_excp;
        else if((index < 0) || (index >= (obj->size / sizeof(int16_t)))) {
            JClass *excpCls = Flint::findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int16_t)));
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
        restoreContext();
        code = this->code;
        stackPushInt32(retVal);
        pc = lr;
        goto *opcodes[code[pc]];
    }
    op_lreturn:
    op_dreturn: {
        int64_t retVal = stackPopInt64();
        restoreContext();
        code = this->code;
        stackPushInt64(retVal);
        pc = lr;
        goto *opcodes[code[pc]];
    }
    op_areturn: {
        int32_t retVal = (int32_t)stackPopObject();
        restoreContext();
        code = this->code;
        stackPushObject((JObject *)retVal);
        pc = lr;
        goto *opcodes[code[pc]];
    }
    op_return: {
        restoreContext();
        code = this->code;
        peakSp = sp;
        pc = lr;
        goto *opcodes[code[pc]];
    }
    op_getstatic: {
        ConstField *constField = method->loader->getConstField(this, ARRAY_TO_INT16(&code[pc + 1]));
        if(constField == NULL) goto exception_handler;
        ClassLoader *clsLoader = constField->loader;
        if(clsLoader == NULL) {
            clsLoader = Flint::findLoader(this, constField->className);
            if(clsLoader == NULL) goto exception_handler;
            constField->loader = clsLoader;
        }
        StaticInitStatus initStatus = clsLoader->getStaticInitStatus();
        if(initStatus == INITIALIZED || (initStatus == INITIALIZING && clsLoader->monitorOwnId == (uint32_t)this)) {
            switch(constField->nameAndType->desc[0]) {
                case 'J':
                case 'D': {
                    Field64 *fieldData = clsLoader->getStaticField64(this, constField);
                    if(fieldData == NULL) goto exception_handler;
                    stackPushInt64(fieldData->value);
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                case 'L':
                case '[': {
                    FieldObj *fieldData = clsLoader->getStaticFieldObj(this, constField);
                    if(fieldData == NULL) goto exception_handler;
                    stackPushObject(fieldData->value);
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                default: {
                    Field32 *fieldData = clsLoader->getStaticField32(this, constField);
                    if(fieldData == NULL) goto exception_handler;
                    stackPushInt32(fieldData->value);
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
            }
        }
        else if(initStatus == UNINITIALIZED) {
            invokeStaticCtor(clsLoader);
            if(excp != NULL) goto exception_handler;
            code = this->code;
            goto *opcodes[code[pc]];
        }
        FlintAPI::Thread::yield();
        if(hasTerminateRequest()) {
            // TODO - ERROR
            return;
        }
        goto *opcodeLabels[OP_GETSTATIC];
    }
    op_putstatic: {
        ConstField *constField = method->loader->getConstField(this, ARRAY_TO_INT16(&code[pc + 1]));
        if(constField == NULL) goto exception_handler;
        ClassLoader *clsLoader = constField->loader;
        if(clsLoader == NULL) {
            clsLoader = Flint::findLoader(this, constField->className);
            if(clsLoader == NULL) goto exception_handler;
            constField->loader = clsLoader;
        }
        StaticInitStatus initStatus = clsLoader->getStaticInitStatus();
        if(initStatus == INITIALIZED || (initStatus == INITIALIZING && clsLoader->monitorOwnId == (uint32_t)this)) {
            switch(constField->nameAndType->desc[0]) {
                case 'Z':
                case 'B': {
                    Field32 *fieldData = clsLoader->getStaticField32(this, constField);
                    if(fieldData == NULL) goto exception_handler;
                    fieldData->value = (int8_t)stackPopInt32();
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                case 'C':
                case 'S': {
                    Field32 *fieldData = clsLoader->getStaticField32(this, constField);
                    if(fieldData == NULL) goto exception_handler;
                    fieldData->value = (int16_t)stackPopInt32();
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                case 'J':
                case 'D': {
                    Field64 *fieldData = clsLoader->getStaticField64(this, constField);
                    if(fieldData == NULL) goto exception_handler;
                    fieldData->value = stackPopInt64();
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                case 'L':
                case '[': {
                    FieldObj *fieldData = clsLoader->getStaticFieldObj(this, constField);
                    if(fieldData == NULL) goto exception_handler;
                    fieldData->value = stackPopObject();
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                default: {
                    Field32 *fieldData = clsLoader->getStaticField32(this, constField);
                    if(fieldData == NULL) goto exception_handler;
                    fieldData->value = stackPopInt32();
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
            }
        }
        else if(initStatus == UNINITIALIZED) {
            invokeStaticCtor(clsLoader);
            if(excp != NULL) goto exception_handler;
            code = this->code;
            goto *opcodes[code[pc]];
        }
        FlintAPI::Thread::yield();
        if(hasTerminateRequest()) {
            // TODO - ERROR
            return;
        }
        goto *opcodeLabels[OP_PUTSTATIC];
    }
    op_getfield: {
        ConstField *constField = method->loader->getConstField(this, ARRAY_TO_INT16(&code[pc + 1]));
        if(constField == NULL) goto exception_handler;
        JObject *obj = stackPopObject();
        if(obj == 0) {
            JClass *excpCls = Flint::findClass(this, "java/lang/NullPointerException");
            throwNew(excpCls, "Cannot access field %s.%s from null object", constField->className, constField->nameAndType->name);
            goto exception_handler;
        }
        switch(constField->nameAndType->desc[0]) {
            case 'J':
            case 'D': {
                Field64 *fieldData = obj->getField64(this, constField);
                if(fieldData == NULL) goto exception_handler;
                stackPushInt64(fieldData->value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'L':
            case '[': {
                FieldObj *fieldData = obj->getFieldObj(this, constField);
                if(fieldData == NULL) goto exception_handler;
                stackPushObject(fieldData->value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            default: {
                Field32 *fieldData = obj->getField32(this, constField);
                if(fieldData == NULL) goto exception_handler;
                stackPushInt32(fieldData->value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
        }
    }
    op_putfield: {
        ConstField *constField = method->loader->getConstField(this, ARRAY_TO_INT16(&code[pc + 1]));
        if(constField == NULL) goto exception_handler;
        switch(constField->nameAndType->desc[0]) {
            case 'Z':
            case 'B': {
                int32_t value = stackPopInt32();
                JObject *obj = stackPopObject();
                if(obj == 0) {
                    JClass *excpCls = Flint::findClass(this, "java/lang/NullPointerException");
                    throwNew(excpCls, "Cannot access field %s.%s from null object", constField->className, constField->nameAndType->name);
                    goto exception_handler;
                }
                Field32 *fieldData = obj->getField32(this, constField);
                if(fieldData == NULL) goto exception_handler;
                fieldData->value = (int8_t)value;
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'C':
            case 'S': {
                int32_t value = stackPopInt32();
                JObject *obj = stackPopObject();
                if(obj == 0) {
                    JClass *excpCls = Flint::findClass(this, "java/lang/NullPointerException");
                    throwNew(excpCls, "Cannot access field %s.%s from null object", constField->className, constField->nameAndType->name);
                    goto exception_handler;
                }
                Field32 *fieldData = obj->getField32(this, constField);
                if(fieldData == NULL) goto exception_handler;
                fieldData->value = (int16_t)value;
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'J':
            case 'D': {
                int64_t value = stackPopInt64();
                JObject *obj = stackPopObject();
                if(obj == 0) {
                    JClass *excpCls = Flint::findClass(this, "java/lang/NullPointerException");
                    throwNew(excpCls, "Cannot access field %s.%s from null object", constField->className, constField->nameAndType->name);
                    goto exception_handler;
                }
                Field64 *fieldData = obj->getField64(this, constField);
                if(fieldData == NULL) goto exception_handler;
                fieldData->value = value;
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'L':
            case '[': {
                JObject *value = stackPopObject();
                JObject *obj = stackPopObject();
                if(obj == 0) {
                    JClass *excpCls = Flint::findClass(this, "java/lang/NullPointerException");
                    throwNew(excpCls, "Cannot access field %s.%s from null object", constField->className, constField->nameAndType->name);
                    goto exception_handler;
                }
                FieldObj *fieldData = obj->getFieldObj(this, constField);
                if(fieldData == NULL) goto exception_handler;
                fieldData->value = value;
                pc += 3;
                goto *opcodes[code[pc]];
            }
            default: {
                int32_t value = stackPopInt32();
                JObject *obj = stackPopObject();
                if(obj == 0) {
                    JClass *excpCls = Flint::findClass(this, "java/lang/NullPointerException");
                    throwNew(excpCls, "Cannot access field %s.%s from null object", constField->className, constField->nameAndType->name);
                    goto exception_handler;
                }
                Field32 *fieldData = obj->getField32(this, constField);
                if(fieldData == NULL) goto exception_handler;
                fieldData->value = value;
                pc += 3;
                goto *opcodes[code[pc]];
            }
        }
    }
    op_invokevirtual: {
        ConstMethod *constMethod = method->loader->getConstMethod(this, ARRAY_TO_INT16(&code[pc + 1]));
        if(constMethod == NULL) goto exception_handler;
        invokeVirtual(constMethod);
        if(excp != NULL) goto exception_handler;
        code = this->code;
        goto *opcodes[code[pc]];
    }
    op_invokespecial: {
        ConstMethod *constMethod = method->loader->getConstMethod(this, ARRAY_TO_INT16(&code[pc + 1]));
        if(constMethod == NULL) goto exception_handler;
        invokeSpecial(constMethod);
        if(excp != NULL) goto exception_handler;
        code = this->code;
        goto *opcodes[code[pc]];
    }
    op_invokestatic: {
        ConstMethod *constMethod = method->loader->getConstMethod(this, ARRAY_TO_INT16(&code[pc + 1]));
        if(constMethod == NULL) goto exception_handler;
        invokeStatic(constMethod);
        if(excp != NULL) goto exception_handler;
        code = this->code;
        goto *opcodes[code[pc]];
    }
    op_invokeinterface: {
        ConstInterfaceMethod *interfaceMethod = method->loader->getConstInterfaceMethod(this, ARRAY_TO_INT16(&code[pc + 1]));
        if(interfaceMethod == NULL) goto exception_handler;
        uint8_t count = code[pc + 3];
        invokeInterface(interfaceMethod, count);
        if(excp != NULL) goto exception_handler;
        code = this->code;
        goto *opcodes[code[pc]];
    }
    op_invokedynamic: {
        invokeDynamic(method->loader->getConstInvokeDynamic(ARRAY_TO_INT16(&code[pc + 1])));
        if(excp != NULL) goto exception_handler;
        code = this->code;
        goto *opcodes[code[pc]];
    }
    op_new: {
        JClass *cls = method->loader->getConstClass(this, ARRAY_TO_INT16(&code[pc + 1]));
        JObject *obj = Flint::newObject(this, cls);
        if(obj == NULL) goto exception_handler;
        stackPushObject(obj);
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_newarray: {
        static constexpr const char *primArrayTypeName[] = {"[Z", "[C", "[F", "[D", "[B", "[S", "[I", "[J"};
        int32_t count = stackPopInt32();
        if(count < 0)
            goto negative_array_size_excp;
        uint8_t atype = code[pc + 1];
        JObject *obj = Flint::newArray(this, Flint::findClass(this, primArrayTypeName[atype - 4]), count);
        if(obj == NULL) goto exception_handler;
        obj->clearData();
        stackPushObject(obj);
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_anewarray: {
        int32_t count = stackPopInt32();
        if(count < 0)
            goto negative_array_size_excp;
        JClass *cls = method->loader->getConstClass(this, ARRAY_TO_INT16(&code[pc + 1]));
        if(cls == NULL) goto exception_handler;
        cls = Flint::findClassOfArray(this, cls->getTypeName(), 1);
        JObject *array = Flint::newArray(this, cls, count);
        if(array == NULL) goto exception_handler;
        array->clearData();
        stackPushObject(array);
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_arraylength: {
        JObject *obj = stackPopObject();
        if(obj == 0) {
            throwNew(Flint::findClass(this, "java/lang/NullPointerException"), "Cannot read the array length from null object");
            goto exception_handler;
        }
        stackPushInt32(obj->size / obj->type->componentSize());
        pc++;
        goto *opcodes[code[pc]];
    }
    op_athrow: {
        excp = (JThrowable *)stackPopObject();
        if(excp == NULL)
            throwNew(Flint::findClass(this, "java/lang/NullPointerException"), "Cannot throw exception by null object");
        goto exception_handler;
    }
    exception_handler: {
        uint32_t tracePc = pc;
        int32_t traceStartSp = startSp;
        MethodInfo *traceMethod = method;
        JObject *obj = excp;
        if(((uint32_t)obj & 0x01) != 0) return; /* Is a fatal error, not a throwable, Cannot be handled */
        if(dbg && dbg->exceptionIsEnabled())
            dbg->caughtException(this);
        while(1) {
            uint16_t exceptionLength = traceMethod->getExceptionLength();
            for(uint16_t i = 0; i < exceptionLength; i++) {
                ExceptionTable *exception = traceMethod->getException(i);
                if(exception->startPc <= tracePc && tracePc < exception->endPc) {
                    bool isMatch = false;
                    if(exception->catchType == 0)
                        isMatch = true;
                    else {
                        JClass *catchType = traceMethod->loader->getConstClass(this, exception->catchType);
                        if(catchType == NULL) {
                            while(startSp > traceStartSp) restoreContext();
                            code = this->code;
                            sp = startSp + traceMethod->getMaxLocals();
                            pc = exception->handlerPc;
                            goto exception_handler;
                        }
                        isMatch = Flint::isInstanceof(this, obj, catchType);
                        if(isMatch == false && excp != obj) {
                            while(startSp > traceStartSp) restoreContext();
                            code = this->code;
                            sp = startSp + traceMethod->getMaxLocals();
                            pc = exception->handlerPc;
                            goto exception_handler;
                        }
                    }
                    if(isMatch) {
                        excp = NULL;
                        while(startSp > traceStartSp) restoreContext();
                        code = this->code;
                        sp = startSp + traceMethod->getMaxLocals();
                        pc = exception->handlerPc;
                        stackPushObject(obj);
                        goto *opcodes[code[pc]];
                    }
                }
            }
            if(traceStartSp < 4) {
                if(dbg && !dbg->exceptionIsEnabled())
                    dbg->caughtException(this);
                return;
            }
            traceMethod = (MethodInfo *)stack[traceStartSp - 3];
            tracePc = stack[traceStartSp - 2];
            traceStartSp = stack[traceStartSp];
            if(tracePc == 0xFFFFFFFF) return;
        }
    }
    op_checkcast: {
        JObject *obj = (JObject *)stack[sp];
        if(obj != 0) {
            JClass *catchType = method->loader->getConstClass(this, ARRAY_TO_INT16(&code[pc + 1]));
            bool isIns = Flint::isInstanceof(this, obj, catchType);
            if(isIns == false) {
                if(excp == NULL) {
                    JClass *excpCls = Flint::findClass(this, "java/lang/ClassCastException");
                    throwNew(excpCls, "Class %s cannot be cast to class %s", obj->getTypeName(), catchType->getTypeName());
                }
                goto exception_handler;
            }
        }
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_instanceof: {
        JObject *obj = stackPopObject();
        JClass *type = method->loader->getConstClass(this, ARRAY_TO_INT16(&code[pc + 1]));
        bool isIns = Flint::isInstanceof(this, obj, type);
        if(isIns == true)
            stackPushInt32(1);
        else {
            if(excp != NULL) goto exception_handler;
            stackPushInt32(0);
        }
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_monitorenter: {
        JObject *obj = stackPopObject();
        if(obj == 0) {
            throwNew(Flint::findClass(this, "java/lang/NullPointerException"), "Cannot enter synchronized block by null object");
            goto exception_handler;
        }
        if(lockObject(obj) == false) {
            if(excp != NULL) goto exception_handler;
            FlintAPI::Thread::yield();
            goto *opcodes[code[pc]];
        }
        pc++;
        goto *opcodes[code[pc]];
    }
    op_monitorexit: {
        JObject *obj = stackPopObject();
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
                stackPushObject((JObject *)locals[index]);
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
        JClass *cls = method->loader->getConstClass(this, ARRAY_TO_INT16(&code[pc + 1]));
        uint8_t dimensions = code[pc + 3];
        sp -= dimensions - 1;
        for(int32_t i = 0; i < dimensions; i++) {
            if(stack[sp + i] < 0)
                goto negative_array_size_excp;
        }
        JObject *array = Flint::newMultiArray(this, cls, &stack[sp], dimensions);
        if(array == NULL) goto exception_handler;
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
        if(hasTerminateRequest()) {
            // TODO - ERROR
            return;
        }
        uint8_t op = code[pc];
        if(op == OP_BREAKPOINT) { /* Check opcode again, maybe breakpoint was deleted by user */
            op = dbg->getSavedOpcode(pc, method);
            if(op == OP_UNKNOW)
                op = code[pc];
            else if(op == OP_BREAKPOINT) {
                pc++;
                goto *opcodes[op];
            }
        }
        goto *opcodeLabels[op];
    }
    op_breakpoint_dummy: {
        uint8_t op = code[pc];
        if(op == OP_BREAKPOINT_DUMMY) {
            ((uint8_t *)code)[pc] = OP_BREAKPOINT;
            op = dbg->getSavedOpcode(pc, method);
        }
        goto *opcodeLabels[op];
    }
    op_unknow: {
        throwNew(Flint::findClass(this, "java/lang/ClassFormatError"), "Invalid opcode %u", code[pc]);
        return;
    }
    divided_by_zero_excp: {
        throwNew(Flint::findClass(this, "java/lang/ArithmeticException"), "Divided by zero");
        goto exception_handler;
    }
    negative_array_size_excp: {
        throwNew(Flint::findClass(this, "java/lang/NegativeArraySizeException"), "Size of the array is a negative number");
        goto exception_handler;
    }
    load_null_array_excp: {
        throwNew(Flint::findClass(this, "java/lang/NullPointerException"), "Cannot load from null array object");
        goto exception_handler;
    }
    store_null_array_excp: {
        throwNew(Flint::findClass(this, "java/lang/NullPointerException"), "Cannot store to null array object");
        goto exception_handler;
    }
    op_exit:
        return;
}

void FExec::runTask(FExec *exec) {
    exec->exec(true);
    if(exec->excp != NULL) {
        if(((uint32_t)exec->excp & 0x01) == 0) { /* Is throwable object */
            JString *str = exec->excp->getDetailMessage();
            if(str != NULL) {
                Flint::print(exec->excp->getTypeName());
                Flint::print(": ");
                Flint::println(str);
            }
            else
                Flint::println(exec->excp->getTypeName());
        }
        else {  /* Is not throwable object, it is error message */
            const char *msg = (char *)((uint32_t)exec->excp & 0xFFFFFFFC);
            Flint::println(msg);
        }
    }
    while(exec->startSp > 3) exec->restoreContext();
    exec->peakSp = -1;
    Flint::freeExecution(exec);
    FlintAPI::Thread::terminate(0);
}

bool FExec::run(MethodInfo *method, uint32_t argc, ...) {
    if(!opcodes) {
        initExitPoint(method);
        if(argc > 0) {
            va_list args;
            va_start(args, argc);
            stackPushArgs(argc, args);
        }
        invoke(method, argc);
        return (FlintAPI::Thread::create((void (*)(void *))runTask, (void *)this) != 0);
    }
    return false;
}

JClass *FExec::getCallerClass(void) {
    if(startSp < 4) return NULL;
    return ((MethodInfo *)stack[startSp - 3])->loader->getThisClass(this);
}

void FExec::throwNew(JClass *cls, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vThrowNew(cls, msg, args);
}

void FExec::vThrowNew(JClass *cls, const char *msg, va_list args) {
    if(cls == NULL) return;
    if(((uint32_t)excp & 0x01) != 0) return; /* There was a previous fatal error. */
    JThrowable *obj = (JThrowable *)Flint::newObject(NULL, cls);
    if(obj == NULL) {
        alignas(4) static const char err[] = "Cannot create Throwable object";
        excp = (JThrowable *)((uint32_t)err | 0x01);
        return;
    }
    if(msg != NULL) {
        JString *str = Flint::newAscii(NULL, msg, args);
        if(str == NULL) {
            alignas(4) static const char err[] = "Cannot create java/lang/String object";
            excp = (JThrowable *)((uint32_t)err | 0x01);
            return;
        }
        obj->setDetailMessage(str);
    }
    if(excp != NULL) obj->setCause(excp);
    excp = obj;
}

bool FExec::hasException(void) const {
    return (excp != NULL) ? true : false;
}

void FExec::stopRequest(void) {
    opcodes = opcodeLabelsStop;
}

void FExec::terminateRequest(void) {
    opcodes = opcodeLabelsExit;
}

bool FExec::hasTerminateRequest(void) const {
    return (opcodes == opcodeLabelsExit);
}

JThread *FExec::getOnwerThread() {
    Flint::lock();
    if(onwerThread == NULL) {
        JObject *obj = Flint::newObject(this, Flint::findClass(this, "java/lang/Thread"));
        if(obj != NULL) onwerThread = (JThread *)obj;
        Flint::unlock();
        return (JThread *)obj;
    }
    Flint::unlock();
    return onwerThread;
}
