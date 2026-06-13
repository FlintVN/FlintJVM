
#ifndef __FLINT_NATIVE_OBJECT_H
#define __FLINT_NATIVE_OBJECT_H

#include "flint_native.h"

jclass NativeObject_GetClass(FNIEnv *env, jobject obj);
jint NativeObject_HashCode(FNIEnv *env, jobject obj);
jobject NativeObject_Clone(FNIEnv *env, jobject obj);
jvoid NativeObject_Notify(FNIEnv *env, jobject obj);
jvoid NativeObject_NotifyAll(FNIEnv *env, jobject obj);
jvoid NativeObject_Wait(FNIEnv *env, jobject obj, jlong millis);

inline constexpr NativeMethod objectMethods[] = {
    NATIVE_METHOD("getClass",  "()Ljava/lang/Class;",  NativeObject_GetClass),
    NATIVE_METHOD("hashCode",  "()I",                  NativeObject_HashCode),
    NATIVE_METHOD("clone",     "()Ljava/lang/Object;", NativeObject_Clone),
    NATIVE_METHOD("notify",    "()V",                  NativeObject_Notify),
    NATIVE_METHOD("notifyAll", "()V",                  NativeObject_NotifyAll),
    NATIVE_METHOD("wait",      "(J)V",                 NativeObject_Wait),
};

#endif /* __FLINT_NATIVE_OBJECT_H */
