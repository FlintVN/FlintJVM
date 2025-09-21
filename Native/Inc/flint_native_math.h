
#ifndef __FLINT_NATIVE_MATH_H
#define __FLINT_NATIVE_MATH_H

#include "flint_native.h"

jdouble nativeSin(FNIEnv *env, jdouble a);
jdouble nativeCos(FNIEnv *env, jdouble a);
jdouble nativeTan(FNIEnv *env, jdouble a);
jdouble nativeAsin(FNIEnv *env, jdouble a);
jdouble nativeAcos(FNIEnv *env, jdouble a);
jdouble nativeAtan(FNIEnv *env, jdouble a);
jdouble nativeLog(FNIEnv *env, jdouble a);
jdouble nativeLog10(FNIEnv *env, jdouble a);
jdouble nativeSqrt(FNIEnv *env, jdouble a);
jdouble nativeCbrt(FNIEnv *env, jdouble a);
jdouble nativeAtan2(FNIEnv *env, jdouble y, jdouble x);
jdouble nativePow(FNIEnv *env, jdouble a, jdouble b);
jdouble nativeSinh(FNIEnv *env, jdouble x);
jdouble nativeCosh(FNIEnv *env, jdouble x);
jdouble nativeTanh(FNIEnv *env, jdouble x);

static constexpr NativeMethod mathMethods[] = {
    NATIVE_METHOD("sin",   "(D)D",  nativeSin),
    NATIVE_METHOD("cos",   "(D)D",  nativeCos),
    NATIVE_METHOD("tan",   "(D)D",  nativeTan),
    NATIVE_METHOD("asin",  "(D)D",  nativeAsin),
    NATIVE_METHOD("acos",  "(D)D",  nativeAcos),
    NATIVE_METHOD("atan",  "(D)D",  nativeAtan),
    NATIVE_METHOD("log",   "(D)D",  nativeLog),
    NATIVE_METHOD("log10", "(D)D",  nativeLog10),
    NATIVE_METHOD("sqrt",  "(D)D",  nativeSqrt),
    NATIVE_METHOD("cbrt",  "(D)D",  nativeCbrt),
    NATIVE_METHOD("atan2", "(DD)D", nativeAtan2),
    NATIVE_METHOD("pow",   "(DD)D", nativePow),
    NATIVE_METHOD("sinh",  "(D)D",  nativeSinh),
    NATIVE_METHOD("cosh",  "(D)D",  nativeCosh),
    NATIVE_METHOD("tanh",  "(D)D",  nativeTanh),
};

#endif /* __FLINT_NATIVE_MATH_H */
