
#ifndef __FLINT_NATIVE_THREAD_H
#define __FLINT_NATIVE_THREAD_H

#include "flint_native.h"

jvoid NativeThread_Start0(FNIEnv *env, jthread thread);
jvoid NativeThread_Yield0(FNIEnv *env);
jvoid NativeThread_Interrupt0(FNIEnv *env, jthread thread);
jthread NativeThread_CurrentThread(FNIEnv *env);
jvoid NativeThread_Sleep0(FNIEnv *env, jlong millis);

static constexpr NativeMethod threadMethods[] = {
    NATIVE_METHOD("start0",        "()V",                  NativeThread_Start0),
    NATIVE_METHOD("yield0",        "()V",                  NativeThread_Yield0),
    NATIVE_METHOD("interrupt0",    "()V",                  NativeThread_Interrupt0),
    NATIVE_METHOD("currentThread", "()Ljava/lang/Thread;", NativeThread_CurrentThread),
    NATIVE_METHOD("sleep0",        "(J)V",                 NativeThread_Sleep0),
};

#endif /* __FLINT_NATIVE_THREAD_H */
