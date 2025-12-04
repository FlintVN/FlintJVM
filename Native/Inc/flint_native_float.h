
#ifndef __FLINT_NATIVE_FLOAT_H
#define __FLINT_NATIVE_FLOAT_H

#include "flint_native.h"

jint NativeFloat_FloatToRawIntBits(FNIEnv *env, jfloat value);
jfloat NativeFloat_IntBitsToFloat(FNIEnv *env, jint bits);

static constexpr NativeMethod floatMethods[] = {
    NATIVE_METHOD("floatToRawIntBits", "(F)I", NativeFloat_FloatToRawIntBits),
    NATIVE_METHOD("intBitsToFloat",    "(I)F", NativeFloat_IntBitsToFloat),
};

#endif /* __FLINT_NATIVE_FLOAT_H */
