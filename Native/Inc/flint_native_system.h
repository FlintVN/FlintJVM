
#ifndef __FLINT_NATIVE_SYSTEM_H
#define __FLINT_NATIVE_SYSTEM_H

#include "flint_native.h"

jvoid NativeSystem_setOut0(FNIEnv *env, jobject out);
jlong NativeSystem_CurrentTimeMillis(FNIEnv *env);
jlong NativeSystem_NanoTime(FNIEnv *env);
jvoid NativeSystem_Arraycopy(FNIEnv *env, jobject src, jint srcPos, jobject dest, jint destPos, jint length);
jint NativeSystem_IdentityHashCode(FNIEnv *env, jobject obj);

static constexpr NativeMethod systemMethods[] = {
    NATIVE_METHOD("setOut0",           "(Ljava/io/PrintStream;)V",                   NativeSystem_setOut0),
    NATIVE_METHOD("currentTimeMillis", "()J",                                        NativeSystem_CurrentTimeMillis),
    NATIVE_METHOD("nanoTime",          "()J",                                        NativeSystem_NanoTime),
    NATIVE_METHOD("arraycopy",         "(Ljava/lang/Object;ILjava/lang/Object;II)V", NativeSystem_Arraycopy),
    NATIVE_METHOD("identityHashCode",  "(Ljava/lang/Object;)I",                      NativeSystem_IdentityHashCode),
};

#endif /* __FLINT_NATIVE_SYSTEM_H */
