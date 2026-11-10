
#ifndef __FLINT_NATIVE_RUNTIME_H
#define __FLINT_NATIVE_RUNTIME_H

#include "flint_native.h"

jvoid NativeRuntime_ExitInternal(FNIEnv *env, jobject obj, jint status);
jlong NativeRuntime_FreeMemory(FNIEnv *env, jobject obj);
jlong NativeRuntime_TotalMemory(FNIEnv *env, jobject obj);
jvoid NativeRuntime_Gc(FNIEnv *env, jobject obj);

inline constexpr NativeMethod runtimeMethods[] = {
    NATIVE_METHOD("exitInternal", "(I)V", NativeRuntime_ExitInternal),
    NATIVE_METHOD("freeMemory",   "()J",  NativeRuntime_FreeMemory),
    NATIVE_METHOD("totalMemory",  "()J",  NativeRuntime_TotalMemory),
    NATIVE_METHOD("gc",           "()V",  NativeRuntime_Gc),
};

#endif /* __FLINT_NATIVE_RUNTIME_H */
