
#include "flint.h"
#include "flint_java_object.h"
#include "flint_const_name_base.h"
#include "flint_native_double_class.h"

static FlintError nativeDoubleToRawLongBits(FlintExecution *exec) {
    double value = exec->stackPopDouble();
    exec->stackPushInt64(*(int64_t *)&value);
    return ERR_OK;
}

static FlintError nativeLongBitsToDouble(FlintExecution *exec) {
    int64_t bits = exec->stackPopInt64();
    exec->stackPushDouble(*(double *)&bits);
    return ERR_OK;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x13\x00\x4A\xB5""doubleToRawLongBits", "\x04\x00\x28\x09""(D)J", nativeDoubleToRawLongBits),
    NATIVE_METHOD("\x10\x00\xAA\x93""longBitsToDouble",    "\x04\x00\xC8\x0E""(J)D", nativeLongBitsToDouble),
};

const FlintNativeClass DOUBLE_CLASS = NATIVE_CLASS(doubleClassName, methods);
