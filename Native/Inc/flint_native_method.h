
#ifndef __FLINT_NATIVE_METHOD_H
#define __FLINT_NATIVE_METHOD_H

#include "flint_native.h"

jobject NativeMethod_Invoke0(FNIEnv *env, jobject thisObj, jobject obj, jobjectArray args);

static constexpr NativeMethod methodMethods[] = {
    NATIVE_METHOD("invoke0", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;", NativeMethod_Invoke0),
};

#endif /* __FLINT_NATIVE_METHOD_H */
