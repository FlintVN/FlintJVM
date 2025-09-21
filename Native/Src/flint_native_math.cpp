
#include <math.h>
#include "flint_native_math.h"

jdouble nativeSin(FNIEnv *env, jdouble a) {
    (void)env;
    return sin(a);
}

jdouble nativeCos(FNIEnv *env, jdouble a) {
    (void)env;
    return cos(a);
}

jdouble nativeTan(FNIEnv *env, jdouble a) {
    (void)env;
    return tan(a);
}

jdouble nativeAsin(FNIEnv *env, jdouble a) {
    (void)env;
    return asin(a);
}

jdouble nativeAcos(FNIEnv *env, jdouble a) {
    (void)env;
    return acos(a);
}

jdouble nativeAtan(FNIEnv *env, jdouble a) {
    (void)env;
    return atan(a);
}

jdouble nativeLog(FNIEnv *env, jdouble a) {
    (void)env;
    return log(a);
}

jdouble nativeLog10(FNIEnv *env, jdouble a) {
    (void)env;
    return log10(a);
}

jdouble nativeSqrt(FNIEnv *env, jdouble a) {
    (void)env;
    return sqrt(a);
}

jdouble nativeCbrt(FNIEnv *env, jdouble a) {
    (void)env;
    return cbrt(a);
}

jdouble nativeAtan2(FNIEnv *env, jdouble y, jdouble x) {
    (void)env;
    return atan2(y, x);
}

jdouble nativePow(FNIEnv *env, jdouble a, jdouble b) {
    (void)env;
    return pow(a, b);
}

jdouble nativeSinh(FNIEnv *env, jdouble x) {
    (void)env;
    return sinh(x);
}

jdouble nativeCosh(FNIEnv *env, jdouble x) {
    (void)env;
    return cosh(x);
}

jdouble nativeTanh(FNIEnv *env, jdouble x) {
    (void)env;
    return tanh(x);
}
