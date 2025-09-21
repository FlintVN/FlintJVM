
#include "flint_native_double.h"

jlong nativeDoubleToRawLongBits(FNIEnv *env, jdouble value) {
    (void)env;
    return *(jlong *)&value;
}

jdouble nativeLongBitsToDouble(FNIEnv *env, jlong bits) {
    (void)env;
    return *(jdouble *)&bits;
}
