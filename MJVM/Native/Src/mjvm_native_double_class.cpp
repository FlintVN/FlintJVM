
#include "mjvm.h"
#include "mjvm_object.h"
#include "mjvm_const_name.h"
#include "mjvm_native_double_class.h"

static bool nativeDoubleToRawLongBits(Execution &execution) {
    double value = execution.stackPopDouble();
    execution.stackPushInt64(*(int64_t *)&value);
    return true;
}

static bool nativeLongBitsToDouble(Execution &execution) {
    int64_t bits = execution.stackPopInt64();
    execution.stackPushDouble(*(double *)&bits);
    return true;
}

static const NativeMethod methods[] = {
    NATIVE_METHOD("\x13\x00""doubleToRawLongBits", "\x04\x00""(D)J", nativeDoubleToRawLongBits),
    NATIVE_METHOD("\x10\x00""longBitsToDouble",    "\x04\x00""(J)D", nativeLongBitsToDouble),
};

const NativeClass DOUBLE_CLASS = NATIVE_CLASS(doubleClass, methods);
