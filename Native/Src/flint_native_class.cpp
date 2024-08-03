
#include "flint_native_math_class.h"
#include "flint_native_class_class.h"
#include "flint_native_float_class.h"
#include "flint_native_double_class.h"
#include "flint_native_object_class.h"
#include "flint_native_string_class.h"
#include "flint_native_system_class.h"
#include "flint_native_character_class.h"
#include "flint_native_print_stream_class.h"

const FlintNativeClass *BASE_NATIVE_CLASS_LIST[9] = {
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
