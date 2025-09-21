
#include "flint_native_float.h"

jint nativeFloatToRawIntBits(FNIEnv *env, jfloat value) {
    (void)env;
    return *(jint *)&value;
}

jfloat nativeIntBitsToFloat(FNIEnv *env, jint bits) {
    (void)env;
    return *(jfloat *)&bits;
}
