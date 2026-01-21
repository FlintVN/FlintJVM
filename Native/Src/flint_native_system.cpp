
#include <string.h>
#include "flint_java_class.h"
#include "flint_system_api.h"
#include "flint_array_object.h"
#include "flint_native_system.h"

jvoid NativeSystem_setOut0(FNIEnv *env, jobject out) {
    jclass sysCls = env->findClass("java/lang/System");
    if(sysCls == NULL) return;
    FieldValue *outField = sysCls->getClassLoader()->getStaticField(env->exec, "out");
    if(outField == NULL) return;
    outField->setObj(out);
}

jlong NativeSystem_CurrentTimeMillis(FNIEnv *env) {
    (void)env;
    return FlintAPI::System::getTimeMillis();
}

jlong NativeSystem_NanoTime(FNIEnv *env) {
    (void)env;
    return FlintAPI::System::getTimeNanos();
}

static bool checkParam(FNIEnv *env, jobject src, jint srcPos, jobject dest, jint destPos, jint length) {
    if(src == NULL || dest == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return false;
    }
    if(!src->isArray() || !dest->isArray()) {
        jclass excpCls = env->findClass("java/lang/ArrayStoreException");
        jobject obj = !src->isArray() ? src : dest;
        env->throwNew(excpCls, "%s type %s is not an array", !src->isArray() ? "source" : "destination", obj->getTypeName());
        return false;
    }
    if(src->type != dest->type) {
        jclass excpCls = env->findClass("java/lang/ArrayStoreException");
        const char *msg = "type mismatch, can not copy %.*s[] into %.*s[]";
        uint16_t len1, len2;
        const char *name1 = ((jarray)src)->getCompTypeName(&len1);
        const char *name2 = ((jarray)dest)->getCompTypeName(&len2);
        env->throwNew(excpCls, msg, len1, name1, len2, name2);
        return false;
    }
    if(length < 0) {
        jclass excpCls = env->findClass("java/lang/ArrayIndexOutOfBoundsException");
        env->throwNew(excpCls, "length -%d is negative", length);
        return false;
    }
    if(srcPos < 0) {
        jclass excpCls = env->findClass("java/lang/ArrayIndexOutOfBoundsException");
        uint16_t len;
        const char *name = ((jarray)src)->getCompTypeName(&len);
        env->throwNew(excpCls, "source index %d out of bounds for %.*s[%d]", srcPos, len, name, ((jarray)src)->getLength());
        return false;
    }
    if(destPos < 0) {
        jclass excpCls = env->findClass("java/lang/ArrayIndexOutOfBoundsException");
        uint16_t len;
        const char *name = ((jarray)dest)->getCompTypeName(&len);
        env->throwNew(excpCls, "destination index %d out of bounds for %.*s[%d]", destPos, len, name, ((jarray)dest)->getLength());
        return false;
    }
    if((srcPos + length) > ((jarray)src)->getLength()) {
        jclass excpCls = env->findClass("java/lang/ArrayIndexOutOfBoundsException");
        uint16_t len;
        const char *name = ((jarray)src)->getCompTypeName(&len);
        env->throwNew(excpCls, "last source index %d out of bounds for %.*s[%d]", srcPos + length, len, name, ((jarray)src)->getLength());
        return false;
    }
    if((destPos + length) > ((jarray)dest)->getLength()) {
        jclass excpCls = env->findClass("java/lang/ArrayIndexOutOfBoundsException");
        uint16_t len;
        const char *name = ((jarray)dest)->getCompTypeName(&len);
        env->throwNew(excpCls, "last destination index %d out of bounds for %.*s[%d]", destPos + length, len, name, ((jarray)dest)->getLength());
        return false;
    }
    return true;
}

jvoid NativeSystem_Arraycopy(FNIEnv *env, jobject src, jint srcPos, jobject dest, jint destPos, jint length) {
    if(checkParam(env, src, srcPos, dest, destPos, length) == false) return;
    uint8_t *srcVal = (uint8_t *)((jarray)src)->getData();
    uint8_t *dstVal = (uint8_t *)((jarray)dest)->getData();
    uint8_t compSz = ((jarray)src)->componentSize();
    memcpy(dstVal + destPos * compSz, srcVal + srcPos * compSz, length * compSz);
}

jint NativeSystem_IdentityHashCode(FNIEnv *env, jobject obj) {
    (void)env;
    return (int32_t)obj;
}
