
#ifndef __FLINT_NATIVE_THREAD_H
#define __FLINT_NATIVE_THREAD_H

#include "flint_native.h"

jvoid nativeStart0(FNIEnv *env, jthread thread);
jvoid nativeYield0(FNIEnv *env);
jvoid nativeInterrupt0(FNIEnv *env, jthread thread);
jthread nativeCurrentThread(FNIEnv *env);
jvoid nativeSleep0(FNIEnv *env, jlong millis);

static constexpr NativeMethod threadMethods[] = {
    NATIVE_METHOD("start0",        "()V",                  nativeStart0),
    NATIVE_METHOD("yield0",        "()V",                  nativeYield0),
    NATIVE_METHOD("interrupt0",    "()V",                  nativeInterrupt0),
    NATIVE_METHOD("currentThread", "()Ljava/lang/Thread;", nativeCurrentThread),
    NATIVE_METHOD("sleep0",        "(J)V",                 nativeSleep0),
};

#endif /* __FLINT_NATIVE_THREAD_H */
