
#include <math.h>
#include "flint_const_name_base.h"
#include "flint_native_math_class.h"

static FlintError nativeSin(FlintExecution *exec) {
    exec->stackPushDouble(sin(exec->stackPopDouble()));
    return ERR_OK;
}

static FlintError nativeCos(FlintExecution *exec) {
    exec->stackPushDouble(cos(exec->stackPopDouble()));
    return ERR_OK;
}

static FlintError nativeTan(FlintExecution *exec) {
    exec->stackPushDouble(tan(exec->stackPopDouble()));
    return ERR_OK;
}

static FlintError nativeAsin(FlintExecution *exec) {
    exec->stackPushDouble(asin(exec->stackPopDouble()));
    return ERR_OK;
}

static FlintError nativeAcos(FlintExecution *exec) {
    exec->stackPushDouble(acos(exec->stackPopDouble()));
    return ERR_OK;
}

static FlintError nativeAtan(FlintExecution *exec) {
    exec->stackPushDouble(atan(exec->stackPopDouble()));
    return ERR_OK;
}

static FlintError nativeLog(FlintExecution *exec) {
    exec->stackPushDouble(log(exec->stackPopDouble()));
    return ERR_OK;
}

static FlintError nativeLog10(FlintExecution *exec) {
    exec->stackPushDouble(log10(exec->stackPopDouble()));
    return ERR_OK;
}

static FlintError nativeSqrt(FlintExecution *exec) {
    exec->stackPushDouble(sqrt(exec->stackPopDouble()));
    return ERR_OK;
}

static FlintError nativeCbrt(FlintExecution *exec) {
    exec->stackPushDouble(cbrt(exec->stackPopDouble()));
    return ERR_OK;
}

static FlintError nativeAtan2(FlintExecution *exec) {
    double x = exec->stackPopDouble();
    double y = exec->stackPopDouble();
    exec->stackPushDouble(atan2(y, x));
    return ERR_OK;
}

static FlintError nativePow(FlintExecution *exec) {
    double b = exec->stackPopDouble();
    double a = exec->stackPopDouble();
    exec->stackPushDouble(pow(a, b));
    return ERR_OK;
}

static FlintError nativeSinh(FlintExecution *exec) {
    exec->stackPushDouble(sinh(exec->stackPopDouble()));
    return ERR_OK;
}

static FlintError nativeCosh(FlintExecution *exec) {
    exec->stackPushDouble(cosh(exec->stackPopDouble()));
    return ERR_OK;
}

static FlintError nativeTanh(FlintExecution *exec) {
    exec->stackPushDouble(tanh(exec->stackPopDouble()));
    return ERR_OK;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x03\x00\xD0\x58""sin",   "\x04\x00\xA9\xCD""(D)D",  nativeSin),
    NATIVE_METHOD("\x03\x00\x12\x34""cos",   "\x04\x00\xA9\xCD""(D)D",  nativeCos),
    NATIVE_METHOD("\x03\x00\x66\x59""tan",   "\x04\x00\xA9\xCD""(D)D",  nativeTan),
    NATIVE_METHOD("\x04\x00\xBE\x40""asin",  "\x04\x00\xA9\xCD""(D)D",  nativeAsin),
    NATIVE_METHOD("\x04\x00\x7C\x2C""acos",  "\x04\x00\xA9\xCD""(D)D",  nativeAcos),
    NATIVE_METHOD("\x04\x00\x08\x41""atan",  "\x04\x00\xA9\xCD""(D)D",  nativeAtan),
    NATIVE_METHOD("\x03\x00\x22\x38""log",   "\x04\x00\xA9\xCD""(D)D",  nativeLog),
    NATIVE_METHOD("\x05\x00\xF2\xB9""log10", "\x04\x00\xA9\xCD""(D)D",  nativeLog10),
    NATIVE_METHOD("\x04\x00\x91\xC3""sqrt",  "\x04\x00\xA9\xCD""(D)D",  nativeSqrt),
    NATIVE_METHOD("\x04\x00\x64\xC6""cbrt",  "\x04\x00\xA9\xCD""(D)D",  nativeCbrt),
    NATIVE_METHOD("\x05\x00\x81\xAC""atan2", "\x05\x00\xF0\xBF""(DD)D", nativeAtan2),
    NATIVE_METHOD("\x03\x00\xE2\x32""pow",   "\x05\x00\xF0\xBF""(DD)D", nativePow),
    NATIVE_METHOD("\x04\x00\x18\xCD""sinh",  "\x04\x00\xA9\xCD""(D)D",  nativeSinh),
    NATIVE_METHOD("\x04\x00\xF5\x5C""cosh",  "\x04\x00\xA9\xCD""(D)D",  nativeCosh),
    NATIVE_METHOD("\x04\x00\x98\x7B""tanh",  "\x04\x00\xA9\xCD""(D)D",  nativeTanh),
};

const FlintNativeClass MATH_CLASS = NATIVE_CLASS(mathClassName, methods);
