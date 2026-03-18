
#include <string.h>
#include "flint.h"
#include "flint_native_interface.h"

extern uint8_t parseArgc(const char *desc);

jlong::operator int64_t() const {
    uint64_t ret;
    ((uint32_t *)&ret)[0] = low;
    ((uint32_t *)&ret)[1] = high;
    return ret;
}

void jlong::operator=(int64_t value) {
    low = ((uint32_t *)&value)[0];
    high = ((uint32_t *)&value)[1];
}

jdouble::operator double() const {
    double ret;
    ((uint32_t *)&ret)[0] = low;
    ((uint32_t *)&ret)[1] = high;
    return ret;
}

void jdouble::operator=(double value) {
    low = ((uint32_t *)&value)[0];
    high = ((uint32_t *)&value)[1];
}

FNIEnv::FNIEnv(FExec *exec) : exec(exec) {

}

Flint *FNIEnv::getFlint(void) const {
    return exec->getFlint();
}

jvoid FNIEnv::print(const char *txt) const {
    getFlint()->print(txt);
}

jvoid FNIEnv::print(jstring str) const {
    getFlint()->print(str);
}

jclass FNIEnv::findClass(const char *name, uint16_t length) {
    return getFlint()->findClass(exec, name, length, true);
}

jbool FNIEnv::isInstanceof(jobject obj, jclass type) {
    return getFlint()->isInstanceof(exec, obj, type);
}

jbool FNIEnv::isAssignableFrom(jclass fromType, jclass toType) {
    return getFlint()->isAssignableFrom(exec, fromType, toType);
}

jobject FNIEnv::newObject(jclass type) {
    return getFlint()->newObject(exec, type);
}

jobject FNIEnv::newObject(jclass type, jmethodId ctor, ...) {
    if(ctor == NULL) return NULL;
    jobject obj = newObject(type);
    if(obj == NULL) return NULL;
    va_list args;
    va_start(args, ctor);
    uint8_t argc = GetArgSlotCount(ctor->desc);
    exec->stackPushObject(obj);
    exec->stackPushArgs(argc, args);
    exec->callMethod(ctor, argc + 1);
    if(exec->hasException() || exec->hasTerminateRequest()) {
        getFlint()->freeObject(obj);
        return NULL;
    }
    return obj;
}

jstring FNIEnv::newString(const char *format, ...) {
    va_list args;
    va_start(args, format);
    return getFlint()->newAscii(exec, format, args);
}

jboolArray FNIEnv::newBoolArray(uint32_t count) {
    Flint *flint = getFlint();
    jboolArray ret = (jboolArray)flint->newArray(exec, flint->findClass(exec, "[Z"), count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jbyteArray FNIEnv::newByteArray(uint32_t count) {
    Flint *flint = getFlint();
    jbyteArray ret = (jbyteArray)flint->newArray(exec, flint->findClass(exec, "[B"), count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jcharArray FNIEnv::newCharArray(uint32_t count) {
    Flint *flint = getFlint();
    jcharArray ret = (jcharArray)flint->newArray(exec, flint->findClass(exec, "[C"), count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jshortArray FNIEnv::newShortArray(uint32_t count) {
    Flint *flint = getFlint();
    jshortArray ret = (jshortArray)flint->newArray(exec, flint->findClass(exec, "[S"), count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jintArray FNIEnv::newIntArray(uint32_t count) {
    Flint *flint = getFlint();
    jintArray ret = (jintArray)flint->newArray(exec, flint->findClass(exec, "[I"), count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jlongArray FNIEnv::newLongArray(uint32_t count) {
    Flint *flint = getFlint();
    jlongArray ret = (jlongArray)flint->newArray(exec, flint->findClass(exec, "[J"), count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jfloatArray FNIEnv::newFloatArray(uint32_t count) {
    Flint *flint = getFlint();
    jfloatArray ret = (jfloatArray)flint->newArray(exec, flint->findClass(exec, "[F"), count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jdoubleArray FNIEnv::newDoubleArray(uint32_t count) {
    Flint *flint = getFlint();
    jdoubleArray ret = (jdoubleArray)flint->newArray(exec, flint->findClass(exec, "[D"), count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jobjectArray FNIEnv::newObjectArray(jclass type, uint32_t count) {
    if(type == NULL) return NULL;
    Flint *flint = getFlint();
    jclass cls = flint->findClassOfArray(exec, type->getTypeName(), 1);
    jobjectArray ret = (jobjectArray)flint->newArray(exec, cls, count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jmethodId FNIEnv::getMethodId(jclass cls, const char *name, const char *sig) {
    ConstNameAndType nameAndType(name, sig);
    return getFlint()->findMethod(exec, cls, &nameAndType);
}

jmethodId FNIEnv::getConstructorId(jclass cls, const char *sig) {
    ConstNameAndType nameAndType("<init>", sig);
    return getFlint()->findMethod(exec, cls, &nameAndType);
}

uint64_t FNIEnv::vCallMethod(jmethodId mtid, va_list args) {
    if(mtid == NULL) return 0;
    uint8_t argc = GetArgSlotCount(mtid->desc);
    if(!(mtid->accessFlag & METHOD_STATIC)) argc++;
    exec->stackPushArgs(argc, args);
    return exec->callMethod(mtid, argc);
}

jvoid FNIEnv::callVoidMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    vCallMethod(mtid, args);
}

jbool FNIEnv::callBoolMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    return !!vCallMethod(mtid, args);
}

jbyte FNIEnv::callByteMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    return (jbyte)vCallMethod(mtid, args);
}

jchar FNIEnv::callCharMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    return (jchar)vCallMethod(mtid, args);
}

jshort FNIEnv::callShortMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    return (jshort)vCallMethod(mtid, args);
}

jint FNIEnv::callIntMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    return (jint)vCallMethod(mtid, args);
}

jlong FNIEnv::callLongMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    return vCallMethod(mtid, args);
}

jfloat FNIEnv::callFloatMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    uint32_t ret = (uint32_t)vCallMethod(mtid, args);
    return *(float *)&ret;
}

jdouble FNIEnv::callDoubleMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    uint64_t ret = vCallMethod(mtid, args);
    return *(double *)&ret;
}

jobject FNIEnv::callObjectMethod(jmethodId mtid, ...) {
    va_list args;
    va_start(args, mtid);
    return (jobject)vCallMethod(mtid, args);
}

jvoid FNIEnv::throwNew(jclass cls, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    exec->vThrowNew(cls, msg, args);
}

jvoid FNIEnv::freeObject(jobject obj) {
    /* Do not free if obj is an instance of jclass */
    if(obj == NULL || obj->type == NULL) return;
    getFlint()->freeObject(obj);
}
