
#include "flint_const_name.h"

const FlintConstUtf8 * const primTypeConstUtf8List[] = {
    (FlintConstUtf8 *)"\x01\x00\x5A\x00""Z",                 /* boolean */
    (FlintConstUtf8 *)"\x01\x00\x43\x00""C",                 /* char */
    (FlintConstUtf8 *)"\x01\x00\x46\x00""F",                 /* float */
    (FlintConstUtf8 *)"\x01\x00\x44\x00""D",                 /* double */
    (FlintConstUtf8 *)"\x01\x00\x42\x00""B",                 /* byte */
    (FlintConstUtf8 *)"\x01\x00\x53\x00""S",                 /* short */
    (FlintConstUtf8 *)"\x01\x00\x49\x00""I",                 /* integer */
    (FlintConstUtf8 *)"\x01\x00\x4A\x00""J",                 /* long */
};

const uint32_t stringNameFieldName[] = {
    (uint32_t)"\x04\x00\xA1\x01""name",                 /* field name */
    (uint32_t)"\x12\x00\xA0\x06""Ljava/lang/String;"    /* field type */
};

const uint32_t exceptionDetailMessageFieldName[] = {
    (uint32_t)"\x0D\x00\x38\x05""detailMessage",        /* field name */
    (uint32_t)"\x12\x00\xA0\x06""Ljava/lang/String;"    /* field type */
};

const FlintConstUtf8 &mathClassName = *(const FlintConstUtf8 *)"\x0E\x00\x2C\x05""java/lang/Math";
const FlintConstUtf8 &classClassName = *(const FlintConstUtf8 *)"\x0F\x00\x98\x05""java/lang/Class";
const FlintConstUtf8 &floatClassName = *(const FlintConstUtf8 *)"\x0F\x00\x98\x05""java/lang/Float";
const FlintConstUtf8 &doubleClassName = *(const FlintConstUtf8 *)"\x10\x00\xFD\x05""java/lang/Double";
const FlintConstUtf8 &objectClassName = *(const FlintConstUtf8 *)"\x10\x00\xF9\x05""java/lang/Object";
const FlintConstUtf8 &systemClassName = *(const FlintConstUtf8 *)"\x10\x00\x27\x06""java/lang/System";
const FlintConstUtf8 &stringClassName = *(const FlintConstUtf8 *)"\x10\x00\x19\x06""java/lang/String";
const FlintConstUtf8 &characterClassName = *(const FlintConstUtf8 *)"\x13\x00\x2F\x07""java/lang/Character";
const FlintConstUtf8 &throwableClassName = *(const FlintConstUtf8 *)"\x13\x00\x4A\x07""java/lang/Throwable";
const FlintConstUtf8 &printStreamClassName = *(const FlintConstUtf8 *)"\x13\x00\x51\x07""java/io/PrintStream";
const FlintConstUtf8 &nullPtrExcpClassName = *(const FlintConstUtf8 *)"\x1E\x00\xCD\x0B""java/lang/NullPointerException";
const FlintConstUtf8 &arrayStoreExceptionClassName = *(const FlintConstUtf8 *)"\x1D\x00\x5D\x0B""java/lang/ArrayStoreException";
const FlintConstUtf8 &arithmeticExceptionClassName = *(const FlintConstUtf8 *)"\x1D\x00\x5B\x0B""java/lang/ArithmeticException";
const FlintConstUtf8 &classNotFoundExceptionClassName = *(const FlintConstUtf8 *)"\x20\x00\x74\x0C""java/lang/ClassNotFoundException";
const FlintConstUtf8 &cloneNotSupportedExceptionClassName = *(const FlintConstUtf8 *)"\x24\x00\x39\x0E""java/lang/CloneNotSupportedException";
const FlintConstUtf8 &negativeArraySizeExceptionClassName = *(const FlintConstUtf8 *)"\x24\x00\x1E\x0E""java/lang/NegativeArraySizeException";
const FlintConstUtf8 &unsupportedOperationExceptionClassName = *(const FlintConstUtf8 *)"\x27\x00\xAB\x0F""java/lang/UnsupportedOperationException";
const FlintConstUtf8 &arrayIndexOutOfBoundsExceptionClassName = *(const FlintConstUtf8 *)"\x28\x00\xA0\x0F""java/lang/ArrayIndexOutOfBoundsException";
