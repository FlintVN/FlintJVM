
#include "mjvm_const_name.h"

const MjvmConstUtf8 * const primTypeConstUtf8List[] = {
    (MjvmConstUtf8 *)"\x01\x00\x5A\x00""Z",                 /* boolean */
    (MjvmConstUtf8 *)"\x01\x00\x43\x00""C",                 /* char */
    (MjvmConstUtf8 *)"\x01\x00\x46\x00""F",                 /* float */
    (MjvmConstUtf8 *)"\x01\x00\x44\x00""D",                 /* double */
    (MjvmConstUtf8 *)"\x01\x00\x42\x00""B",                 /* byte */
    (MjvmConstUtf8 *)"\x01\x00\x53\x00""S",                 /* short */
    (MjvmConstUtf8 *)"\x01\x00\x49\x00""I",                 /* integer */
    (MjvmConstUtf8 *)"\x01\x00\x4A\x00""J",                 /* long */
};

const uint32_t stringNameFieldName[] = {
    (uint32_t)"\x04\x00\xA1\x01""name",                 /* field name */
    (uint32_t)"\x12\x00\xA0\x06""Ljava/lang/String;"    /* field type */
};

const uint32_t stringValueFieldName[] = {
    (uint32_t)"\x05\x00\x1D\x02""value",                /* field name */
    (uint32_t)"\x02\x00\x9D\x00""[B"                    /* field type */
};

const uint32_t stringCoderFieldName[] = {
    (uint32_t)"\x05\x00\x0D\x02""coder",                /* field name */
    (uint32_t)"\x01\x00\x42\x00""B"                     /* field type */
};

const uint32_t exceptionDetailMessageFieldName[] = {
    (uint32_t)"\x0D\x00\x38\x05""detailMessage",        /* field name */
    (uint32_t)"\x12\x00\xA0\x06""Ljava/lang/String;"    /* field type */
};

const MjvmConstUtf8 &mathClassName = *(const MjvmConstUtf8 *)"\x0E\x00\x2C\x05""java/lang/Math";
const MjvmConstUtf8 &classClassName = *(const MjvmConstUtf8 *)"\x0F\x00\x98\x05""java/lang/Class";
const MjvmConstUtf8 &floatClassName = *(const MjvmConstUtf8 *)"\x0F\x00\x98\x05""java/lang/Float";
const MjvmConstUtf8 &doubleClassName = *(const MjvmConstUtf8 *)"\x10\x00\xFD\x05""java/lang/Double";
const MjvmConstUtf8 &objectClassName = *(const MjvmConstUtf8 *)"\x10\x00\xF9\x05""java/lang/Object";
const MjvmConstUtf8 &systemClassName = *(const MjvmConstUtf8 *)"\x10\x00\x27\x06""java/lang/System";
const MjvmConstUtf8 &stringClassName = *(const MjvmConstUtf8 *)"\x10\x00\x19\x06""java/lang/String";
const MjvmConstUtf8 &characterClassName = *(const MjvmConstUtf8 *)"\x13\x00\x2F\x07""java/lang/Character";
const MjvmConstUtf8 &printStreamClassName = *(const MjvmConstUtf8 *)"\x13\x00\x51\x07""java/io/PrintStream";
const MjvmConstUtf8 &nullPtrExcpClassName = *(const MjvmConstUtf8 *)"\x1E\x00\xCD\x0B""java/lang/NullPointerException";
const MjvmConstUtf8 &arrayStoreExceptionClassName = *(const MjvmConstUtf8 *)"\x1D\x00\x5D\x0B""java/lang/ArrayStoreException";
const MjvmConstUtf8 &arithmeticExceptionClassName = *(const MjvmConstUtf8 *)"\x1D\x00\x5B\x0B""java/lang/ArithmeticException";
const MjvmConstUtf8 &classNotFoundExceptionClassName = *(const MjvmConstUtf8 *)"\x20\x00\x74\x0C""java/lang/ClassNotFoundException";
const MjvmConstUtf8 &cloneNotSupportedExceptionClassName = *(const MjvmConstUtf8 *)"\x24\x00\x39\x0E""java/lang/CloneNotSupportedException";
const MjvmConstUtf8 &negativeArraySizeExceptionClassName = *(const MjvmConstUtf8 *)"\x24\x00\x1E\x0E""java/lang/NegativeArraySizeException";
const MjvmConstUtf8 &arrayIndexOutOfBoundsExceptionClassName = *(const MjvmConstUtf8 *)"\x28\x00\xA0\x0F""java/lang/ArrayIndexOutOfBoundsException";
