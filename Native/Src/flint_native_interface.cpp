
#include <string.h>
#include "flint.h"
#include "flint_native_interface.h"

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

jvoid FNIEnv::print(const char *txt) const {
    Flint::print(txt);
}

jvoid FNIEnv::print(jstring str) const {
    Flint::print(str);
}

jclass FNIEnv::findClass(const char *name, uint16_t length) {
    return Flint::findClass(exec, name, length, true);
}

jmethodId FNIEnv::findConstructor(jclass cls, const char *desc, uint16_t length) {
    return Flint::findMethod(exec, cls, "<init>", 6, desc, length);
}

jmethodId FNIEnv::findMethod(jclass cls, const char *name, const char *desc) {
    return Flint::findMethod(exec, cls, name, 0xFFFF, desc, 0xFFFF);
}

jmethodId FNIEnv::findMethod(jclass cls, const char *name, uint16_t nameLen, const char *desc, uint16_t descLen) {
    return Flint::findMethod(exec, cls, name, nameLen, desc, descLen);
}

jbool FNIEnv::isInstanceof(jobject obj, jclass type) {
    return Flint::isInstanceof(exec, obj, type);
}

jbool FNIEnv::isAssignableFrom(jclass fromType, jclass toType) {
    return Flint::isAssignableFrom(exec, fromType, toType);
}

jobject FNIEnv::newObject(jclass type) {
    return Flint::newObject(exec, type);
}

jstring FNIEnv::newString(const char *format, ...) {
    va_list args;
    va_start(args, format);
    return Flint::newAscii(exec, format, args);
}

jboolArray FNIEnv::newBoolArray(uint32_t count) {
    jboolArray ret = (jboolArray)Flint::newArray(exec, Flint::findClass(exec, "[Z"), count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jbyteArray FNIEnv::newByteArray(uint32_t count) {
    jbyteArray ret = (jbyteArray)Flint::newArray(exec, Flint::findClass(exec, "[B"), count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jcharArray FNIEnv::newCharArray(uint32_t count) {
    jcharArray ret = (jcharArray)Flint::newArray(exec, Flint::findClass(exec, "[C"), count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jshortArray FNIEnv::newShortArray(uint32_t count) {
    jshortArray ret = (jshortArray)Flint::newArray(exec, Flint::findClass(exec, "[S"), count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jintArray FNIEnv::newIntArray(uint32_t count) {
    jintArray ret = (jintArray)Flint::newArray(exec, Flint::findClass(exec, "[I"), count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jlongArray FNIEnv::newLongArray(uint32_t count) {
    jlongArray ret = (jlongArray)Flint::newArray(exec, Flint::findClass(exec, "[J"), count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jfloatArray FNIEnv::newFloatArray(uint32_t count) {
    jfloatArray ret = (jfloatArray)Flint::newArray(exec, Flint::findClass(exec, "[F"), count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jdoubleArray FNIEnv::newDoubleArray(uint32_t count) {
    jdoubleArray ret = (jdoubleArray)Flint::newArray(exec, Flint::findClass(exec, "[D"), count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jobjectArray FNIEnv::newObjectArray(jclass type, uint32_t count) {
    if(type == NULL) return NULL;
    jclass cls = Flint::findClassOfArray(exec, type->getTypeName(), 1);
    jobjectArray ret = (jobjectArray)Flint::newArray(exec, cls, count);
    if(ret != NULL) ret->clearData();
    return ret;
}

uint64_t FNIEnv::vCallMethod(jmethodId mtid, va_list args) {
    if(mtid == NULL) return 0;
    uint8_t argc = getArgSlotCount(mtid->desc);
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
    Flint::freeObject(obj);
}
