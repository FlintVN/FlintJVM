
#include "flint.h"
#include "flint_object.h"
#include "flint_const_name.h"
#include "flint_native_float_class.h"

static bool nativeFloatToRawIntBits(FlintExecution &execution) {
    float value = execution.stackPopFloat();
    execution.stackPushInt32(*(int32_t *)&value);
    return true;
}

static bool nativeIntBitsToFloat(FlintExecution &execution) {
    int32_t bits = execution.stackPopInt32();
    execution.stackPushFloat(*(float *)&bits);
    return true;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x11\x00\xC0\x06""floatToRawIntBits", "\x04\x00\xE0\x00""(F)I", nativeFloatToRawIntBits),
    NATIVE_METHOD("\x0E\x00\x96\x05""intBitsToFloat",    "\x04\x00\xE0\x00""(I)F", nativeIntBitsToFloat),
};

const FlintNativeClass FLOAT_CLASS = NATIVE_CLASS(floatClassName, methods);
