
#ifndef __FLINT_NATIVE_SYSTEM_H
#define __FLINT_NATIVE_SYSTEM_H

#include "flint_native.h"

jvoid nativesetOut0(FNIEnv *env, jobject out);
jlong nativeCurrentTimeMillis(FNIEnv *env);
jlong nativeNanoTime(FNIEnv *env);
jvoid nativeArraycopy(FNIEnv *env, jobject src, jint srcPos, jobject dest, jint destPos, jint length);
jint nativeIdentityHashCode(FNIEnv *env, jobject obj);

static constexpr NativeMethod systemMethods[] = {
    NATIVE_METHOD("setOut0",           "(Ljava/io/PrintStream;)V",                   nativesetOut0),
    NATIVE_METHOD("currentTimeMillis", "()J",                                        nativeCurrentTimeMillis),
    NATIVE_METHOD("nanoTime",          "()J",                                        nativeNanoTime),
    NATIVE_METHOD("arraycopy",         "(Ljava/lang/Object;ILjava/lang/Object;II)V", nativeArraycopy),
    NATIVE_METHOD("identityHashCode",  "(Ljava/lang/Object;)I",                      nativeIdentityHashCode),
};

#endif /* __FLINT_NATIVE_SYSTEM_H */
