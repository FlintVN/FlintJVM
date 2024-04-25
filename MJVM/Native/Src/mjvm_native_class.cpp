
#include "mjvm_math_class.h"
#include "mjvm_system_class.h"
#include "mjvm_print_stream_class.h"

const NativeClass *NATIVE_CLASS_LIST[] = {
    &MATH_CLASS,
    &SYSTEM_CLASS,
    &PRINT_STREAM_CLASS,
};

const uint32_t NATIVE_CLASS_COUNT = LENGTH(NATIVE_CLASS_LIST);
