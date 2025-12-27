
#ifndef __FLINT_NATIVE_DOUBLE_H
#define __FLINT_NATIVE_DOUBLE_H

#include "flint_native.h"

jlong NativeDouble_DoubleToRawLongBits(FNIEnv *env, jdouble value);
jdouble NativeDouble_LongBitsToDouble(FNIEnv *env, jlong bits);

inline constexpr NativeMethod doubleMethods[] = {
    NATIVE_METHOD("doubleToRawLongBits", "(D)J", NativeDouble_DoubleToRawLongBits),
    NATIVE_METHOD("longBitsToDouble",    "(J)D", NativeDouble_LongBitsToDouble),
};

#endif /* __FLINT_NATIVE_DOUBLE_H */
