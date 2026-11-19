
#include <math.h>
#include <iostream>
#include <string.h>
#include "flint.h"
#include "flint_opcodes.h"
#include "flint_execution.h"
#include "flint_system_api.h"
#include "flint_default_conf.h"

#define ARRAY_TO_INT16(array)               (int16_t)(((array)[0] << 8) | (array)[1])
#define ARRAY_TO_INT32(array)               (int32_t)(((array)[0] << 24) | ((array)[1] << 16) | ((array)[2] << 8) | (array)[3])

#define GET_STACK_VALUE(_index)             stack[_index]
#define SET_STACK_VALUE(_index, _value)     stack[_index] = _value

#define STACK_PUSH_I32(_value)              stack[++sp] = _value
#define STACK_PUSH_I64(_value)              execPushInt64(stack, sp, (_value))
#define STACK_PUSH_F32(_value)              execPushFloat(stack, sp, (_value))
#define STACK_PUSH_F64(_value)              execPushDouble(stack, sp, (_value))
#define STACK_POP_I32()                     stack[sp--]
#define STACK_POP_I64()                     execPopInt64(stack, sp)
#define STACK_POP_F32()                     execPopFloat(stack, sp)
#define STACK_POP_F64()                     execPopDouble(stack, sp)
#define STACK_POP_OBJ()                     ((JObject *)stack[sp--])

#define PUSH_OBJ(o) do {                                        \
    JObject *_pushObj = (JObject *)(o);                         \
    stack[++sp] = (int32_t)_pushObj;                            \
    peakSp = sp;                                                \
    if (_pushObj && (_pushObj->getProtected() & 0x02))          \
        flint->clearProtLv2(_pushObj);                          \
} while (0)

#define SYNC_STATE()                        do { this->pc = pc; this->sp = sp; } while (0)
#define LOAD_STATE()                        do { pc = this->pc; sp = this->sp; code = this->code; locals = this->locals; } while (0)
#define EXEC_RETURN()                       do { SYNC_STATE(); return; } while (0)
#define THROW_NEW(...)                      do { SYNC_STATE(); FExec::throwNew(__VA_ARGS__); } while (0)

static const void **opcodeLabelsStop = NULL;
static const void **opcodeLabelsExit = NULL;

static inline __attribute__((always_inline)) void execPushInt64(int32_t *stack, int32_t &sp, int64_t value) {
    stack[++sp] = (int32_t)(uint32_t)value;
    stack[++sp] = (int32_t)(uint32_t)((uint64_t)value >> 32);
}

static inline __attribute__((always_inline)) void execPushFloat(int32_t *stack, int32_t &sp, float value) {
    int32_t tmp;
    memcpy(&tmp, &value, sizeof(tmp));
    stack[++sp] = tmp;
}

static inline __attribute__((always_inline)) void execPushDouble(int32_t *stack, int32_t &sp, double value) {
    int64_t tmp;
    memcpy(&tmp, &value, sizeof(tmp));
    execPushInt64(stack, sp, tmp);
}

static inline __attribute__((always_inline)) int64_t execPopInt64(const int32_t *stack, int32_t &sp) {
    uint32_t hi = (uint32_t)stack[sp--];
    uint32_t lo = (uint32_t)stack[sp--];
    return (int64_t)(((uint64_t)hi << 32) | lo);
}

static inline __attribute__((always_inline)) float execPopFloat(const int32_t *stack, int32_t &sp) {
    float ret;
    int32_t tmp = stack[sp--];
    memcpy(&ret, &tmp, sizeof(ret));
    return ret;
}

static inline __attribute__((always_inline)) double execPopDouble(const int32_t *stack, int32_t &sp) {
    double ret;
    int64_t tmp = execPopInt64(stack, sp);
    memcpy(&ret, &tmp, sizeof(ret));
    return ret;
}

jclass FExec::findClass(const char *name, uint16_t length) {
    if ((length == 15 || length == 0xFFFF) && strncmp("java/lang/Class", name, 15) == 0)
        return flint->getClassOfClass(this);
    return flint->findClass(this, name, length, true);
}

jbool FExec::isInstanceof(jobject obj, jclass type) {
    return flint->isInstanceof(this, obj, type);
}

jbool FExec::isAssignableFrom(jclass fromType, jclass toType) {
    return flint->isAssignableFrom(this, fromType, toType);
}

jobject FExec::newObject(jclass type) {
    return flint->newObject(this, type);
}

jobject FExec::newObject(jclass type, jmethodId ctor, ...) {
    if (ctor == NULL) return NULL;
    jobject obj = newObject(type);
    if (obj == NULL) return NULL;
    va_list args;
    va_start(args, ctor);
    uint8_t argc = GetArgSlotCount(ctor->desc);
    stackPushObject(obj);
    stackPushArgs(argc, args);
    va_end(args);
    callMethod(ctor, argc + 1);
    if (hasException() || FExec::hasTerminateRequest()) {
        flint->freeObject(obj);
        return NULL;
    }
    return obj;
}

jstring FExec::newString(const char *format, ...) {
    va_list args;
    va_start(args, format);
    jstring str = flint->newAscii(this, format, args);
    va_end(args);
    return str;
}

jboolArray FExec::newBoolArray(uint32_t count) {
    jboolArray ret = (jboolArray)flint->newArray(this, flint->findClass(this, "[Z"), count);
    if (ret != NULL) ret->clearData();
    return ret;
}

jbyteArray FExec::newByteArray(uint32_t count) {
    jbyteArray ret = (jbyteArray)flint->newArray(this, flint->findClass(this, "[B"), count);
    if (ret != NULL) ret->clearData();
    return ret;
}

jcharArray FExec::newCharArray(uint32_t count) {
    jcharArray ret = (jcharArray)flint->newArray(this, flint->findClass(this, "[C"), count);
    if (ret != NULL) ret->clearData();
    return ret;
}

jshortArray FExec::newShortArray(uint32_t count) {
    jshortArray ret = (jshortArray)flint->newArray(this, flint->findClass(this, "[S"), count);
    if (ret != NULL) ret->clearData();
    return ret;
}

jintArray FExec::newIntArray(uint32_t count) {
    jintArray ret = (jintArray)flint->newArray(this, flint->findClass(this, "[I"), count);
    if (ret != NULL) ret->clearData();
    return ret;
}

jlongArray FExec::newLongArray(uint32_t count) {
    jlongArray ret = (jlongArray)flint->newArray(this, flint->findClass(this, "[J"), count);
    if (ret != NULL) ret->clearData();
    return ret;
}

jfloatArray FExec::newFloatArray(uint32_t count) {
    jfloatArray ret = (jfloatArray)flint->newArray(this, flint->findClass(this, "[F"), count);
    if (ret != NULL) ret->clearData();
    return ret;
}

jdoubleArray FExec::newDoubleArray(uint32_t count) {
    jdoubleArray ret = (jdoubleArray)flint->newArray(this, flint->findClass(this, "[D"), count);
    if (ret != NULL) ret->clearData();
    return ret;
}

jobjectArray FExec::newObjectArray(jclass type, uint32_t count) {
    if (type == NULL) return NULL;
    jclass cls = flint->findClassOfArray(this, type->getTypeName(), 1);
    jobjectArray ret = (jobjectArray)flint->newArray(this, cls, count);
    if (ret != NULL) ret->clearData();
    return ret;
}

static void InvalidAccessFieldType(FExec *exec, jfieldId fid) {
    jclass excp = exec->findClass("java/lang/IllegalAccessException");
    const char *desc = fid->getFieldInfo()->desc;
    const char *name = fid->getFieldInfo()->name;
    exec->throwNew(excp, "Incorrect type access to %s %s", desc, name);
}

jfieldId FExec::getFieldId(jobject obj, const char *name) {
    return obj->getField(this, name);
}

jbool FExec::getBoolField(jfieldId fid) {
    if (fid->getFieldInfo()->desc[0] != 'Z') {
        InvalidAccessFieldType(this, fid);
        return false;
    }
    return fid->getInt32();
}

jbyte FExec::getByteField(jfieldId fid) {
    if (fid->getFieldInfo()->desc[0] != 'B') {
        InvalidAccessFieldType(this, fid);
        return 0;
    }
    return fid->getInt32();
}

jchar FExec::getCharField(jfieldId fid) {
    if (fid->getFieldInfo()->desc[0] != 'C') {
        InvalidAccessFieldType(this, fid);
        return 0;
    }
    return fid->getInt32();
}

jshort FExec::getShortField(jfieldId fid) {
    if (fid->getFieldInfo()->desc[0] != 'S') {
        InvalidAccessFieldType(this, fid);
        return 0;
    }
    return fid->getInt32();
}

jint FExec::getIntField(jfieldId fid) {
    if (fid->getFieldInfo()->desc[0] != 'I') {
        InvalidAccessFieldType(this, fid);
        return 0;
    }
    return fid->getInt32();
}

jfloat FExec::getFloatField(jfieldId fid) {
    if (fid->getFieldInfo()->desc[0] != 'F') {
        InvalidAccessFieldType(this, fid);
        return 0;
    }
    int32_t ret = fid->getInt32();
    return *(float *)&ret;
}

jlong FExec::getLongField(jfieldId fid) {
    if (fid->getFieldInfo()->desc[0] != 'J') {
        InvalidAccessFieldType(this, fid);
        return 0;
    }
    return fid->getInt64();
}

jdouble FExec::getDoubleField(jfieldId fid) {
    if (fid->getFieldInfo()->desc[0] != 'D') {
        InvalidAccessFieldType(this, fid);
        return 0;
    }
    int64_t ret = fid->getInt64();
    return *(double *)&ret;
}

jobject FExec::getObjField(jfieldId fid) {
    const char *desc = fid->getFieldInfo()->desc;
    if (desc[0] == 'L' || desc[0] == '[')
        return fid->getObj();
    InvalidAccessFieldType(this, fid);
    return NULL;
}

jvoid FExec::setBoolField(jfieldId fid, jbool val) {
    if (fid->getFieldInfo()->desc[0] != 'Z')
        InvalidAccessFieldType(this, fid);
    else
        fid->setInt32(val);
}

jvoid FExec::setByteField(jfieldId fid, jbyte val) {
    if (fid->getFieldInfo()->desc[0] != 'B')
        InvalidAccessFieldType(this, fid);
    else
        fid->setInt32(val);
}

jvoid FExec::setCharField(jfieldId fid, jchar val) {
    if (fid->getFieldInfo()->desc[0] != 'C')
        InvalidAccessFieldType(this, fid);
    else
        fid->setInt32(val);
}

jvoid FExec::setShortField(jfieldId fid, jshort val) {
    if (fid->getFieldInfo()->desc[0] != 'S')
        InvalidAccessFieldType(this, fid);
    else
        fid->setInt32(val);
}

jvoid FExec::setIntField(jfieldId fid, jint val) {
    if (fid->getFieldInfo()->desc[0] != 'I')
        InvalidAccessFieldType(this, fid);
    else
        fid->setInt32(val);
}

jvoid FExec::setFloatField(jfieldId fid, jfloat val) {
    if (fid->getFieldInfo()->desc[0] != 'F')
        InvalidAccessFieldType(this, fid);
    else
        fid->setInt32(*(int32_t *)&val);
}

jvoid FExec::setLongField(jfieldId fid, jlong val) {
    if (fid->getFieldInfo()->desc[0] != 'J')
        InvalidAccessFieldType(this, fid);
    else
        fid->setInt64(val);
}

jvoid FExec::setDoubleField(jfieldId fid, jdouble val) {
    if (fid->getFieldInfo()->desc[0] != 'D')
        InvalidAccessFieldType(this, fid);
    else {
        double v = val;
        fid->setInt64(*(int64_t *)&v);
    }
}

jvoid FExec::setObjField(jfieldId fid, jobject obj) {
    const char *desc = fid->getFieldInfo()->desc;
    if (desc[0] == 'L' || desc[0] == '[')
        fid->setObj(obj);
    else
        InvalidAccessFieldType(this, fid);
}

jmethodId FExec::getMethodId(jclass cls, const char *name, const char *sig) {
    ConstNameAndType nameAndType(name, sig);
    return flint->findMethod(this, cls, &nameAndType);
}

jmethodId FExec::getConstructorId(jclass cls, const char *sig) {
    ConstNameAndType nameAndType("<init>", sig);
    return flint->findMethod(this, cls, &nameAndType);
}

uint64_t FExec::vCallMethod(jmethodId mtid, va_list args) {
    if (mtid == NULL) return 0;
    uint8_t argc = GetArgSlotCount(mtid->desc);
    if (!(mtid->accessFlag & METHOD_STATIC)) argc++;
    stackPushArgs(argc, args);
    return callMethod(mtid, argc);
}

jvoid FExec::callVoidMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    vCallMethod(mtid, args);
    va_end(args);
}

jbool FExec::callBoolMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    jbool ret = !!vCallMethod(mtid, args);
    va_end(args);
    return ret;
}

jbyte FExec::callByteMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    jbyte ret = (jbyte)vCallMethod(mtid, args);
    va_end(args);
    return ret;
}

jchar FExec::callCharMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    jchar ret = (jchar)vCallMethod(mtid, args);
    va_end(args);
    return ret;
}

jshort FExec::callShortMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    jshort ret = (jshort)vCallMethod(mtid, args);
    va_end(args);
    return ret;
}

jint FExec::callIntMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    jint ret = (jint)vCallMethod(mtid, args);
    va_end(args);
    return ret;
}

jlong FExec::callLongMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    jlong ret = vCallMethod(mtid, args);
    va_end(args);
    return ret;
}

jfloat FExec::callFloatMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    uint32_t ret = (uint32_t)vCallMethod(mtid, args);
    va_end(args);
    return *(float *)&ret;
}

jdouble FExec::callDoubleMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    uint64_t ret = vCallMethod(mtid, args);
    va_end(args);
    return *(double *)&ret;
}

jobject FExec::callObjectMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    jobject ret = (jobject)vCallMethod(mtid, args);
    va_end(args);
    return ret;
}

jvoid FExec::freeObject(jobject obj) {
    /* Do not free if obj is an instance of jclass */
    if (obj == NULL || obj->type == NULL) return;
    flint->freeObject(obj);
}

FExec::FExec(Flint *flint, JThread *owner, uint32_t stackSize) : ListNode(), FNIEnv(), flint(flint), stackLength(stackSize / sizeof(int32_t)) {
    this->opcodes = 0;
    this->lr = -1;
    this->sp = -1;
    this->startSp = sp;
    this->peakSp = sp;
    this->ownerThread = owner;
    this->excp = NULL;
    this->waitingObj = NULL;
}

Flint *FExec::getFlint(void) const {
    return flint;
}

void FExec::stackPushInt32(int32_t value) {
    stack[++sp] = value;
}

void FExec::stackPushInt64(int64_t value) {
    stack[++sp] = ((uint32_t *)&value)[0];
    stack[++sp] = ((uint32_t *)&value)[1];
}

void FExec::stackPushFloat(float value) {
    stack[++sp] = *(uint32_t *)&value;
}

void FExec::stackPushDouble(double value) {
    stack[++sp] = ((uint32_t *)&value)[0];
    stack[++sp] = ((uint32_t *)&value)[1];
}

void FExec::stackPushObject(JObject *obj) {
    stack[++sp] = (int32_t)obj;
    peakSp = sp;
    if (obj && (obj->getProtected() & 0x02))
        flint->clearProtLv2(obj);
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
    ((uint32_t *)&ret)[1] = stack[sp--];
    ((uint32_t *)&ret)[0] = stack[sp--];
    return *(double *)&ret;
}

JObject *FExec::stackPopObject(void) {
    return (JObject *)stack[sp--];
}

int32_t FExec::getStackTrace(StackFrame *stackTrace, int32_t traceSp) const {
    if (traceSp < 4) return -1;
    uint32_t tracePc = stack[traceSp - 2];
    MethodInfo *traceMethod = (MethodInfo *)stack[traceSp - 3];
    new (stackTrace)StackFrame(tracePc, stack[traceSp], traceMethod);
    return stack[traceSp];
}

bool FExec::getStackTrace(uint32_t index, StackFrame *stackTrace, bool *isEndStack) const {
    if (index == 0) {
        new (stackTrace)StackFrame(pc, startSp, method);
        if (isEndStack) *isEndStack = (startSp < 4);
        return true;
    }
    else {
        int32_t traceSp = startSp;
        do {
            traceSp = getStackTrace(stackTrace, traceSp);
            if (traceSp < 0) return false;
            /* Check if pc == -1 or not to skip exit point */
            if (stackTrace->pc != 0xFFFFFFFF) index--;
        } while (stackTrace->pc == 0xFFFFFFFF || index);
        if (isEndStack) *isEndStack = (traceSp < 4);
        return true;
    }
}

bool FExec::readLocal(uint32_t stackIndex, uint32_t localIndex, uint32_t *value, bool *isObject) const {
    StackFrame stackTrace;
    if (!getStackTrace(stackIndex, &stackTrace, 0)) return false;
    *value = stack[stackTrace.baseSp + 1 + localIndex];
    if (*isObject) *isObject = flint->isObject((void *)*value);
    return true;
}

bool FExec::readLocal(uint32_t stackIndex, uint32_t localIndex, uint64_t *value) const {
    StackFrame stackTrace;
    if (!getStackTrace(stackIndex, &stackTrace, 0)) return false;
    *value = *(int64_t *)&stack[stackTrace.baseSp + 1 + localIndex];
    return true;
}

void FExec::stackPushArgs(uint32_t argc, va_list args) {
    for (uint32_t i = 0; i < argc; i++) {
        int32_t val = va_arg(args, int32_t);
        if (flint->isObject((void *)val)) stackPushObject((JObject *)val);
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
    if (method->accessFlag & (METHOD_SYNCHRONIZED | METHOD_CLINIT) && pc != 0xFFFFFFFF) {
        if (method->accessFlag & METHOD_STATIC) {
            if (method->accessFlag & METHOD_CLINIT)
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
    if (code == NULL)
        return FExec::throwNew(flint->findClass(this, "java/lang/LinkageError"), methodInfo->loader->getName(), methodInfo->name);
    pc = 0;
    locals = &stack[sp + 1];
    sp += maxLocals;
}

void FExec::initExitPoint(MethodInfo *methodInfo) {
    this->method = methodInfo;
    this->pc = -1;
    this->lr = methodInfo->getCodeLength();  /* OP_EXIT - Initialize exit point */
}

bool FExec::lockClass(ClassLoader *cls) {
    flint->lock();
    if (cls->monitorCount == 0 || cls->monitorOwnId == (int32_t)this) {
        cls->monitorOwnId = (int32_t)this;
        if (cls->monitorCount < 0xFFFFFFFF) {
            cls->monitorCount++;
            flint->unlock();
            return true;
        }
        flint->unlock();
        FExec::throwNew(flint->findClass(this, "java/lang/IllegalMonitorStateException"));
        return false;
    }
    flint->unlock();
    return false;
}

void FExec::unlockClass(ClassLoader *cls) {
    flint->lock();
    if (cls->monitorCount)
        cls->monitorCount--;
    flint->unlock();
}

bool FExec::lockObject(JObject *obj) {
    flint->lock();
    if (obj->monitorCount == 0 || obj->ownId == (int32_t)this) {
        obj->ownId = (int32_t)this;
        if (obj->monitorCount < 0xFFFFFFFF) {
            obj->monitorCount++;
            flint->unlock();
            return true;
        }
        flint->unlock();
        FExec::throwNew(flint->findClass(this, "java/lang/IllegalMonitorStateException"));
        return false;
    }
    flint->unlock();
    return false;
}

void FExec::unlockObject(JObject *obj) {
    flint->lock();
    if (obj->monitorCount)
        obj->monitorCount--;
    flint->unlock();
}

bool FExec::holdsLock(JObject *obj) {
    if (obj == NULL) return false;
    return obj->ownId == (int32_t)this;
}

bool FExec::checkInvokeArgs(JObject *obj, MethodInfo *methodInfo) {
    if (obj == NULL) {
        JClass *excpCls = flint->findClass(this, "java/lang/NullPointerException");
        FExec::throwNew(excpCls, "Can not invoke \"%s.%s\" by null object", methodInfo->loader->getName(), methodInfo->name);
        return false;
    }
    JClass *cls = methodInfo->loader->getThisClass(this);
    if (cls == NULL) return 0;
    if (!flint->isInstanceof(this, obj, cls)) {
        JClass *excpCls = flint->findClass(this, "java/lang/IncompatibleClassChangeError");
        FExec::throwNew(excpCls, "object type %s cannot be used as the \"this\" parameter for the \"%s.%s\" method", obj->getTypeName(), methodInfo->loader->getName(), methodInfo->name);
        return false;
    }
    return true;
}

uint64_t FExec::callMethod(MethodInfo *methodInfo, uint8_t argc) {
    MethodAccessFlag flag = methodInfo->accessFlag;
    if (!(flag & METHOD_NATIVE)) {
        peakSp = sp + 4;
        sp -= argc;
        if (peakSp >= stackLength) {
            FExec::throwNew(flint->findClass(this, "java/lang/StackOverflowError"));
            return 0;
        }
        int32_t *args = &stack[sp + 1];
        for (int32_t i = argc - 1; i >= 0; i--) {
            args[i + 4] = args[i];
            __asm__ volatile("" ::: "memory"); /* Keep GCC from turning this loop back into a memmove call */
        }
        stackSaveContext();
        sp += argc;
        initExitPoint(methodInfo);
    }
    if (!(flag & METHOD_STATIC) && !checkInvokeArgs((JObject *)stack[sp - argc + 1], methodInfo)) return 0;

    /* Lock Class/Object if method is SYNCHRONIZED */
    if (flag & (METHOD_SYNCHRONIZED | METHOD_CLINIT)) {
        if (flag & METHOD_STATIC) while (lockClass(methodInfo->loader) == false) {
            if (FExec::hasTerminateRequest()) return 0;
            FlintAPI::Thread::yield();
        }
        else while (lockObject((JObject *)stack[sp - argc + 1]) == false) {
            if (FExec::hasTerminateRequest()) return 0;
            FlintAPI::Thread::yield();
        }
    }

    invoke(methodInfo, argc);
    if (hasException() || FExec::hasTerminateRequest()) return 0;
    if (!(flag & METHOD_NATIVE)) {
        exec(false);
        if (hasException() || FExec::hasTerminateRequest()) return 0;
    }
    uint64_t ret = 0;
    switch (methodInfo->getReturnType()[0]) {
        case 'V': break;
        case 'J':
        case 'D': ret = stackPopInt64(); break;
        case 'L':
        case '[': ret = (uint64_t)stackPopObject(); break;
        default: ret = stackPopInt32(); break;
    }
    if (!(flag & METHOD_NATIVE)) stackRestoreContext();
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
    if (nmtptr == NULL) {
        FExec::throwNew(flint->findClass(this, "java/lang/LinkageError"), "%s.%s", methodInfo->loader->getName(), methodInfo->name);
        return;
    }
    switch (methodInfo->getReturnType()[0]) {
        case 'V': {
            callToNative(this, (void (*)(FNIEnv *, ...))nmtptr, &stack[sp - argc + 1], argc);
            sp = sp - argc;
            if (excp != NULL) return;
            pc = lr;
            return;
        }
        case 'F': {
            float val = callToNative(this, (float (*)(FNIEnv *, ...))nmtptr, &stack[sp - argc + 1], argc);
            sp = sp - argc;
            if (excp != NULL) return;
            stackPushFloat(val);
            pc = lr;
            return;
        }
        case 'J': {
            int64_t val = callToNative(this, (int64_t (*)(FNIEnv *, ...))nmtptr, &stack[sp - argc + 1], argc);
            sp = sp - argc;
            if (excp != NULL) return;
            stackPushInt64(val);
            pc = lr;
            return;
        }
        case 'D': {
            double val = callToNative(this, (double (*)(FNIEnv *, ...))nmtptr, &stack[sp - argc + 1], argc);
            sp = sp - argc;
            if (excp != NULL) return;
            stackPushDouble(val);
            pc = lr;
            return;
        }
        case 'L':
        case '[': {
            JObject *val = callToNative(this, (JObject *(*)(FNIEnv *, ...))nmtptr, &stack[sp - argc + 1], argc);
            sp = sp - argc;
            if (excp != NULL) return;
            stackPushObject(val);
            pc = lr;
            return;
        }
        default: {
            int32_t val = callToNative(this, (int32_t (*)(FNIEnv *, ...))nmtptr, &stack[sp - argc + 1], argc);
            sp = sp - argc;
            if (excp != NULL) return;
            stackPushInt32(val);
            pc = lr;
            return;
        }
    }
}

void FExec::invoke(MethodInfo *methodInfo, uint8_t argc) {
    if (!(methodInfo->accessFlag & METHOD_NATIVE)) {
        peakSp = sp + 4;
        sp -= argc;
        if ((sp + methodInfo->getMaxLocals() + methodInfo->getMaxStack() + 4) >= stackLength)
            return FExec::throwNew(flint->findClass(this, "java/lang/StackOverflowError"));
        int32_t *args = &stack[sp + 1];
        for (int32_t i = argc - 1; i >= 0; i--) {
            args[i + 4] = args[i];
            __asm__ volatile("" ::: "memory"); /* Keep GCC from turning this loop back into a memmove call */
        }
        stackSaveContext();
        initNewContext(methodInfo, argc);
    }
    else if (methodInfo->accessFlag & (METHOD_SYNCHRONIZED | METHOD_CLINIT)) {
        if (methodInfo->accessFlag & METHOD_STATIC) {
            invokeNativeMethod(methodInfo, argc);
            unlockClass(methodInfo->loader);
        }
        else {
            JObject *obj = (JObject *)stack[sp - argc + 1];
            invokeNativeMethod(methodInfo, argc);
            unlockObject(obj);
        }
    }
    else
        invokeNativeMethod(methodInfo, argc);
}

void FExec::invokeStatic(ConstMethod *constMethod) {
    MethodInfo *methodInfo = constMethod->methodInfo;
    if (methodInfo == NULL) {
        methodInfo = flint->findMethod(this, flint->findClass(this, constMethod->className), constMethod->nameAndType);
        if (methodInfo == NULL) return;
        constMethod->methodInfo = methodInfo;
        if (methodInfo->loader->getStaticInitStatus() == UNINITIALIZED)
            return invokeStaticCtor(methodInfo->loader);
    }
    if (methodInfo->accessFlag & (METHOD_SYNCHRONIZED | METHOD_CLINIT)) {
        if (lockClass(methodInfo->loader) == false)
            return FlintAPI::Thread::yield();
    }
    lr = pc + 3;
    invoke(methodInfo, constMethod->getArgc());
}

void FExec::invokeSpecial(ConstMethod *constMethod) {
    uint8_t argc = constMethod->getArgc() + 1;
    if ((JObject *)stack[sp - argc + 1] == NULL) {
        JClass *excpCls = flint->findClass(this, "java/lang/NullPointerException");
        return FExec::throwNew(excpCls, "Cannot invoke \"%s.%s\" by null object", constMethod->className, constMethod->nameAndType->name);
    }
    MethodInfo *methodInfo = constMethod->methodInfo;
    if (methodInfo == NULL) {
        methodInfo = flint->findMethod(this, flint->findClass(this, constMethod->className), constMethod->nameAndType);
        if (methodInfo == NULL) return;
        constMethod->methodInfo = methodInfo;
        if (methodInfo->loader->getStaticInitStatus() == UNINITIALIZED)
            return invokeStaticCtor(methodInfo->loader);
    }
    if (methodInfo->accessFlag & (METHOD_SYNCHRONIZED | METHOD_CLINIT)) {
        if (lockObject((JObject *)stack[sp - argc + 1]) == false)
            return FlintAPI::Thread::yield();
    }
    lr = pc + 3;
    invoke(methodInfo, argc);
}

void FExec::invokeVirtual(ConstMethod *constMethod) {
    uint8_t argc = constMethod->getArgc();
    JObject *obj = (JObject *)stack[sp - argc];
    if (obj == NULL) {
        JClass *excpCls = flint->findClass(this, "java/lang/NullPointerException");
        return FExec::throwNew(excpCls, "Cannot invoke \"%s.%s\" by null object", constMethod->className, constMethod->nameAndType->name);
    }
    MethodInfo *methodInfo = constMethod->methodInfo;
    JClass *objType;
    if (obj->type != NULL) objType = obj->type;
    else {
        objType = flint->getClassOfClass(this);
        if (objType == NULL) return;
    }
    if (methodInfo == NULL || constMethod->loaderCache != objType->getClassLoader()) {
        methodInfo = flint->findMethod(this, objType, constMethod->nameAndType);
        if (methodInfo == NULL) return;
        constMethod->methodInfo = methodInfo;
        constMethod->loaderCache = objType->getClassLoader();
        if (methodInfo->loader->getStaticInitStatus() == UNINITIALIZED)
            return invokeStaticCtor(methodInfo->loader);
    }
    if (methodInfo->accessFlag & (METHOD_SYNCHRONIZED | METHOD_CLINIT)) {
        if (lockObject(obj) == false)
            return FlintAPI::Thread::yield();
    }
    argc++;
    lr = pc + 3;
    invoke(methodInfo, argc);
}

void FExec::invokeInterface(ConstInterfaceMethod *interfaceMethod, uint8_t argc) {
    JObject *obj = (JObject *)stack[sp - argc + 1];
    if (obj == NULL) {
        JClass *excpCls = flint->findClass(this, "java/lang/NullPointerException");
        return FExec::throwNew(excpCls, "Cannot invoke \"%s.%s\" by null object", interfaceMethod->className, interfaceMethod->nameAndType->name);
    }
    MethodInfo *methodInfo = interfaceMethod->methodInfo;
    JClass *objType;
    if (obj->type != NULL) objType = obj->type;
    else {
        objType = flint->getClassOfClass(this);
        if (objType == NULL) return;
    }
    if (methodInfo == NULL || interfaceMethod->loaderCache != objType->getClassLoader()) {
        methodInfo = flint->findMethod(this, objType, interfaceMethod->nameAndType);
        if (methodInfo == NULL) return;
        interfaceMethod->methodInfo = methodInfo;
        interfaceMethod->loaderCache = objType->getClassLoader();
        if (methodInfo->loader->getStaticInitStatus() == UNINITIALIZED)
            return invokeStaticCtor(methodInfo->loader);
    }
    if (methodInfo->accessFlag & (METHOD_SYNCHRONIZED | METHOD_CLINIT)) {
        if (lockObject(obj) == false)
            return FlintAPI::Thread::yield();
    }
    lr = pc + 5;
    invoke(methodInfo, argc);
}

void FExec::invokeStaticCtor(ClassLoader *loader) {
    if (lockClass(loader) == false) { FlintAPI::Thread::yield(); return; }
    if (loader->getStaticInitStatus() != UNINITIALIZED) { unlockClass(loader); return; }
    if (loader->initStaticFields(this) == false) { unlockClass(loader); return; }
    if (loader->hasStaticCtor()) {
        MethodInfo *ctorMethod = loader->getStaticCtor(this);
        if (ctorMethod == NULL) {
            if (excp == NULL) FExec::throwNew(flint->findClass(this, "java/lang/LinkageError"), "<clinit>()");
            return unlockClass(loader);
        }
        if (code[pc] == OP_BREAKPOINT)
            ((uint8_t *)code)[pc] = OP_BREAKPOINT_DUMMY;
        lr = pc;
        invoke(ctorMethod, 0);
    }
    else {
        loader->staticInitialized();
        unlockClass(loader);
    }
}

void FExec::exec(bool initOpcodeLabels) {
    #include "flint_instruction_label.h"

    ::opcodeLabelsStop = (const void **)opcodeLabelsStop;
    ::opcodeLabelsExit = (const void **)opcodeLabelsExit;

    FDbg *dbg = flint->getDebugger();
    if (initOpcodeLabels) opcodes = (const void ** volatile)opcodeLabels;

    const uint8_t *code;
    uint32_t pc;
    int32_t sp;
    int32_t *locals;
    LOAD_STATE();

    if (method->loader->getStaticInitStatus() == UNINITIALIZED) {
        invokeStaticCtor(method->loader);
        LOAD_STATE();
        if (excp != NULL) goto exception_handler;
        goto *opcodes[code[pc]];
    }

    goto *opcodes[code[pc]];
    dbg_stop: {
        SYNC_STATE();
        if (dbg && !dbg->checkStop(this)) {
            if (FExec::hasTerminateRequest()) {
                // TODO - ERROR
                return;
            }
            opcodes = (const void ** volatile)opcodeLabels;
        }
        LOAD_STATE();
        goto *opcodeLabels[code[pc]];
    }
    op_nop:
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_m1:
        STACK_PUSH_I32(-1);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_0:
    op_aconst_null:
        STACK_PUSH_I32(0);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_1:
        STACK_PUSH_I32(1);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_2:
        STACK_PUSH_I32(2);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_3:
        STACK_PUSH_I32(3);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_4:
        STACK_PUSH_I32(4);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_5:
        STACK_PUSH_I32(5);
        pc++;
        goto *opcodes[code[pc]];
    op_lconst_0:
        STACK_PUSH_I64(0);
        pc++;
        goto *opcodes[code[pc]];
    op_lconst_1:
        STACK_PUSH_I64(1);
        pc++;
        goto *opcodes[code[pc]];
    op_fconst_0:
        STACK_PUSH_F32(0);
        pc++;
        goto *opcodes[code[pc]];
    op_fconst_1:
        STACK_PUSH_F32(1);
        pc++;
        goto *opcodes[code[pc]];
    op_fconst_2:
        STACK_PUSH_F32(2);
        pc++;
        goto *opcodes[code[pc]];
    op_dconst_0:
        STACK_PUSH_F64(0);
        pc++;
        goto *opcodes[code[pc]];
    op_dconst_1:
        STACK_PUSH_F64(1);
        pc++;
        goto *opcodes[code[pc]];
    op_bipush:
        STACK_PUSH_I32((int8_t)code[pc + 1]);
        pc += 2;
        goto *opcodes[code[pc]];
    op_sipush:
        STACK_PUSH_I32(ARRAY_TO_INT16(&code[pc + 1]));
        pc += 3;
        goto *opcodes[code[pc]];
    op_ldc: {
        uint8_t poolIndex = code[pc + 1];
        ClassLoader *loader = method->loader;
        switch (loader->getConstPoolTag(poolIndex)) {
            case CONST_INTEGER:
                STACK_PUSH_I32(loader->getConstInteger(poolIndex));
                pc += 2;
                goto *opcodes[code[pc]];
            case CONST_FLOAT:
                STACK_PUSH_F32(loader->getConstFloat(poolIndex));
                pc += 2;
                goto *opcodes[code[pc]];
            case CONST_STRING: {
                JString *str = loader->getConstString(this, poolIndex);
                if (str == NULL) goto exception_handler;
                PUSH_OBJ(str);
                pc += 2;
                goto *opcodes[code[pc]];
            }
            case CONST_CLASS: {
                JClass *cls = loader->getConstClass(this, poolIndex);
                if (cls == NULL) goto exception_handler;
                PUSH_OBJ(cls);
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
                JClass *excpCls = flint->findClass(this, "java/lang/ClassFormatError");
                THROW_NEW(excpCls, "Constant pool tag value (%u) is invalid in class %s", loader->getConstPoolTag(poolIndex), loader->getName());
                EXEC_RETURN();
            }
        }
    }
    op_ldc_w: {
        uint16_t poolIndex = ARRAY_TO_INT16(&code[pc + 1]);
        ClassLoader *loader = method->loader;
        switch (loader->getConstPoolTag(poolIndex)) {
            case CONST_INTEGER:
                STACK_PUSH_I32(loader->getConstInteger(poolIndex));
                pc += 3;
                goto *opcodes[code[pc]];
            case CONST_FLOAT:
                STACK_PUSH_F32(loader->getConstFloat(poolIndex));
                pc += 3;
                goto *opcodes[code[pc]];
            case CONST_STRING: {
                JString *str = loader->getConstString(this, poolIndex);
                if (str == NULL) goto exception_handler;
                PUSH_OBJ(str);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case CONST_CLASS: {
                JClass *cls = loader->getConstClass(this, poolIndex);
                if (cls == NULL) goto exception_handler;
                PUSH_OBJ(cls);
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
                JClass *excpCls = flint->findClass(this, "java/lang/ClassFormatError");
                THROW_NEW(excpCls, "Constant pool tag value (%u) is invalid in class %s", loader->getConstPoolTag(poolIndex), loader->getName());
                EXEC_RETURN();
            }
        }
    }
    op_ldc2_w: {
        uint16_t poolIndex = ARRAY_TO_INT16(&code[pc + 1]);
        ClassLoader *loader = method->loader;
        switch (loader->getConstPoolTag(poolIndex)) {
            case CONST_LONG:
                STACK_PUSH_I64(loader->getConstLong(poolIndex));
                pc += 3;
                goto *opcodes[code[pc]];
            case CONST_DOUBLE:
                STACK_PUSH_F64(loader->getConstDouble(poolIndex));
                pc += 3;
                goto *opcodes[code[pc]];
            default: {
                JClass *excpCls = flint->findClass(this, "java/lang/ClassFormatError");
                THROW_NEW(excpCls, "Constant pool tag value (%u) is invalid in class %s", loader->getConstPoolTag(poolIndex), loader->getName());
                EXEC_RETURN();
            }
        }
    }
    op_iload:
    op_fload:
        STACK_PUSH_I32(locals[code[pc + 1]]);
        pc += 2;
        goto *opcodes[code[pc]];
    op_iload_0:
    op_fload_0:
        STACK_PUSH_I32(locals[0]);
        pc++;
        goto *opcodes[code[pc]];
    op_iload_1:
    op_fload_1:
        STACK_PUSH_I32(locals[1]);
        pc++;
        goto *opcodes[code[pc]];
    op_iload_2:
    op_fload_2:
        STACK_PUSH_I32(locals[2]);
        pc++;
        goto *opcodes[code[pc]];
    op_iload_3:
    op_fload_3:
        STACK_PUSH_I32(locals[3]);
        pc++;
        goto *opcodes[code[pc]];
    op_lload:
    op_dload:
        STACK_PUSH_I64(*(int64_t *)&locals[code[pc + 1]]);
        pc += 2;
        goto *opcodes[code[pc]];
    op_lload_0:
    op_dload_0:
        STACK_PUSH_I64(*(int64_t *)&locals[0]);
        pc++;
        goto *opcodes[code[pc]];
    op_lload_1:
    op_dload_1:
        STACK_PUSH_I64(*(int64_t *)&locals[1]);
        pc++;
        goto *opcodes[code[pc]];
    op_lload_2:
    op_dload_2:
        STACK_PUSH_I64(*(int64_t *)&locals[2]);
        pc++;
        goto *opcodes[code[pc]];
    op_lload_3:
    op_dload_3:
        STACK_PUSH_I64(*(int64_t *)&locals[3]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload:
        PUSH_OBJ((JObject *)locals[code[pc + 1]]);
        pc += 2;
        goto *opcodes[code[pc]];
    op_aload_0:
        PUSH_OBJ((JObject *)locals[0]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload_1:
        PUSH_OBJ((JObject *)locals[1]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload_2:
        PUSH_OBJ((JObject *)locals[2]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload_3:
        PUSH_OBJ((JObject *)locals[3]);
        pc++;
        goto *opcodes[code[pc]];
    op_iaload:
    op_faload: {
        int32_t index = STACK_POP_I32();
        JObject *obj = STACK_POP_OBJ();
        if (obj == NULL)
            goto load_null_array_excp;
        else if (index < 0 || index >= (obj->size / sizeof(int32_t))) {
            JClass *excpCls = flint->findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            THROW_NEW(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int32_t)));
            goto exception_handler;
        }
        STACK_PUSH_I32(((int32_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_laload:
    op_daload: {
        int32_t index = STACK_POP_I32();
        JObject *obj = STACK_POP_OBJ();
        if (obj == NULL)
            goto load_null_array_excp;
        else if (index < 0 || index >= (obj->size / sizeof(int64_t))) {
            JClass *excpCls = flint->findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            THROW_NEW(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int64_t)));
            goto exception_handler;
        }
        STACK_PUSH_I64(((int64_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_aaload: {
        int32_t index = STACK_POP_I32();
        JObject *obj = STACK_POP_OBJ();
        if (obj == NULL)
            goto load_null_array_excp;
        else if (index < 0 || index >= (obj->size / sizeof(int32_t))) {
            JClass *excpCls = flint->findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            THROW_NEW(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int32_t)));
            goto exception_handler;
        }
        PUSH_OBJ(((JObject **)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_baload: {
        int32_t index = STACK_POP_I32();
        JObject *obj = STACK_POP_OBJ();
        if (obj == NULL)
            goto load_null_array_excp;
        else if (index < 0 || index >= (obj->size / sizeof(int8_t))) {
            JClass *excpCls = flint->findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            THROW_NEW(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int8_t)));
            goto exception_handler;
        }
        STACK_PUSH_I32(((int8_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_caload: {
        int32_t index = STACK_POP_I32();
        JObject *obj = STACK_POP_OBJ();
        if (obj == NULL)
            goto load_null_array_excp;
        else if (index < 0 || index >= (obj->size / sizeof(uint16_t))) {
            JClass *excpCls = flint->findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            THROW_NEW(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(uint16_t)));
            goto exception_handler;
        }
        STACK_PUSH_I32(((uint16_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_saload: {
        int32_t index = STACK_POP_I32();
        JObject *obj = STACK_POP_OBJ();
        if (obj == NULL)
            goto load_null_array_excp;
        else if (index < 0 || index >= (obj->size / sizeof(int16_t))) {
            JClass *excpCls = flint->findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            THROW_NEW(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int16_t)));
            goto exception_handler;
        }
        STACK_PUSH_I32(((int16_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_istore:
    op_fstore: {
        uint32_t index = code[pc + 1];
        locals[index] = STACK_POP_I32();
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_lstore:
    op_dstore: {
        uint32_t index = code[pc + 1];
        *(uint64_t *)&locals[index] = STACK_POP_I64();
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_astore: {
        uint32_t index = code[pc + 1];
        locals[index] = STACK_POP_I32();
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_istore_0:
    op_fstore_0: {
        locals[0] = STACK_POP_I32();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_istore_1:
    op_fstore_1: {
        locals[1] = STACK_POP_I32();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_istore_2:
    op_fstore_2: {
        locals[2] = STACK_POP_I32();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_istore_3:
    op_fstore_3: {
        locals[3] = STACK_POP_I32();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lstore_0:
    op_dstore_0: {
        *(uint64_t *)&locals[0] = STACK_POP_I64();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lstore_1:
    op_dstore_1: {
        *(uint64_t *)&locals[1] = STACK_POP_I64();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lstore_2:
    op_dstore_2: {
        *(uint64_t *)&locals[2] = STACK_POP_I64();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lstore_3:
    op_dstore_3: {
        *(uint64_t *)&locals[3] = STACK_POP_I64();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_astore_0: {
        locals[0] = STACK_POP_I32();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_astore_1: {
        locals[1] = STACK_POP_I32();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_astore_2: {
        locals[2] = STACK_POP_I32();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_astore_3: {
        locals[3] = STACK_POP_I32();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iastore:
    op_fastore:
    op_aastore: {
        int32_t value = STACK_POP_I32();
        int32_t index = STACK_POP_I32();
        JObject *obj = STACK_POP_OBJ();
        if (obj == NULL)
            goto store_null_array_excp;
        else if ((index < 0) || (index >= (obj->size / sizeof(int32_t)))) {
            JClass *excpCls = flint->findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            THROW_NEW(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int32_t)));
            goto exception_handler;
        }
        ((int32_t *)obj->data)[index] = value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lastore:
    op_dastore: {
        int64_t value = STACK_POP_I64();
        int32_t index = STACK_POP_I32();
        JObject *obj = STACK_POP_OBJ();
        if (obj == NULL)
            goto store_null_array_excp;
        else if ((index < 0) || (index >= (obj->size / sizeof(int64_t)))) {
            JClass *excpCls = flint->findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            THROW_NEW(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int64_t)));
            goto exception_handler;
        }
        ((int64_t *)obj->data)[index] = value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_bastore: {
        int32_t value = STACK_POP_I32();
        int32_t index = STACK_POP_I32();
        JObject *obj = STACK_POP_OBJ();
        if (obj == NULL)
            goto store_null_array_excp;
        else if ((index < 0) || (index >= (obj->size / sizeof(int8_t)))) {
            JClass *excpCls = flint->findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            THROW_NEW(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int8_t)));
            goto exception_handler;
        }
        ((int8_t *)obj->data)[index] = value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_castore:
    op_sastore: {
        int32_t value = STACK_POP_I32();
        int32_t index = STACK_POP_I32();
        JObject *obj = STACK_POP_OBJ();
        if (obj == NULL)
            goto store_null_array_excp;
        else if ((index < 0) || (index >= (obj->size / sizeof(int16_t)))) {
            JClass *excpCls = flint->findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            THROW_NEW(excpCls, "Index %d out of bounds for length %d", index, (obj->size / sizeof(int16_t)));
            goto exception_handler;
        }
        ((int16_t *)obj->data)[index] = value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_pop:
        sp--;
        pc++;
        goto *opcodes[code[pc]];
    op_pop2:
        sp -= 2;
        pc++;
        goto *opcodes[code[pc]];
    op_dup: {
        int32_t value = GET_STACK_VALUE(sp);
        STACK_PUSH_I32(value);
        peakSp = sp;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dup_x1: {
        int32_t value2 = GET_STACK_VALUE(sp - 1);
        int32_t value1 = GET_STACK_VALUE(sp - 0);
        STACK_PUSH_I32(value1);
        peakSp = sp;
        SET_STACK_VALUE(sp - 1, value2);
        SET_STACK_VALUE(sp - 2, value1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dup_x2: {
        int32_t value3 = GET_STACK_VALUE(sp - 2);
        int32_t value2 = GET_STACK_VALUE(sp - 1);
        int32_t value1 = GET_STACK_VALUE(sp - 0);
        STACK_PUSH_I32(value1);
        peakSp = sp;
        SET_STACK_VALUE(sp - 1, value2);
        SET_STACK_VALUE(sp - 2, value3);
        SET_STACK_VALUE(sp - 3, value1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dup2: {
        int32_t value2 = GET_STACK_VALUE(sp - 1);
        int32_t value1 = GET_STACK_VALUE(sp - 0);
        STACK_PUSH_I32(value2);
        STACK_PUSH_I32(value1);
        peakSp = sp;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dup2_x1: {
        int32_t value3 = GET_STACK_VALUE(sp - 2);
        int32_t value2 = GET_STACK_VALUE(sp - 1);
        int32_t value1 = GET_STACK_VALUE(sp - 0);
        STACK_PUSH_I32(value2);
        STACK_PUSH_I32(value1);
        peakSp = sp;
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
        STACK_PUSH_I32(value2);
        STACK_PUSH_I32(value1);
        peakSp = sp;
        SET_STACK_VALUE(sp - 2, value3);
        SET_STACK_VALUE(sp - 3, value4);
        SET_STACK_VALUE(sp - 4, value1);
        SET_STACK_VALUE(sp - 5, value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_swap: {
        int32_t value2 = GET_STACK_VALUE(sp - 1);
        int32_t value1 = GET_STACK_VALUE(sp - 0);
        /* value1 may hold an object pointer. During the swap operation, the reference could temporarily
         * disappear from the stack, creating a risk that the garbage collector may reclaim the object.
         * To prevent this, value1 is pushed onto the stack as a temporary root before the swap is performed. */
        STACK_PUSH_I32(value1);
        peakSp = sp;
        SET_STACK_VALUE(sp - 1, value2);
        SET_STACK_VALUE(sp - 2, value1);
        sp--;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iadd: {
        int32_t value2 = STACK_POP_I32();
        int32_t value1 = STACK_POP_I32();
        STACK_PUSH_I32(value1 + value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ladd: {
        int64_t value2 = STACK_POP_I64();
        int64_t value1 = STACK_POP_I64();
        STACK_PUSH_I64(value1 + value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fadd: {
        float value2 = STACK_POP_F32();
        float value1 = STACK_POP_F32();
        STACK_PUSH_F32(value1 + value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dadd: {
        double value2 = STACK_POP_F64();
        double value1 = STACK_POP_F64();
        STACK_PUSH_F64(value1 + value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_isub: {
        int32_t value2 = STACK_POP_I32();
        int32_t value1 = STACK_POP_I32();
        STACK_PUSH_I32(value1 - value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lsub: {
        int64_t value2 = STACK_POP_I64();
        int64_t value1 = STACK_POP_I64();
        STACK_PUSH_I64(value1 - value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fsub: {
        float value2 = STACK_POP_F32();
        float value1 = STACK_POP_F32();
        STACK_PUSH_F32(value1 - value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dsub: {
        double value2 = STACK_POP_F64();
        double value1 = STACK_POP_F64();
        STACK_PUSH_F64(value1 - value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_imul: {
        int32_t value2 = STACK_POP_I32();
        int32_t value1 = STACK_POP_I32();
        STACK_PUSH_I32(value1 * value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lmul: {
        int64_t value2 = STACK_POP_I64();
        int64_t value1 = STACK_POP_I64();
        STACK_PUSH_I64(value1 * value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fmul: {
        float value2 = STACK_POP_F32();
        float value1 = STACK_POP_F32();
        STACK_PUSH_F32(value1 * value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dmul: {
        double value2 = STACK_POP_F64();
        double value1 = STACK_POP_F64();
        STACK_PUSH_F64(value1 * value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_idiv: {
        int32_t value2 = STACK_POP_I32();
        int32_t value1 = STACK_POP_I32();
        if (value2 == 0)
            goto divided_by_zero_excp;
        STACK_PUSH_I32(value1 / value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ldiv: {
        int64_t value2 = STACK_POP_I64();
        int64_t value1 = STACK_POP_I64();
        if (value2 == 0)
            goto divided_by_zero_excp;
        STACK_PUSH_I64(value1 / value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fdiv: {
        float value2 = STACK_POP_F32();
        float value1 = STACK_POP_F32();
        STACK_PUSH_F32(value1 / value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ddiv: {
        double value2 = STACK_POP_F64();
        double value1 = STACK_POP_F64();
        STACK_PUSH_F64(value1 / value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_irem: {
        int32_t value2 = STACK_POP_I32();
        int32_t value1 = STACK_POP_I32();
        if (value2 == 0)
            goto divided_by_zero_excp;
        STACK_PUSH_I32(value1 % value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lrem: {
        int64_t value2 = STACK_POP_I64();
        int64_t value1 = STACK_POP_I64();
        if (value2 == 0)
            goto divided_by_zero_excp;
        STACK_PUSH_I64(value1 % value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_frem: {
        float value2 = STACK_POP_F32();
        float value1 = STACK_POP_F32();
        STACK_PUSH_F32(fmodf(value1, value2));
        pc++;
        goto *opcodes[code[pc]];
    }
    op_drem: {
        double value2 = STACK_POP_F64();
        double value1 = STACK_POP_F64();
        STACK_PUSH_F64(fmod(value1, value2));
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ineg:
        stack[sp] = -stack[sp];
        pc++;
        goto *opcodes[code[pc]];
    op_lneg:
        STACK_PUSH_I64(-STACK_POP_I64());
        pc++;
        goto *opcodes[code[pc]];
    op_fneg:
        STACK_PUSH_F32(-STACK_POP_F32());
        pc++;
        goto *opcodes[code[pc]];
    op_dneg:
        STACK_PUSH_F64(-STACK_POP_F64());
        pc++;
        goto *opcodes[code[pc]];
    op_ishl: {
        int32_t position = STACK_POP_I32();
        int32_t value = STACK_POP_I32();
        STACK_PUSH_I32(value << position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lshl: {
        int32_t position = STACK_POP_I32();
        int64_t value = STACK_POP_I64();
        STACK_PUSH_I64(value << position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ishr: {
        int32_t position = STACK_POP_I32();
        int32_t value = STACK_POP_I32();
        STACK_PUSH_I32(value >> position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lshr: {
        int32_t position = STACK_POP_I32();
        int64_t value = STACK_POP_I64();
        STACK_PUSH_I64(value >> position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iushr: {
        int32_t position = STACK_POP_I32();
        uint32_t value = STACK_POP_I32();
        STACK_PUSH_I32(value >> position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lushr: {
        int32_t position = STACK_POP_I32();
        uint64_t value = STACK_POP_I64();
        STACK_PUSH_I64(value >> position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iand: {
        int32_t value2 = STACK_POP_I32();
        int32_t value1 = STACK_POP_I32();
        STACK_PUSH_I32(value1 & value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_land: {
        int64_t value2 = STACK_POP_I64();
        int64_t value1 = STACK_POP_I64();
        STACK_PUSH_I64(value1 & value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ior: {
        int32_t value2 = STACK_POP_I32();
        int32_t value1 = STACK_POP_I32();
        STACK_PUSH_I32(value1 | value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lor: {
        int64_t value2 = STACK_POP_I64();
        int64_t value1 = STACK_POP_I64();
        STACK_PUSH_I64(value1 | value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ixor: {
        int32_t value2 = STACK_POP_I32();
        int32_t value1 = STACK_POP_I32();
        STACK_PUSH_I32(value1 ^ value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lxor: {
        int64_t value2 = STACK_POP_I64();
        int64_t value1 = STACK_POP_I64();
        STACK_PUSH_I64(value1 ^ value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iinc:
        locals[code[pc + 1]] += (int8_t)code[pc + 2];
        pc += 3;
        goto *opcodes[code[pc]];
    op_i2l:
        STACK_PUSH_I64(STACK_POP_I32());
        pc++;
        goto *opcodes[code[pc]];
    op_i2f: {
        float value = stack[sp];
        stack[sp] = *(int32_t *)&value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_i2d:
        STACK_PUSH_F64(STACK_POP_I32());
        pc++;
        goto *opcodes[code[pc]];
    op_l2i:
        STACK_PUSH_I32(STACK_POP_I64());
        pc++;
        goto *opcodes[code[pc]];
    op_l2f:
        STACK_PUSH_F32(STACK_POP_I64());
        pc++;
        goto *opcodes[code[pc]];
    op_l2d:
        STACK_PUSH_F64(STACK_POP_I64());
        pc++;
        goto *opcodes[code[pc]];
    op_f2i:
        stack[sp] = *(float *)&stack[sp];
        pc++;
        goto *opcodes[code[pc]];
    op_f2l:
        STACK_PUSH_I64(STACK_POP_F32());
        pc++;
        goto *opcodes[code[pc]];
    op_f2d:
        STACK_PUSH_F64(STACK_POP_F32());
        pc++;
        goto *opcodes[code[pc]];
    op_d2i:
        STACK_PUSH_I32(STACK_POP_F64());
        pc++;
        goto *opcodes[code[pc]];
    op_d2l:
        STACK_PUSH_I64(STACK_POP_F64());
        pc++;
        goto *opcodes[code[pc]];
    op_d2f:
        STACK_PUSH_F32(STACK_POP_F64());
        pc++;
        goto *opcodes[code[pc]];
    op_i2b:
        stack[sp] = (int8_t)stack[sp];
        pc++;
        goto *opcodes[code[pc]];
    op_i2c:
        stack[sp] = (uint16_t)stack[sp];
        pc++;
        goto *opcodes[code[pc]];
    op_i2s:
        stack[sp] = (int16_t)stack[sp];
        pc++;
        goto *opcodes[code[pc]];
    op_lcmp: {
        int64_t value2 = STACK_POP_I64();
        int64_t value1 = STACK_POP_I64();
        STACK_PUSH_I32((value1 == value2) ? 0 : ((value1 < value2) ? -1 : 1));
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fcmpl:
    op_fcmpg: {
        float value2 = STACK_POP_F32();
        float value1 = STACK_POP_F32();
        if ((value1 != value1) || (value2 != value2))
            STACK_PUSH_I32((code[pc] == OP_FCMPL) ? -1 : 1);
        else if (value1 > value2)
            STACK_PUSH_I32(1);
        else if (value1 == value2)
            STACK_PUSH_I32(0);
        else
            STACK_PUSH_I32(-1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dcmpl:
    op_dcmpg: {
        double value2 = STACK_POP_F64();
        double value1 = STACK_POP_F64();
        if ((value1 != value1) || (value2 != value2))
            STACK_PUSH_I32((code[pc] == OP_DCMPL) ? -1 : 1);
        else if (value1 > value2)
            STACK_PUSH_I32(1);
        else if (value1 == value2)
            STACK_PUSH_I32(0);
        else
            STACK_PUSH_I32(-1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ifeq:
    op_ifnull:
        pc += (!STACK_POP_I32()) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_ifne:
    op_ifnonnull:
        pc += STACK_POP_I32() ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_iflt:
        pc += (STACK_POP_I32() < 0) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_ifge:
        pc += (STACK_POP_I32() >= 0) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_ifgt:
        pc += (STACK_POP_I32() > 0) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_ifle:
        pc += (STACK_POP_I32() <= 0) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_if_icmpeq:
    op_if_acmpeq: {
        int32_t value2 = STACK_POP_I32();
        int32_t value1 = STACK_POP_I32();
        pc += (value1 == value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmpne:
    op_if_acmpne: {
        int32_t value2 = STACK_POP_I32();
        int32_t value1 = STACK_POP_I32();
        pc += (value1 != value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmplt: {
        int32_t value2 = STACK_POP_I32();
        int32_t value1 = STACK_POP_I32();
        pc += (value1 < value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmpge: {
        int32_t value2 = STACK_POP_I32();
        int32_t value1 = STACK_POP_I32();
        pc += (value1 >= value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmpgt: {
        int32_t value2 = STACK_POP_I32();
        int32_t value1 = STACK_POP_I32();
        pc += (value1 > value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmple: {
        int32_t value2 = STACK_POP_I32();
        int32_t value1 = STACK_POP_I32();
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
        STACK_PUSH_I32(pc + 3);
        pc += ARRAY_TO_INT16(&code[pc + 1]);
        goto *opcodes[code[pc]];
    op_jsrw:
        STACK_PUSH_I32(pc + 5);
        pc += ARRAY_TO_INT32(&code[pc + 1]);
        goto *opcodes[code[pc]];
    op_ret:
        pc = locals[code[pc + 1]];
        goto *opcodes[code[pc]];
    op_tableswitch: {
        int32_t index = STACK_POP_I32();
        uint8_t padding = (4 - ((pc + 1) % 4)) % 4;
        const uint8_t *table = &code[pc + padding + 1];
        int32_t low = ARRAY_TO_INT32(&table[4]);
        int32_t height = ARRAY_TO_INT32(&table[8]);
        if (index < low || index > height) {
            int32_t defaultOffset = ARRAY_TO_INT32(table);
            pc += defaultOffset;
            goto *opcodes[code[pc]];
        }
        table = &table[12 + (index - low) * 4];
        pc += ARRAY_TO_INT32(table);
        goto *opcodes[code[pc]];
    }
    op_lookupswitch: {
        int32_t key = STACK_POP_I32();
        uint8_t padding = (4 - (pc + 1)) & 0x03;
        const uint8_t *lookupswitch = &code[pc + padding + 1];
        const uint8_t *table = &lookupswitch[8];
        int32_t left = 0;
        int32_t right = ARRAY_TO_INT32(&lookupswitch[4]) - 1;
        while (left <= right) {
            int32_t mid = left + (right - left) / 2;
            const uint8_t *pair = &table[mid << 3];
            int32_t pairKey = ARRAY_TO_INT32(pair);
            if (key == pairKey) {
                pc += ARRAY_TO_INT32(&pair[4]);
                goto *opcodes[code[pc]];
            }
            else if (pairKey < key)
                left = mid + 1;
            else
                right = mid - 1;
        }
        pc += ARRAY_TO_INT32(lookupswitch);
        goto *opcodes[code[pc]];
    }
    op_ireturn:
    op_freturn: {
        int32_t retVal = STACK_POP_I32();
        SYNC_STATE();
        restoreContext();
        LOAD_STATE();
        STACK_PUSH_I32(retVal);
        peakSp = sp;
        pc = lr;
        goto *opcodes[code[pc]];
    }
    op_lreturn:
    op_dreturn: {
        int64_t retVal = STACK_POP_I64();
        SYNC_STATE();
        restoreContext();
        LOAD_STATE();
        STACK_PUSH_I64(retVal);
        peakSp = sp;
        pc = lr;
        goto *opcodes[code[pc]];
    }
    op_areturn: {
        int32_t retVal = (int32_t)STACK_POP_OBJ();
        SYNC_STATE();
        restoreContext();
        LOAD_STATE();
        PUSH_OBJ((JObject *)retVal);
        pc = lr;
        goto *opcodes[code[pc]];
    }
    op_return: {
        SYNC_STATE();
        restoreContext();
        LOAD_STATE();
        peakSp = sp;
        pc = lr;
        goto *opcodes[code[pc]];
    }
    op_getstatic: {
        ConstField *constField = method->loader->getConstField(this, ARRAY_TO_INT16(&code[pc + 1]));
        if (constField == NULL) goto exception_handler;
        ClassLoader *clsLoader = constField->loader;
        if (clsLoader == NULL) {
            clsLoader = flint->findLoader(this, constField->className);
            if (clsLoader == NULL) goto exception_handler;
            constField->loader = clsLoader;
        }
        StaticInitStatus initStatus = clsLoader->getStaticInitStatus();
        if (initStatus == INITIALIZED || (initStatus == INITIALIZING && clsLoader->monitorOwnId == (uint32_t)this)) {
            FieldValue *fieldValue = clsLoader->getStaticField(this, constField);
            if (fieldValue == NULL) goto exception_handler;
            switch (constField->nameAndType->desc[0]) {
                case 'J':
                case 'D': {
                    STACK_PUSH_I64(fieldValue->getInt64());
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                case 'L':
                case '[': {
                    PUSH_OBJ(fieldValue->getObj());
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                default: {
                    STACK_PUSH_I32(fieldValue->getInt32());
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
            }
        }
        else if (initStatus == UNINITIALIZED) {
            SYNC_STATE();
            invokeStaticCtor(clsLoader);
            LOAD_STATE();
            if (excp != NULL) goto exception_handler;
            goto *opcodes[code[pc]];
        }
        SYNC_STATE();
        FlintAPI::Thread::yield();
        if (FExec::hasTerminateRequest()) {
            // TODO - ERROR
            EXEC_RETURN();
        }
        goto *opcodeLabels[OP_GETSTATIC];
    }
    op_putstatic: {
        ConstField *constField = method->loader->getConstField(this, ARRAY_TO_INT16(&code[pc + 1]));
        if (constField == NULL) goto exception_handler;
        ClassLoader *clsLoader = constField->loader;
        if (clsLoader == NULL) {
            clsLoader = flint->findLoader(this, constField->className);
            if (clsLoader == NULL) goto exception_handler;
            constField->loader = clsLoader;
        }
        StaticInitStatus initStatus = clsLoader->getStaticInitStatus();
        if (initStatus == INITIALIZED || (initStatus == INITIALIZING && clsLoader->monitorOwnId == (uint32_t)this)) {
            FieldValue *fieldValue = clsLoader->getStaticField(this, constField);
            if (fieldValue == NULL) goto exception_handler;
            switch (constField->nameAndType->desc[0]) {
                case 'Z':
                case 'B': {
                    fieldValue->setInt32((int8_t)STACK_POP_I32());
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                case 'C':
                case 'S': {
                    fieldValue->setInt32((int16_t)STACK_POP_I32());
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                case 'J':
                case 'D': {
                    fieldValue->setInt64(STACK_POP_I64());
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                case 'L':
                case '[': {
                    fieldValue->setObj(STACK_POP_OBJ());
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
                default: {
                    fieldValue->setInt32(STACK_POP_I32());
                    pc += 3;
                    goto *opcodes[code[pc]];
                }
            }
        }
        else if (initStatus == UNINITIALIZED) {
            SYNC_STATE();
            invokeStaticCtor(clsLoader);
            LOAD_STATE();
            if (excp != NULL) goto exception_handler;
            goto *opcodes[code[pc]];
        }
        SYNC_STATE();
        FlintAPI::Thread::yield();
        if (FExec::hasTerminateRequest()) {
            // TODO - ERROR
            EXEC_RETURN();
        }
        goto *opcodeLabels[OP_PUTSTATIC];
    }
    op_getfield: {
        ConstField *constField = method->loader->getConstField(this, ARRAY_TO_INT16(&code[pc + 1]));
        if (constField == NULL) goto exception_handler;
        JObject *obj = STACK_POP_OBJ();
        if (obj == NULL) {
            JClass *excpCls = flint->findClass(this, "java/lang/NullPointerException");
            THROW_NEW(excpCls, "Cannot access field %s.%s from null object", constField->className, constField->nameAndType->name);
            goto exception_handler;
        }
        FieldValue *fieldValue = obj->getField(this, constField);
        if (fieldValue == NULL) goto exception_handler;
        switch (constField->nameAndType->desc[0]) {
            case 'J':
            case 'D': {
                STACK_PUSH_I64(fieldValue->getInt64());
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'L':
            case '[': {
                PUSH_OBJ(fieldValue->getObj());
                pc += 3;
                goto *opcodes[code[pc]];
            }
            default: {
                STACK_PUSH_I32(fieldValue->getInt32());
                pc += 3;
                goto *opcodes[code[pc]];
            }
        }
    }
    op_putfield: {
        ConstField *constField = method->loader->getConstField(this, ARRAY_TO_INT16(&code[pc + 1]));
        if (constField == NULL) goto exception_handler;
        switch (constField->nameAndType->desc[0]) {
            case 'Z':
            case 'B': {
                int32_t value = STACK_POP_I32();
                JObject *obj = STACK_POP_OBJ();
                if (obj == NULL) {
                    JClass *excpCls = flint->findClass(this, "java/lang/NullPointerException");
                    THROW_NEW(excpCls, "Cannot access field %s.%s from null object", constField->className, constField->nameAndType->name);
                    goto exception_handler;
                }
                FieldValue *fieldValue = obj->getField(this, constField);
                if (fieldValue == NULL) goto exception_handler;
                fieldValue->setInt32((int8_t)value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'C':
            case 'S': {
                int32_t value = STACK_POP_I32();
                JObject *obj = STACK_POP_OBJ();
                if (obj == NULL) {
                    JClass *excpCls = flint->findClass(this, "java/lang/NullPointerException");
                    THROW_NEW(excpCls, "Cannot access field %s.%s from null object", constField->className, constField->nameAndType->name);
                    goto exception_handler;
                }
                FieldValue *fieldValue = obj->getField(this, constField);
                if (fieldValue == NULL) goto exception_handler;
                fieldValue->setInt32((int16_t)value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'J':
            case 'D': {
                int64_t value = STACK_POP_I64();
                JObject *obj = STACK_POP_OBJ();
                if (obj == NULL) {
                    JClass *excpCls = flint->findClass(this, "java/lang/NullPointerException");
                    THROW_NEW(excpCls, "Cannot access field %s.%s from null object", constField->className, constField->nameAndType->name);
                    goto exception_handler;
                }
                FieldValue *fieldValue = obj->getField(this, constField);
                if (fieldValue == NULL) goto exception_handler;
                fieldValue->setInt64(value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'L':
            case '[': {
                JObject *value = STACK_POP_OBJ();
                JObject *obj = STACK_POP_OBJ();
                if (obj == NULL) {
                    JClass *excpCls = flint->findClass(this, "java/lang/NullPointerException");
                    THROW_NEW(excpCls, "Cannot access field %s.%s from null object", constField->className, constField->nameAndType->name);
                    goto exception_handler;
                }
                FieldValue *fieldValue = obj->getField(this, constField);
                if (fieldValue == NULL) goto exception_handler;
                fieldValue->setObj(value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            default: {
                int32_t value = STACK_POP_I32();
                JObject *obj = STACK_POP_OBJ();
                if (obj == NULL) {
                    JClass *excpCls = flint->findClass(this, "java/lang/NullPointerException");
                    THROW_NEW(excpCls, "Cannot access field %s.%s from null object", constField->className, constField->nameAndType->name);
                    goto exception_handler;
                }
                FieldValue *fieldValue = obj->getField(this, constField);
                if (fieldValue == NULL) goto exception_handler;
                fieldValue->setInt32(value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
        }
    }
    op_invokevirtual: {
        ConstMethod *constMethod = method->loader->getConstMethod(this, ARRAY_TO_INT16(&code[pc + 1]));
        if (constMethod == NULL) goto exception_handler;
        SYNC_STATE();
        invokeVirtual(constMethod);
        LOAD_STATE();
        if (excp != NULL) goto exception_handler;
        goto *opcodes[code[pc]];
    }
    op_invokespecial: {
        ConstMethod *constMethod = method->loader->getConstMethod(this, ARRAY_TO_INT16(&code[pc + 1]));
        if (constMethod == NULL) goto exception_handler;
        SYNC_STATE();
        invokeSpecial(constMethod);
        LOAD_STATE();
        if (excp != NULL) goto exception_handler;
        goto *opcodes[code[pc]];
    }
    op_invokestatic: {
        ConstMethod *constMethod = method->loader->getConstMethod(this, ARRAY_TO_INT16(&code[pc + 1]));
        if (constMethod == NULL) goto exception_handler;
        SYNC_STATE();
        invokeStatic(constMethod);
        LOAD_STATE();
        if (excp != NULL) goto exception_handler;
        goto *opcodes[code[pc]];
    }
    op_invokeinterface: {
        ConstInterfaceMethod *interfaceMethod = method->loader->getConstInterfaceMethod(this, ARRAY_TO_INT16(&code[pc + 1]));
        if (interfaceMethod == NULL) goto exception_handler;
        uint8_t count = code[pc + 3];
        SYNC_STATE();
        invokeInterface(interfaceMethod, count);
        LOAD_STATE();
        if (excp != NULL) goto exception_handler;
        goto *opcodes[code[pc]];
    }
    op_invokedynamic: {
        // TODO
        // goto *opcodes[code[pc]];
        JClass *excpCls = flint->findClass(this, "java/lang/UnsupportedOperationException");
        THROW_NEW(excpCls, "Invokedynamic instructions are not supported");
        goto exception_handler;
    }
    op_new: {
        JClass *cls = method->loader->getConstClass(this, ARRAY_TO_INT16(&code[pc + 1]));
        JObject *obj = flint->newObject(this, cls);
        if (obj == NULL) goto exception_handler;
        PUSH_OBJ(obj);
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_newarray: {
        static constexpr const char *primArrayTypeName[] = {"[Z", "[C", "[F", "[D", "[B", "[S", "[I", "[J"};
        int32_t count = STACK_POP_I32();
        if (count < 0)
            goto negative_array_size_excp;
        uint8_t atype = code[pc + 1];
        JObject *obj = flint->newArray(this, flint->findClass(this, primArrayTypeName[atype - 4]), count);
        if (obj == NULL) goto exception_handler;
        obj->clearData();
        PUSH_OBJ(obj);
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_anewarray: {
        int32_t count = STACK_POP_I32();
        if (count < 0)
            goto negative_array_size_excp;
        JClass *cls = method->loader->getConstClass(this, ARRAY_TO_INT16(&code[pc + 1]));
        if (cls == NULL) goto exception_handler;
        cls = flint->findClassOfArray(this, cls->getTypeName(), 1);
        JObject *array = flint->newArray(this, cls, count);
        if (array == NULL) goto exception_handler;
        array->clearData();
        PUSH_OBJ(array);
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_arraylength: {
        JObject *obj = STACK_POP_OBJ();
        if (obj == NULL) {
            THROW_NEW(flint->findClass(this, "java/lang/NullPointerException"), "Cannot read the array length from null object");
            goto exception_handler;
        }
        STACK_PUSH_I32(obj->size / obj->type->componentSize());
        pc++;
        goto *opcodes[code[pc]];
    }
    op_athrow: {
        excp = (JThrowable *)STACK_POP_OBJ();
        if (excp == NULL)
            THROW_NEW(flint->findClass(this, "java/lang/NullPointerException"), "Cannot throw exception by null object");
        goto exception_handler;
    }
    exception_handler: {
        SYNC_STATE();
        uint32_t tracePc = pc;
        int32_t traceStartSp = startSp;
        MethodInfo *traceMethod = method;
        JObject *obj = excp;
        if (((uint32_t)obj & 0x01) != 0) EXEC_RETURN(); /* Is a fatal error, not a throwable, Cannot be handled */
        if (dbg && dbg->exceptionIsEnabled())
            dbg->caughtException(this);
        while (1) {
            uint16_t exceptionLength = traceMethod->getExceptionEntryLength();
            for (uint16_t i = 0; i < exceptionLength; i++) {
                ExceptionEntry *exception = traceMethod->getExceptionEntry(i);
                if (exception->startPc <= tracePc && tracePc < exception->endPc) {
                    bool isMatch = false;
                    if (exception->catchType == 0)
                        isMatch = true;
                    else {
                        JClass *catchType = traceMethod->loader->getConstClass(this, exception->catchType);
                        if (catchType == NULL) {
                            while (startSp > traceStartSp) restoreContext();
                            LOAD_STATE();
                            sp = startSp + traceMethod->getMaxLocals();
                            pc = exception->handlerPc;
                            peakSp = sp;
                            goto exception_handler;
                        }
                        isMatch = flint->isInstanceof(this, obj, catchType);
                        if (isMatch == false && excp != obj) {
                            while (startSp > traceStartSp) restoreContext();
                            LOAD_STATE();
                            sp = startSp + traceMethod->getMaxLocals();
                            pc = exception->handlerPc;
                            peakSp = sp;
                            goto exception_handler;
                        }
                    }
                    if (isMatch) {
                        while (startSp > traceStartSp) restoreContext();
                        LOAD_STATE();
                        sp = startSp + traceMethod->getMaxLocals();
                        pc = exception->handlerPc;
                        PUSH_OBJ(obj);
                        excp = NULL;
                        goto *opcodes[code[pc]];
                    }
                }
            }
            if (traceStartSp < 4) {
                if (dbg && !dbg->exceptionIsEnabled())
                    dbg->caughtException(this);
                EXEC_RETURN();
            }
            traceMethod = (MethodInfo *)stack[traceStartSp - 3];
            tracePc = stack[traceStartSp - 2];
            traceStartSp = stack[traceStartSp];
            if (tracePc == 0xFFFFFFFF) EXEC_RETURN();
        }
    }
    op_checkcast: {
        JObject *obj = (JObject *)stack[sp];
        if (obj != NULL) {
            JClass *catchType = method->loader->getConstClass(this, ARRAY_TO_INT16(&code[pc + 1]));
            bool isIns = flint->isInstanceof(this, obj, catchType);
            if (isIns == false) {
                if (excp == NULL) {
                    JClass *excpCls = flint->findClass(this, "java/lang/ClassCastException");
                    THROW_NEW(excpCls, "Class %s cannot be cast to class %s", obj->getTypeName(), catchType->getTypeName());
                }
                goto exception_handler;
            }
        }
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_instanceof: {
        JObject *obj = STACK_POP_OBJ();
        JClass *type = method->loader->getConstClass(this, ARRAY_TO_INT16(&code[pc + 1]));
        bool isIns = flint->isInstanceof(this, obj, type);
        if (isIns == true)
            STACK_PUSH_I32(1);
        else {
            if (excp != NULL) goto exception_handler;
            STACK_PUSH_I32(0);
        }
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_monitorenter: {
        JObject *obj = (JObject *)GET_STACK_VALUE(sp);
        if (obj == NULL) {
            THROW_NEW(flint->findClass(this, "java/lang/NullPointerException"), "Cannot enter synchronized block by null object");
            goto exception_handler;
        }
        if (lockObject(obj) == false) {
            if (excp != NULL) goto exception_handler;
            SYNC_STATE();
            FlintAPI::Thread::yield();
            goto *opcodes[code[pc]];
        }
        sp--;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_monitorexit: {
        JObject *obj = STACK_POP_OBJ();
        unlockObject(obj);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_wide: {
        switch ((FlintOpCode)code[pc + 1]) {
            case OP_IINC: {
                uint16_t index = ARRAY_TO_INT16(&code[pc + 2]);
                locals[index] += ARRAY_TO_INT16(&code[pc + 4]);
                pc += 6;
                goto *opcodes[code[pc]];
            }
            case OP_ALOAD: {
                uint16_t index = ARRAY_TO_INT16(&code[pc + 2]);
                PUSH_OBJ((JObject *)locals[index]);
                pc += 4;
                goto *opcodes[code[pc]];
            }
            case OP_FLOAD:
            case OP_ILOAD: {
                uint16_t index = ARRAY_TO_INT16(&code[pc + 2]);
                STACK_PUSH_I32(locals[index]);
                pc += 4;
                goto *opcodes[code[pc]];
            }
            case OP_LLOAD:
            case OP_DLOAD: {
                uint16_t index = ARRAY_TO_INT16(&code[pc + 2]);
                STACK_PUSH_I64(*(int64_t *)&locals[index]);
                pc += 4;
                goto *opcodes[code[pc]];
            }
            case OP_ASTORE: {
                uint16_t index = ARRAY_TO_INT16(&code[pc + 2]);
                locals[index] = STACK_POP_I32();
                pc += 4;
                goto *opcodes[code[pc]];
            }
            case OP_FSTORE:
            case OP_ISTORE: {
                uint16_t index = ARRAY_TO_INT16(&code[pc + 2]);
                locals[index] = STACK_POP_I32();
                pc += 4;
                goto *opcodes[code[pc]];
            }
            case OP_LSTORE:
            case OP_DSTORE: {
                uint16_t index = ARRAY_TO_INT16(&code[pc + 2]);
                *(uint64_t *)&locals[index] = STACK_POP_I64();
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
        for (int32_t i = 0; i < dimensions; i++) {
            if (stack[sp - i] < 0)
                goto negative_array_size_excp;
        }
        JObject *array = flint->newMultiArray(this, cls, &stack[sp - dimensions + 1], dimensions);
        if (array == NULL) goto exception_handler;
        sp -= dimensions;
        PUSH_OBJ(array);
        pc += 4;
        goto *opcodes[code[pc]];
    }
    op_breakpoint: {
        if (!dbg) {
            pc++;
            goto *opcodes[code[pc]];
        }
        SYNC_STATE();
        dbg->hitBreakpoint(this);
        if (FExec::hasTerminateRequest()) {
            // TODO - ERROR
            EXEC_RETURN();
        }
        LOAD_STATE();
        uint8_t op = code[pc];
        if (op == OP_BREAKPOINT) { /* Check opcode again, maybe breakpoint was deleted by user */
            op = dbg->getSavedOpcode(pc, method);
            if (op == OP_UNKNOW)
                op = code[pc];
            else if (op == OP_BREAKPOINT) {
                pc++;
                goto *opcodes[op];
            }
        }
        goto *opcodeLabels[op];
    }
    op_breakpoint_dummy: {
        uint8_t op = code[pc];
        if (op == OP_BREAKPOINT_DUMMY) {
            ((uint8_t *)code)[pc] = OP_BREAKPOINT;
            op = dbg->getSavedOpcode(pc, method);
        }
        goto *opcodeLabels[op];
    }
    op_unknow: {
        THROW_NEW(flint->findClass(this, "java/lang/ClassFormatError"), "Invalid opcode %u", code[pc]);
        EXEC_RETURN();
    }
    divided_by_zero_excp: {
        THROW_NEW(flint->findClass(this, "java/lang/ArithmeticException"), "Divided by zero");
        goto exception_handler;
    }
    negative_array_size_excp: {
        THROW_NEW(flint->findClass(this, "java/lang/NegativeArraySizeException"), "Size of the array is a negative number");
        goto exception_handler;
    }
    load_null_array_excp: {
        THROW_NEW(flint->findClass(this, "java/lang/NullPointerException"), "Cannot load from null array object");
        goto exception_handler;
    }
    store_null_array_excp: {
        THROW_NEW(flint->findClass(this, "java/lang/NullPointerException"), "Cannot store to null array object");
        goto exception_handler;
    }
    op_exit:
        EXEC_RETURN();
}

void FExec::runTask(FExec *exec) {
    FlintAPI::Thread::wait(0);
    Flint *flint = exec->getFlint();
    exec->exec(true);
    if (exec->excp != NULL) {
        if (((uint32_t)exec->excp & 0x01) == 0) { /* Is throwable object */
            JString *str = exec->excp->getDetailMessage();
            if (str != NULL) {
                flint->print(exec->excp->getTypeName());
                flint->print(": ");
                flint->println(str);
            }
            else
                flint->println(exec->excp->getTypeName());
        }
        else {  /* Is not throwable object, it is error message */
            const char *msg = (char *)((uint32_t)exec->excp & 0xFFFFFFFC);
            flint->println(msg);
        }
        flint->terminateRequest();
        if (flint->getExitCode() == 0)
            flint->setExitCode(1);
    }
    while (exec->startSp > 3) exec->restoreContext();
    exec->peakSp = -1;
    flint->freeExecution(exec);
    FlintAPI::Thread::terminate(0);
}

bool FExec::run(MethodInfo *method, uint32_t argc, ...) {
    va_list args;
    va_start(args, argc);
    bool ret = vRun(method, argc, args);
    va_end(args);
    return ret;
}

bool FExec::vRun(MethodInfo *method, uint32_t argc, va_list args) {
    if (!opcodes) {
        initExitPoint(method);
        if (argc > 0)
            stackPushArgs(argc, args);
        invoke(method, argc);
        FlintAPI::Thread::ThreadHandle handle = FlintAPI::Thread::create((void (*)(void *))runTask, (void *)this);
        if (handle != NULL) {
            getOwnerThread()->setHandle(handle);
            getOwnerThread()->clearInterrupt();
            FlintAPI::Thread::notify(handle, FlintAPI::Thread::THREAD_NOTIFY_INTERRUPT);
            return true;
        }
    }
    return false;
}

JClass *FExec::getCallerClass(void) {
    if (startSp < 4) return NULL;
    return ((MethodInfo *)stack[startSp - 3])->loader->getThisClass(this);
}

void FExec::throwNew(JClass *cls, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vThrowNew(cls, msg, args);
    va_end(args);
}

void FExec::vThrowNew(JClass *cls, const char *msg, va_list args) {
    if (cls == NULL) return;
    if (((uint32_t)excp & 0x01) != 0) return; /* There was a previous fatal error. */
    JThrowable *obj = (JThrowable *)flint->newObject(NULL, cls);
    if (obj == NULL) {
        alignas(4) static const char err[] = "Cannot create Throwable object";
        excp = (JThrowable *)((uint32_t)err | 0x01);
        return;
    }
    if (msg != NULL) {
        JString *str = flint->newAscii(NULL, msg, args);
        if (str == NULL) {
            alignas(4) static const char err[] = "Cannot create java/lang/String object";
            excp = (JThrowable *)((uint32_t)err | 0x01);
            flint->freeObject(obj);
            return;
        }
        obj->setDetailMessage(str);
    }
    obj->setCause(excp != NULL ? excp : obj);
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
    FlintAPI::Thread::notify(getOwnerThread()->getHandle(), FlintAPI::Thread::THREAD_NOTIFY_TERMINATE);
}

jbool FExec::hasTerminateRequest(void) {
    return (opcodes == opcodeLabelsExit);
}

JThread *FExec::getOwnerThread() {
    return ownerThread;
}
