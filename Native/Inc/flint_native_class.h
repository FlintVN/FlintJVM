
#ifndef __FLINT_NATIVE_CLASS_H
#define __FLINT_NATIVE_CLASS_H

#include "flint_native.h"

jclass nativeGetPrimitiveClass(FNIEnv *env, jstring name);
jclass nativeForName(FNIEnv *env, jstring name);
jbool nativeIsInstance(FNIEnv *env, jclass cls, jobject obj);
jbool nativeIsAssignableFrom(FNIEnv *env, jclass thisCls, jclass cls);
jbool nativeIsInterface(FNIEnv *env, jclass cls);
jbool nativeIsArray(FNIEnv *env, jclass cls);
jbool nativeIsPrimitive(FNIEnv *env, jclass cls);
jstring nativeInitClassName(FNIEnv *env, jclass cls);
jclass nativeGetSuperclass(FNIEnv *env, jclass cls);
jobjectArray nativeGetInterfaces0(FNIEnv *env, jclass cls);
jclass nativeGetComponentType(FNIEnv *env, jclass cls);
jint nativeGetModifiers(FNIEnv *env, jclass cls);
jclass nativeGetNestHost0(FNIEnv *env, jclass cls);
jbool nativeIsHidden(FNIEnv *env);
jobjectArray nativeGetDeclaredFields0(FNIEnv *env, jclass cls);
jobjectArray nativeGetDeclaredMethods0(FNIEnv *env, jclass cls);
jobjectArray nativeGetDeclaredConstructors0(FNIEnv *env, jclass cls);
jclass nativeGetDeclaringClass0(FNIEnv *env, jclass cls);

static constexpr NativeMethod classMethods[] = {
    NATIVE_METHOD("getPrimitiveClass",        "(Ljava/lang/String;)Ljava/lang/Class;", nativeGetPrimitiveClass),
    NATIVE_METHOD("forName",                  "(Ljava/lang/String;)Ljava/lang/Class;", nativeForName),
    NATIVE_METHOD("isInstance",               "(Ljava/lang/Object;)Z",                 nativeIsInstance),
    NATIVE_METHOD("isAssignableFrom",         "(Ljava/lang/Class;)Z",                  nativeIsAssignableFrom),
    NATIVE_METHOD("isInterface",              "()Z",                                   nativeIsInterface),
    NATIVE_METHOD("isArray",                  "()Z",                                   nativeIsArray),
    NATIVE_METHOD("isPrimitive",              "()Z",                                   nativeIsPrimitive),
    NATIVE_METHOD("initClassName",            "()Ljava/lang/String;",                  nativeInitClassName),
    NATIVE_METHOD("getSuperclass",            "()Ljava/lang/Class;",                   nativeGetSuperclass),
    NATIVE_METHOD("getInterfaces0",           "()[Ljava/lang/Class;",                  nativeGetInterfaces0),
    NATIVE_METHOD("getComponentType",         "()Ljava/lang/Class;",                   nativeGetComponentType),
    NATIVE_METHOD("getModifiers",             "()I",                                   nativeGetModifiers),
    NATIVE_METHOD("getNestHost0",             "()Ljava/lang/Class;",                   nativeGetNestHost0),
    NATIVE_METHOD("isHidden",                 "()Z",                                   nativeIsHidden),
    NATIVE_METHOD("getDeclaredFields0",       "()[Ljava/lang/reflect/Field;",          nativeGetDeclaredFields0),
    NATIVE_METHOD("getDeclaredMethods0",      "()[Ljava/lang/reflect/Method;",         nativeGetDeclaredMethods0),
    NATIVE_METHOD("getDeclaredConstructors0", "()[Ljava/lang/reflect/Constructor;",    nativeGetDeclaredConstructors0),
    NATIVE_METHOD("getDeclaringClass0",       "()Ljava/lang/Class;",                   nativeGetDeclaringClass0),
};

#endif /* __FLINT_NATIVE_CLASS_H */
