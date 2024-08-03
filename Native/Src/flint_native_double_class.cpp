
#include "flint.h"
#include "flint_object.h"
#include "flint_const_name.h"
#include "flint_native_double_class.h"

static bool nativeDoubleToRawLongBits(FlintExecution &execution) {
    double value = execution.stackPopDouble();
    execution.stackPushInt64(*(int64_t *)&value);
    return true;
}

static bool nativeLongBitsToDouble(FlintExecution &execution) {
    int64_t bits = execution.stackPopInt64();
    execution.stackPushDouble(*(double *)&bits);
    return true;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x13\x00\x8A\x07""doubleToRawLongBits", "\x04\x00\xDF\x00""(D)J", nativeDoubleToRawLongBits),
    NATIVE_METHOD("\x10\x00\x60\x06""longBitsToDouble",    "\x04\x00\xDF\x00""(J)D", nativeLongBitsToDouble),
};

const FlintNativeClass DOUBLE_CLASS = NATIVE_CLASS(doubleClassName, methods);
