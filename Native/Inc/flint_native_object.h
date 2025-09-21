
#ifndef __FLINT_NATIVE_OBJECT_H
#define __FLINT_NATIVE_OBJECT_H

#include "flint_native.h"

jclass nativeGetClass(FNIEnv *env, jobject obj);
jint nativeHashCode(FNIEnv *env, jobject obj);
jobject nativeClone(FNIEnv *env, jobject obj);

static constexpr NativeMethod objectMethods[] = {
    NATIVE_METHOD("getClass", "()Ljava/lang/Class;",  nativeGetClass),
    NATIVE_METHOD("hashCode", "()I",                  nativeHashCode),
    NATIVE_METHOD("clone",    "()Ljava/lang/Object;", nativeClone),
};

#endif /* __FLINT_NATIVE_OBJECT_H */
