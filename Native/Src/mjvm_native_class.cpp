
#include "mjvm_native_math_class.h"
#include "mjvm_native_class_class.h"
#include "mjvm_native_float_class.h"
#include "mjvm_native_double_class.h"
#include "mjvm_native_object_class.h"
#include "mjvm_native_string_class.h"
#include "mjvm_native_system_class.h"
#include "mjvm_native_character_class.h"
#include "mjvm_native_print_stream_class.h"

const NativeClass *NATIVE_CLASS_LIST[] = {
    &MATH_CLASS,
    &CLASS_CLASS,
    &FLOAT_CLASS,
    &DOUBLE_CLASS,
    &OBJECT_CLASS,
    &STRING_CLASS,
    &SYSTEM_CLASS,
    &CHARACTER_CLASS,
    &PRINT_STREAM_CLASS,
};

const uint32_t NATIVE_CLASS_COUNT = LENGTH(NATIVE_CLASS_LIST);
