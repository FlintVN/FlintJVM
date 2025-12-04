
#ifndef __FLINT_NATIVE_CLASS_H
#define __FLINT_NATIVE_CLASS_H

#include "flint_native.h"

jclass NativeClass_GetPrimitiveClass(FNIEnv *env, jstring name);
jclass NativeClass_ForName(FNIEnv *env, jstring name);
jbool NativeClass_IsInstance(FNIEnv *env, jclass cls, jobject obj);
jbool NativeClass_IsAssignableFrom(FNIEnv *env, jclass thisCls, jclass cls);
jbool NativeClass_IsInterface(FNIEnv *env, jclass cls);
jbool NativeClass_IsArray(FNIEnv *env, jclass cls);
jbool NativeClass_IsPrimitive(FNIEnv *env, jclass cls);
jstring NativeClass_InitClassName(FNIEnv *env, jclass cls);
jclass NativeClass_GetSuperclass(FNIEnv *env, jclass cls);
jobjectArray NativeClass_GetInterfaces0(FNIEnv *env, jclass cls);
jclass NativeClass_GetComponentType(FNIEnv *env, jclass cls);
jint NativeClass_GetModifiers(FNIEnv *env, jclass cls);
jclass NativeClass_GetNestHost0(FNIEnv *env, jclass cls);
jobjectArray NativeClass_GetNestMembers0(FNIEnv *env, jclass cls);
jbool NativeClass_IsHidden(FNIEnv *env);
jobjectArray NativeClass_GetDeclaredFields0(FNIEnv *env, jclass cls);
jobjectArray NativeClass_GetDeclaredMethods0(FNIEnv *env, jclass cls);
jobjectArray NativeClass_GetDeclaredConstructors0(FNIEnv *env, jclass cls);
jclass NativeClass_GetDeclaringClass0(FNIEnv *env, jclass cls);

static constexpr NativeMethod classMethods[] = {
    NATIVE_METHOD("getPrimitiveClass",        "(Ljava/lang/String;)Ljava/lang/Class;", NativeClass_GetPrimitiveClass),
    NATIVE_METHOD("forName",                  "(Ljava/lang/String;)Ljava/lang/Class;", NativeClass_ForName),
    NATIVE_METHOD("isInstance",               "(Ljava/lang/Object;)Z",                 NativeClass_IsInstance),
    NATIVE_METHOD("isAssignableFrom",         "(Ljava/lang/Class;)Z",                  NativeClass_IsAssignableFrom),
    NATIVE_METHOD("isInterface",              "()Z",                                   NativeClass_IsInterface),
    NATIVE_METHOD("isArray",                  "()Z",                                   NativeClass_IsArray),
    NATIVE_METHOD("isPrimitive",              "()Z",                                   NativeClass_IsPrimitive),
    NATIVE_METHOD("initClassName",            "()Ljava/lang/String;",                  NativeClass_InitClassName),
    NATIVE_METHOD("getSuperclass",            "()Ljava/lang/Class;",                   NativeClass_GetSuperclass),
    NATIVE_METHOD("getInterfaces0",           "()[Ljava/lang/Class;",                  NativeClass_GetInterfaces0),
    NATIVE_METHOD("getComponentType",         "()Ljava/lang/Class;",                   NativeClass_GetComponentType),
    NATIVE_METHOD("getModifiers",             "()I",                                   NativeClass_GetModifiers),
    NATIVE_METHOD("getNestHost0",             "()Ljava/lang/Class;",                   NativeClass_GetNestHost0),
    NATIVE_METHOD("getNestMembers0",          "()[Ljava/lang/Class;",                  NativeClass_GetNestMembers0),
    NATIVE_METHOD("isHidden",                 "()Z",                                   NativeClass_IsHidden),
    NATIVE_METHOD("getDeclaredFields0",       "()[Ljava/lang/reflect/Field;",          NativeClass_GetDeclaredFields0),
    NATIVE_METHOD("getDeclaredMethods0",      "()[Ljava/lang/reflect/Method;",         NativeClass_GetDeclaredMethods0),
    NATIVE_METHOD("getDeclaredConstructors0", "()[Ljava/lang/reflect/Constructor;",    NativeClass_GetDeclaredConstructors0),
    NATIVE_METHOD("getDeclaringClass0",       "()Ljava/lang/Class;",                   NativeClass_GetDeclaringClass0),
};

#endif /* __FLINT_NATIVE_CLASS_H */
