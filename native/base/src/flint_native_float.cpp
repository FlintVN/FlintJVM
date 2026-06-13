
#include "flint_native_float.h"

jint NativeFloat_FloatToRawIntBits(FNIEnv *env, jfloat value) {
    (void)env;
    return *(jint *)&value;
}

jfloat NativeFloat_IntBitsToFloat(FNIEnv *env, jint bits) {
    (void)env;
    return *(jfloat *)&bits;
}
