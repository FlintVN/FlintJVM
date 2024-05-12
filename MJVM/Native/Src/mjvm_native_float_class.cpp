
#include "mjvm.h"
#include "mjvm_object.h"
#include "mjvm_const_name.h"
#include "mjvm_native_float_class.h"

static bool nativeFloatToRawIntBits(Execution &execution) {
    float value = execution.stackPopFloat();
    execution.stackPushInt32(*(int32_t *)&value);
    return true;
}

static bool nativeIntBitsToFloat(Execution &execution) {
    int32_t bits = execution.stackPopInt32();
    execution.stackPushFloat(*(float *)&bits);
    return true;
}

static const NativeMethod methods[] = {
    NATIVE_METHOD("\x11\x00""floatToRawIntBits", "\x04\x00""(F)I", nativeFloatToRawIntBits),
    NATIVE_METHOD("\x0E\x00""intBitsToFloat",    "\x04\x00""(I)F", nativeIntBitsToFloat),
};

const NativeClass FLOAT_CLASS = NATIVE_CLASS(floatClassName, methods);
