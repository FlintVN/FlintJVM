
#ifndef __FLINT_NATIVE_OBJECT_H
#define __FLINT_NATIVE_OBJECT_H

#include "flint_native.h"

jclass NativeObject_GetClass(FNIEnv *env, jobject obj);
jint NativeObject_HashCode(FNIEnv *env, jobject obj);
jobject NativeObject_Clone(FNIEnv *env, jobject obj);

static constexpr NativeMethod objectMethods[] = {
    NATIVE_METHOD("getClass", "()Ljava/lang/Class;",  NativeObject_GetClass),
    NATIVE_METHOD("hashCode", "()I",                  NativeObject_HashCode),
    NATIVE_METHOD("clone",    "()Ljava/lang/Object;", NativeObject_Clone),
};

#endif /* __FLINT_NATIVE_OBJECT_H */
