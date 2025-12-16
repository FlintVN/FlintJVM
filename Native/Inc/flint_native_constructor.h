
#ifndef __FLINT_NATIVE_CONSTRUCTOR_H
#define __FLINT_NATIVE_CONSTRUCTOR_H

#include "flint_native.h"

jobject NativeConstructor_NewInstance0(FNIEnv *env, jobject obj, jobjectArray initargs);

static constexpr NativeMethod constructorMethods[] = {
    NATIVE_METHOD("newInstance0", "([Ljava/lang/Object;)Ljava/lang/Object;", NativeConstructor_NewInstance0),
};

#endif /* __FLINT_NATIVE_CONSTRUCTOR_H */
