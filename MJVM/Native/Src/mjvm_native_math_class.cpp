
#include <math.h>
#include "mjvm_const_name.h"
#include "mjvm_native_math_class.h"

static bool nativeSin(Execution &execution) {
    execution.stackPushDouble(sin(execution.stackPopDouble()));
    return true;
}

static bool nativeCos(Execution &execution) {
    execution.stackPushDouble(cos(execution.stackPopDouble()));
    return true;
}

static bool nativeTan(Execution &execution) {
    execution.stackPushDouble(tan(execution.stackPopDouble()));
    return true;
}

static bool nativeAsin(Execution &execution) {
    execution.stackPushDouble(asin(execution.stackPopDouble()));
    return true;
}

static bool nativeAcos(Execution &execution) {
    execution.stackPushDouble(acos(execution.stackPopDouble()));
    return true;
}

static bool nativeAtan(Execution &execution) {
    execution.stackPushDouble(atan(execution.stackPopDouble()));
    return true;
}

static bool nativeLog(Execution &execution) {
    execution.stackPushDouble(log(execution.stackPopDouble()));
    return true;
}

static bool nativeLog10(Execution &execution) {
    execution.stackPushDouble(log10(execution.stackPopDouble()));
    return true;
}

static bool nativeSqrt(Execution &execution) {
    execution.stackPushDouble(sqrt(execution.stackPopDouble()));
    return true;
}

static bool nativeCbrt(Execution &execution) {
    execution.stackPushDouble(cbrt(execution.stackPopDouble()));
    return true;
}

static bool nativeAtan2(Execution &execution) {
    double x = execution.stackPopDouble();
    double y = execution.stackPopDouble();
    execution.stackPushDouble(atan2(y, x));
    return true;
}

static bool nativePow(Execution &execution) {
    double b = execution.stackPopDouble();
    double a = execution.stackPopDouble();
    execution.stackPushDouble(pow(a, b));
    return true;
}

static bool nativeSinh(Execution &execution) {
    execution.stackPushDouble(sinh(execution.stackPopDouble()));
    return true;
}

static bool nativeCosh(Execution &execution) {
    execution.stackPushDouble(cosh(execution.stackPopDouble()));
    return true;
}

static bool nativeTanh(Execution &execution) {
    execution.stackPushDouble(tanh(execution.stackPopDouble()));
    return true;
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

const NativeClass MATH_CLASS = NATIVE_CLASS(mathClassName, methods);
