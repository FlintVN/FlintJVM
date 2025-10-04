
#include "flint.h"
#include "flint_java_class.h"
#include "flint_native_lookup.h"

static void throwNoSuchMethodError(FNIEnv *env, jclass refc) {
    env->throwNew(env->findClass("java/lang/NoSuchMethodError"), "%s.%s", refc->getTypeName(), "<init>");
}

static void throwNoSuchMethodError(FNIEnv *env, jclass refc, jstring name) {
    env->throwNew(env->findClass("java/lang/NoSuchMethodError"), "%s.%.*s", refc->getTypeName(), name->getLength(), name->getAscii());
}

jobject nativeFindStatic(FNIEnv *env, jobject lookup, jclass refc, jstring name, jstring desc) {
    if(refc->isPrimitive()) {
        throwNoSuchMethodError(env, refc, name);
        return NULL;
    }
    jmethodId mt = env->findMethod(refc, name->getAscii(), name->getLength(), desc->getAscii(), desc->getLength());
    if(mt == NULL) return NULL;
    if(!(mt->accessFlag & METHOD_STATIC)) {
        throwNoSuchMethodError(env, refc, name);
        return NULL;
    }
    return Flint::newMethodHandle(env->exec, mt);
}

jobject nativeFindVirtual(FNIEnv *env, jobject lookup, jclass refc, jstring name, jstring desc) {
    if(refc->isPrimitive()) {
        throwNoSuchMethodError(env, refc, name);
        return NULL;
    }
    jmethodId mt = env->findMethod(refc, name->getAscii(), name->getLength(), desc->getAscii(), desc->getLength());
    if(mt == NULL) return NULL;
    if(mt->accessFlag & METHOD_STATIC) {
        throwNoSuchMethodError(env, refc, name);
        return NULL;
    }
    return Flint::newMethodHandle(env->exec, mt);
}

jobject nativeFindConstructor(FNIEnv *env, jobject lookup, jclass refc, jstring desc) {
    if(refc->isPrimitive()) {
        throwNoSuchMethodError(env, refc);
        return NULL;
    }
    jmethodId mt = env->findConstructor(refc, desc->getAscii(), desc->getLength());
    if(mt == NULL) return NULL;
    if(mt->accessFlag & METHOD_STATIC) {
        throwNoSuchMethodError(env, refc);
        return NULL;
    }
    return Flint::newMethodHandle(env->exec, mt);
}
