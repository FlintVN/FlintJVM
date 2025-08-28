
#ifndef __FLINT_NATIVE_DOUBLE_H
#define __FLINT_NATIVE_DOUBLE_H

#include "flint_native.h"

jlong nativeDoubleToRawLongBits(FNIEnv *env, jdouble value);
jdouble nativeLongBitsToDouble(FNIEnv *env, jlong bits);

static constexpr NativeMethod doubleMethods[] = {
    NATIVE_METHOD("doubleToRawLongBits", "(D)J", nativeDoubleToRawLongBits),
    NATIVE_METHOD("longBitsToDouble",    "(J)D", nativeLongBitsToDouble),
};

#endif /* __FLINT_NATIVE_DOUBLE_H */
