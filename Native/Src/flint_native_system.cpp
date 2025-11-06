
#include <string.h>
#include "flint_java_class.h"
#include "flint_system_api.h"
#include "flint_array_object.h"
#include "flint_native_system.h"

jvoid nativesetOut0(FNIEnv *env, jobject out) {
    jclass sysCls = env->findClass("java/lang/System");
    if(sysCls == NULL) return;
    FieldValue *outField = sysCls->getClassLoader()->getStaticField(env->exec, "out");
    if(outField == NULL) return;
    outField->setObj(out);
}

jlong nativeCurrentTimeMillis(FNIEnv *env) {
    (void)env;
    return FlintAPI::System::getNanoTime() / 1000000;
}

jlong nativeNanoTime(FNIEnv *env) {
    (void)env;
    return FlintAPI::System::getNanoTime();
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

jvoid nativeArraycopy(FNIEnv *env, jobject src, jint srcPos, jobject dest, jint destPos, jint length) {
    if(checkParam(env, src, srcPos, dest, destPos, length) == false) return;
    void *srcVal = ((jarray)src)->getData();
    void *dstVal = ((jarray)dest)->getData();
    switch(((jarray)src)->componentSize()) {
        case 1:
            for(uint32_t i = 0; i < length; i++)
                ((uint8_t *)dstVal)[i + destPos] = ((uint8_t *)srcVal)[i + srcPos];
            break;
        case 2:
            for(uint32_t i = 0; i < length; i++)
                ((uint16_t *)dstVal)[i + destPos] = ((uint16_t *)srcVal)[i + srcPos];
            break;
        case 4:
            for(uint32_t i = 0; i < length; i++)
                ((uint32_t *)dstVal)[i + destPos] = ((uint32_t *)srcVal)[i + srcPos];
            break;
        case 8:
            for(uint32_t i = 0; i < length; i++)
                ((uint64_t *)dstVal)[i + destPos] = ((uint64_t *)srcVal)[i + srcPos];
            break;
    }
}

jint nativeIdentityHashCode(FNIEnv *env, jobject obj) {
    (void)env;
    return (int32_t)obj;
}
