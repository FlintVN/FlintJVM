
#include "mjvm_const_name.h"

const ConstUtf8 *primTypeConstUtf8List[] = {
    (ConstUtf8 *)"\x01\x00""Z",
    (ConstUtf8 *)"\x01\x00""C",
    (ConstUtf8 *)"\x01\x00""F",
    (ConstUtf8 *)"\x01\x00""D",
    (ConstUtf8 *)"\x01\x00""B",
    (ConstUtf8 *)"\x01\x00""S",
    (ConstUtf8 *)"\x01\x00""I",
    (ConstUtf8 *)"\x01\x00""J",
};

const uint32_t stringValueFieldName[] = {
    (uint32_t)"\x05\x00""value",                /* field name */
    (uint32_t)"\x02\x00""[B"                    /* field type */
};

const uint32_t exceptionDetailMessageFieldName[] = {
    (uint32_t)"\x0D\x00""detailMessage",        /* field name */
    (uint32_t)"\x12\x00""Ljava/lang/String;"    /* field type */
};

const ConstUtf8 &stringClassName = *(const ConstUtf8 *)"\x10\x00""java/lang/String";
const ConstUtf8 &arithmeticException = *(const ConstUtf8 *)"\x1D\x00""java/lang/ArithmeticException";
const ConstUtf8 &nullPtrExcpClassName = *(const ConstUtf8 *)"\x1E\x00""java/lang/NullPointerException";
const ConstUtf8 &arrayIndexOutOfBoundsExceptionClassName = *(const ConstUtf8 *)"\x28\x00""java/lang/ArrayIndexOutOfBoundsException";
