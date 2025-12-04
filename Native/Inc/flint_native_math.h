
#ifndef __FLINT_NATIVE_MATH_H
#define __FLINT_NATIVE_MATH_H

#include "flint_native.h"

jdouble NativeMath_Sin(FNIEnv *env, jdouble a);
jdouble NativeMath_Cos(FNIEnv *env, jdouble a);
jdouble NativeMath_Tan(FNIEnv *env, jdouble a);
jdouble NativeMath_Asin(FNIEnv *env, jdouble a);
jdouble NativeMath_Acos(FNIEnv *env, jdouble a);
jdouble NativeMath_Atan(FNIEnv *env, jdouble a);
jdouble NativeMath_Log(FNIEnv *env, jdouble a);
jdouble NativeMath_Log10(FNIEnv *env, jdouble a);
jdouble NativeMath_Sqrt(FNIEnv *env, jdouble a);
jdouble NativeMath_Cbrt(FNIEnv *env, jdouble a);
jdouble NativeMath_Atan2(FNIEnv *env, jdouble y, jdouble x);
jdouble NativeMath_Pow(FNIEnv *env, jdouble a, jdouble b);
jdouble NativeMath_Sinh(FNIEnv *env, jdouble x);
jdouble NativeMath_Cosh(FNIEnv *env, jdouble x);
jdouble NativeMath_Tanh(FNIEnv *env, jdouble x);

static constexpr NativeMethod mathMethods[] = {
    NATIVE_METHOD("sin",   "(D)D",  NativeMath_Sin),
    NATIVE_METHOD("cos",   "(D)D",  NativeMath_Cos),
    NATIVE_METHOD("tan",   "(D)D",  NativeMath_Tan),
    NATIVE_METHOD("asin",  "(D)D",  NativeMath_Asin),
    NATIVE_METHOD("acos",  "(D)D",  NativeMath_Acos),
    NATIVE_METHOD("atan",  "(D)D",  NativeMath_Atan),
    NATIVE_METHOD("log",   "(D)D",  NativeMath_Log),
    NATIVE_METHOD("log10", "(D)D",  NativeMath_Log10),
    NATIVE_METHOD("sqrt",  "(D)D",  NativeMath_Sqrt),
    NATIVE_METHOD("cbrt",  "(D)D",  NativeMath_Cbrt),
    NATIVE_METHOD("atan2", "(DD)D", NativeMath_Atan2),
    NATIVE_METHOD("pow",   "(DD)D", NativeMath_Pow),
    NATIVE_METHOD("sinh",  "(D)D",  NativeMath_Sinh),
    NATIVE_METHOD("cosh",  "(D)D",  NativeMath_Cosh),
    NATIVE_METHOD("tanh",  "(D)D",  NativeMath_Tanh),
};

#endif /* __FLINT_NATIVE_MATH_H */
