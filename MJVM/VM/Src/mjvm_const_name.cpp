
#include "mjvm_const_name.h"

const ConstUtf8 * const primTypeConstUtf8List[] = {
    (ConstUtf8 *)"\x01\x00\x5A\x00""Z",                 /* boolean */
    (ConstUtf8 *)"\x01\x00\x43\x00""C",                 /* char */
    (ConstUtf8 *)"\x01\x00\x46\x00""F",                 /* float */
    (ConstUtf8 *)"\x01\x00\x44\x00""D",                 /* double */
    (ConstUtf8 *)"\x01\x00\x42\x00""B",                 /* byte */
    (ConstUtf8 *)"\x01\x00\x53\x00""S",                 /* short */
    (ConstUtf8 *)"\x01\x00\x49\x00""I",                 /* integer */
    (ConstUtf8 *)"\x01\x00\x4A\x00""J",                 /* long */
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

const ConstUtf8 &mathClassName = *(const ConstUtf8 *)"\x0E\x00\x2C\x05""java/lang/Math";
const ConstUtf8 &classClassName = *(const ConstUtf8 *)"\x0F\x00\x98\x05""java/lang/Class";
const ConstUtf8 &floatClassName = *(const ConstUtf8 *)"\x0F\x00\x98\x05""java/lang/Float";
const ConstUtf8 &doubleClassName = *(const ConstUtf8 *)"\x10\x00\xFD\x05""java/lang/Double";
const ConstUtf8 &objectClassName = *(const ConstUtf8 *)"\x10\x00\xF9\x05""java/lang/Object";
const ConstUtf8 &systemClassName = *(const ConstUtf8 *)"\x10\x00\x27\x06""java/lang/System";
const ConstUtf8 &stringClassName = *(const ConstUtf8 *)"\x10\x00\x19\x06""java/lang/String";
const ConstUtf8 &characterClassName = *(const ConstUtf8 *)"\x13\x00\x2F\x07""java/lang/Character";
const ConstUtf8 &printStreamClassName = *(const ConstUtf8 *)"\x13\x00\x51\x07""java/io/PrintStream";
const ConstUtf8 &nullPtrExcpClassName = *(const ConstUtf8 *)"\x1E\x00\xCD\x0B""java/lang/NullPointerException";
const ConstUtf8 &arrayStoreExceptionClassName = *(const ConstUtf8 *)"\x1D\x00\x5D\x0B""java/lang/ArrayStoreException";
const ConstUtf8 &arithmeticExceptionClassName = *(const ConstUtf8 *)"\x1D\x00\x5B\x0B""java/lang/ArithmeticException";
const ConstUtf8 &classNotFoundExceptionClassName = *(const ConstUtf8 *)"\x20\x00\x74\x0C""java/lang/ClassNotFoundException";
const ConstUtf8 &cloneNotSupportedExceptionClassName = *(const ConstUtf8 *)"\x24\x00\x39\x0E""java/lang/CloneNotSupportedException";
const ConstUtf8 &negativeArraySizeExceptionClassName = *(const ConstUtf8 *)"\x24\x00\x1E\x0E""java/lang/NegativeArraySizeException";
const ConstUtf8 &arrayIndexOutOfBoundsExceptionClassName = *(const ConstUtf8 *)"\x28\x00\xA0\x0F""java/lang/ArrayIndexOutOfBoundsException";
