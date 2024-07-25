
#include "mjvm.h"
#include "mjvm_object.h"
#include "mjvm_const_name.h"
#include "mjvm_native_double_class.h"

static bool nativeDoubleToRawLongBits(MjvmExecution &execution) {
    double value = execution.stackPopDouble();
    execution.stackPushInt64(*(int64_t *)&value);
    return true;
}

static bool nativeLongBitsToDouble(MjvmExecution &execution) {
    int64_t bits = execution.stackPopInt64();
    execution.stackPushDouble(*(double *)&bits);
    return true;
}

static const NativeMethod methods[] = {
    NATIVE_METHOD("\x13\x00\x8A\x07""doubleToRawLongBits", "\x04\x00\xDF\x00""(D)J", nativeDoubleToRawLongBits),
    NATIVE_METHOD("\x10\x00\x60\x06""longBitsToDouble",    "\x04\x00\xDF\x00""(J)D", nativeLongBitsToDouble),
};

const NativeClass DOUBLE_CLASS = NATIVE_CLASS(doubleClassName, methods);
