
#ifndef __FLINT_NATIVE_FLOAT_H
#define __FLINT_NATIVE_FLOAT_H

#include "flint_native.h"

jint nativeFloatToRawIntBits(FNIEnv *env, jfloat value);
jfloat nativeIntBitsToFloat(FNIEnv *env, jint bits);

static constexpr NativeMethod floatMethods[] = {
    NATIVE_METHOD("floatToRawIntBits", "(F)I", nativeFloatToRawIntBits),
    NATIVE_METHOD("intBitsToFloat",    "(I)F", nativeIntBitsToFloat),
};

#endif /* __FLINT_NATIVE_FLOAT_H */
