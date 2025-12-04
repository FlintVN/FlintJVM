
#include "flint_native_double.h"

jlong NativeDouble_DoubleToRawLongBits(FNIEnv *env, jdouble value) {
    (void)env;
    return *(jlong *)&value;
}

jdouble NativeDouble_LongBitsToDouble(FNIEnv *env, jlong bits) {
    (void)env;
    return *(jdouble *)&bits;
}
