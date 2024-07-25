
#include <math.h>
#include "mjvm_const_name.h"
#include "mjvm_native_math_class.h"

static bool nativeSin(MjvmExecution &execution) {
    execution.stackPushDouble(sin(execution.stackPopDouble()));
    return true;
}

static bool nativeCos(MjvmExecution &execution) {
    execution.stackPushDouble(cos(execution.stackPopDouble()));
    return true;
}

static bool nativeTan(MjvmExecution &execution) {
    execution.stackPushDouble(tan(execution.stackPopDouble()));
    return true;
}

static bool nativeAsin(MjvmExecution &execution) {
    execution.stackPushDouble(asin(execution.stackPopDouble()));
    return true;
}

static bool nativeAcos(MjvmExecution &execution) {
    execution.stackPushDouble(acos(execution.stackPopDouble()));
    return true;
}

static bool nativeAtan(MjvmExecution &execution) {
    execution.stackPushDouble(atan(execution.stackPopDouble()));
    return true;
}

static bool nativeLog(MjvmExecution &execution) {
    execution.stackPushDouble(log(execution.stackPopDouble()));
    return true;
}

static bool nativeLog10(MjvmExecution &execution) {
    execution.stackPushDouble(log10(execution.stackPopDouble()));
    return true;
}

static bool nativeSqrt(MjvmExecution &execution) {
    execution.stackPushDouble(sqrt(execution.stackPopDouble()));
    return true;
}

static bool nativeCbrt(MjvmExecution &execution) {
    execution.stackPushDouble(cbrt(execution.stackPopDouble()));
    return true;
}

static bool nativeAtan2(MjvmExecution &execution) {
    double x = execution.stackPopDouble();
    double y = execution.stackPopDouble();
    execution.stackPushDouble(atan2(y, x));
    return true;
}

static bool nativePow(MjvmExecution &execution) {
    double b = execution.stackPopDouble();
    double a = execution.stackPopDouble();
    execution.stackPushDouble(pow(a, b));
    return true;
}

static bool nativeSinh(MjvmExecution &execution) {
    execution.stackPushDouble(sinh(execution.stackPopDouble()));
    return true;
}

static bool nativeCosh(MjvmExecution &execution) {
    execution.stackPushDouble(cosh(execution.stackPopDouble()));
    return true;
}

static bool nativeTanh(MjvmExecution &execution) {
    execution.stackPushDouble(tanh(execution.stackPopDouble()));
    return true;
}

static const NativeMethod methods[] = {
    NATIVE_METHOD("\x03\x00\x4A\x01""sin",   "\x04\x00\xD9\x00""(D)D",  nativeSin),
    NATIVE_METHOD("\x03\x00\x45\x01""cos",   "\x04\x00\xD9\x00""(D)D",  nativeCos),
    NATIVE_METHOD("\x03\x00\x43\x01""tan",   "\x04\x00\xD9\x00""(D)D",  nativeTan),
    NATIVE_METHOD("\x04\x00\xAB\x01""asin",  "\x04\x00\xD9\x00""(D)D",  nativeAsin),
    NATIVE_METHOD("\x04\x00\xA6\x01""acos",  "\x04\x00\xD9\x00""(D)D",  nativeAcos),
    NATIVE_METHOD("\x04\x00\xA4\x01""atan",  "\x04\x00\xD9\x00""(D)D",  nativeAtan),
    NATIVE_METHOD("\x03\x00\x42\x01""log",   "\x04\x00\xD9\x00""(D)D",  nativeLog),
    NATIVE_METHOD("\x05\x00\xA3\x01""log10", "\x04\x00\xD9\x00""(D)D",  nativeLog10),
    NATIVE_METHOD("\x04\x00\xCA\x01""sqrt",  "\x04\x00\xD9\x00""(D)D",  nativeSqrt),
    NATIVE_METHOD("\x04\x00\xAB\x01""cbrt",  "\x04\x00\xD9\x00""(D)D",  nativeCbrt),
    NATIVE_METHOD("\x05\x00\xD6\x01""atan2", "\x05\x00\x1D\x01""(DD)D", nativeAtan2),
    NATIVE_METHOD("\x03\x00\x56\x01""pow",   "\x05\x00\x1D\x01""(DD)D", nativePow),
    NATIVE_METHOD("\x04\x00\xB2\x01""sinh",  "\x04\x00\xD9\x00""(D)D",  nativeSinh),
    NATIVE_METHOD("\x04\x00\xAD\x01""cosh",  "\x04\x00\xD9\x00""(D)D",  nativeCosh),
    NATIVE_METHOD("\x04\x00\xAB\x01""tanh",  "\x04\x00\xD9\x00""(D)D",  nativeTanh),
};

const NativeClass MATH_CLASS = NATIVE_CLASS(mathClassName, methods);
