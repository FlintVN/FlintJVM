
#include "mjvm_native_math_class.h"
#include "mjvm_native_system_class.h"
#include "mjvm_native_print_stream_class.h"

const NativeClass *NATIVE_CLASS_LIST[] = {
    &MATH_CLASS,
    &SYSTEM_CLASS,
    &PRINT_STREAM_CLASS,
};

const uint32_t NATIVE_CLASS_COUNT = LENGTH(NATIVE_CLASS_LIST);
