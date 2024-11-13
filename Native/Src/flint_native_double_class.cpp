
#include "flint.h"
#include "flint_object.h"
#include "flint_const_name.h"
#include "flint_native_double_class.h"

static void nativeDoubleToRawLongBits(FlintExecution &execution) {
    double value = execution.stackPopDouble();
    execution.stackPushInt64(*(int64_t *)&value);
}

static void nativeLongBitsToDouble(FlintExecution &execution) {
    int64_t bits = execution.stackPopInt64();
    execution.stackPushDouble(*(double *)&bits);
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x13\x00\x4A\xB5""doubleToRawLongBits", "\x04\x00\x28\x09""(D)J", nativeDoubleToRawLongBits),
    NATIVE_METHOD("\x10\x00\xAA\x93""longBitsToDouble",    "\x04\x00\xC8\x0E""(J)D", nativeLongBitsToDouble),
};

const FlintNativeClass DOUBLE_CLASS = NATIVE_CLASS(doubleClassName, methods);
