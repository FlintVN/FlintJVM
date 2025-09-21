
#ifndef __FLINT_NATIVE_REFLECTION_H
#define __FLINT_NATIVE_REFLECTION_H

#include "flint_native.h"

jclass nativeGetCallerClass(FNIEnv *env);
jint nativeGetClassAccessFlags(FNIEnv *env, jclass cls);
jbool nativeAreNestMates(FNIEnv *env, jclass currentClass, jclass memberClass);

static constexpr NativeMethod reflectionMethods[] = {
    NATIVE_METHOD("getCallerClass",      "()Ljava/lang/Class;",                   nativeGetCallerClass),
    NATIVE_METHOD("getClassAccessFlags", "(Ljava/lang/Class;)I",                  nativeGetClassAccessFlags),
    NATIVE_METHOD("areNestMates",        "(Ljava/lang/Class;Ljava/lang/Class;)Z", nativeAreNestMates),
};

#endif /* __FLINT_NATIVE_REFLECTION_H */
