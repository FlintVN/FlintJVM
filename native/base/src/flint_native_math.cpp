
#include <math.h>
#include "flint_native_math.h"

jdouble NativeMath_Sin(FNIEnv *env, jdouble a) {
    (void)env;
    return sin(a);
}

jdouble NativeMath_Cos(FNIEnv *env, jdouble a) {
    (void)env;
    return cos(a);
}

jdouble NativeMath_Tan(FNIEnv *env, jdouble a) {
    (void)env;
    return tan(a);
}

jdouble NativeMath_Asin(FNIEnv *env, jdouble a) {
    (void)env;
    return asin(a);
}

jdouble NativeMath_Acos(FNIEnv *env, jdouble a) {
    (void)env;
    return acos(a);
}

jdouble NativeMath_Atan(FNIEnv *env, jdouble a) {
    (void)env;
    return atan(a);
}

jdouble NativeMath_Log(FNIEnv *env, jdouble a) {
    (void)env;
    return log(a);
}

jdouble NativeMath_Log10(FNIEnv *env, jdouble a) {
    (void)env;
    return log10(a);
}

jdouble NativeMath_Sqrt(FNIEnv *env, jdouble a) {
    (void)env;
    return sqrt(a);
}

jdouble NativeMath_Cbrt(FNIEnv *env, jdouble a) {
    (void)env;
    return cbrt(a);
}

jdouble NativeMath_Atan2(FNIEnv *env, jdouble y, jdouble x) {
    (void)env;
    return atan2(y, x);
}

jdouble NativeMath_Pow(FNIEnv *env, jdouble a, jdouble b) {
    (void)env;
    return pow(a, b);
}

jdouble NativeMath_Sinh(FNIEnv *env, jdouble x) {
    (void)env;
    return sinh(x);
}

jdouble NativeMath_Cosh(FNIEnv *env, jdouble x) {
    (void)env;
    return cosh(x);
}

jdouble NativeMath_Tanh(FNIEnv *env, jdouble x) {
    (void)env;
    return tanh(x);
}
