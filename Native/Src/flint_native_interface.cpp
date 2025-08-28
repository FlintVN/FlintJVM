
#include <string.h>
#include "flint.h"
#include "flint_native_interface.h"

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
    jclass cls = Flint::findClassOfArray(exec, cls->getTypeName(), 1);
    jobjectArray ret = (jobjectArray)Flint::newArray(exec, cls, count);
    if(ret != NULL) ret->clearData();
    return ret;
}

jvoid FNIEnv::throwNew(jclass cls, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    exec->vThrowNew(cls, msg, args);
}

jvoid FNIEnv::freeObject(jobject obj) {
    /* Do not free if obj is an instance of jclass */
    if(obj == NULL || obj->type == NULL) return;
    Flint::lock();
    Flint::freeObject(obj);
    Flint::unlock();
}
