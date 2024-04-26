
#include <math.h>
#include "mjvm_const_name.h"
#include "mjvm_native_math_class.h"

static int64_t nativeSin(int32_t args[], int32_t argc) {
    double ret = sin(*(double *)args);
    return *(int64_t *)&ret;
}

static int64_t nativeCos(int32_t args[], int32_t argc) {
    double ret = cos(*(double *)args);
    return *(int64_t *)&ret;
}

static int64_t nativeTan(int32_t args[], int32_t argc) {
    double ret = tan(*(double *)args);
    return *(int64_t *)&ret;
}

static int64_t nativeAsin(int32_t args[], int32_t argc) {
    double ret = asin(*(double *)args);
    return *(int64_t *)&ret;
}

static int64_t nativeAcos(int32_t args[], int32_t argc) {
    double ret = acos(*(double *)args);
    return *(int64_t *)&ret;
}

static int64_t nativeAtan(int32_t args[], int32_t argc) {
    double ret = atan(*(double *)args);
    return *(int64_t *)&ret;
}

static int64_t nativeLog(int32_t args[], int32_t argc) {
    double ret = log(*(double *)args);
    return *(int64_t *)&ret;
}

static int64_t nativeLog10(int32_t args[], int32_t argc) {
    double ret = log10(*(double *)args);
    return *(int64_t *)&ret;
}

static int64_t nativeSqrt(int32_t args[], int32_t argc) {
    double ret = sqrt(*(double *)args);
    return *(int64_t *)&ret;
}

static int64_t nativeCbrt(int32_t args[], int32_t argc) {
    double ret = cbrt(*(double *)args);
    return *(int64_t *)&ret;
}

static int64_t nativeAtan2(int32_t args[], int32_t argc) {
    double ret = atan2(((double *)args)[0], ((double *)args)[1]);
    return *(int64_t *)&ret;
}

static int64_t nativePow(int32_t args[], int32_t argc) {
    double ret = pow(((double *)args)[0], ((double *)args)[1]);
    return *(int64_t *)&ret;
}

static int64_t nativeSinh(int32_t args[], int32_t argc) {
    double ret = sinh(*(double *)args);
    return *(int64_t *)&ret;
}

static int64_t nativeCosh(int32_t args[], int32_t argc) {
    double ret = cosh(*(double *)args);
    return *(int64_t *)&ret;
}

static int64_t nativeTanh(int32_t args[], int32_t argc) {
    double ret = tanh(*(double *)args);
    return *(int64_t *)&ret;
}

static const NativeMethod methods[] = {
    NATIVE_METHOD("\x03\x00""sin",           "\x04\x00""(D)D",  nativeSin),
    NATIVE_METHOD("\x03\x00""cos",           "\x04\x00""(D)D",  nativeCos),
    NATIVE_METHOD("\x03\x00""tan",           "\x04\x00""(D)D",  nativeTan),
    NATIVE_METHOD("\x04\x00""asin",          "\x04\x00""(D)D",  nativeAsin),
    NATIVE_METHOD("\x04\x00""acos",          "\x04\x00""(D)D",  nativeAcos),
    NATIVE_METHOD("\x04\x00""atan",          "\x04\x00""(D)D",  nativeAtan),
    NATIVE_METHOD("\x03\x00""log",           "\x04\x00""(D)D",  nativeLog),
    NATIVE_METHOD("\x05\x00""log10",         "\x04\x00""(D)D",  nativeLog10),
    NATIVE_METHOD("\x04\x00""sqrt",          "\x04\x00""(D)D",  nativeSqrt),
    NATIVE_METHOD("\x04\x00""cbrt",          "\x04\x00""(D)D",  nativeCbrt),
    NATIVE_METHOD("\x05\x00""atan2",         "\x05\x00""(DD)D", nativeAtan2),
    NATIVE_METHOD("\x03\x00""pow",           "\x05\x00""(DD)D", nativePow),
    NATIVE_METHOD("\x04\x00""sinh",          "\x04\x00""(D)D",  nativeSinh),
    NATIVE_METHOD("\x04\x00""cosh",          "\x04\x00""(D)D",  nativeCosh),
    NATIVE_METHOD("\x04\x00""tanh",          "\x04\x00""(D)D",  nativeTanh),
};

const NativeClass MATH_CLASS = NATIVE_CLASS(mathClass, methods);
