
#ifndef __FLINT_NATIVE_REFLECTION_H
#define __FLINT_NATIVE_REFLECTION_H

#include "flint_native.h"

jclass NativeReflection_GetCallerClass(FNIEnv *env);
jint NativeReflection_GetClassAccessFlags(FNIEnv *env, jclass cls);
jbool NativeReflection_AreNestMates(FNIEnv *env, jclass currentClass, jclass memberClass);

inline constexpr NativeMethod reflectionMethods[] = {
    NATIVE_METHOD("getCallerClass",      "()Ljava/lang/Class;",                   NativeReflection_GetCallerClass),
    NATIVE_METHOD("getClassAccessFlags", "(Ljava/lang/Class;)I",                  NativeReflection_GetClassAccessFlags),
    NATIVE_METHOD("areNestMates",        "(Ljava/lang/Class;Ljava/lang/Class;)Z", NativeReflection_AreNestMates),
};

#endif /* __FLINT_NATIVE_REFLECTION_H */
