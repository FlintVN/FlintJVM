
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

const uint32_t stringCoderFieldName[] = {
    (uint32_t)"\x05\x00""coder",                /* field name */
    (uint32_t)"\x01\x00""B"                     /* field type */
};

const uint32_t exceptionDetailMessageFieldName[] = {
    (uint32_t)"\x0D\x00""detailMessage",        /* field name */
    (uint32_t)"\x12\x00""Ljava/lang/String;"    /* field type */
};

const ConstUtf8 &mathClass = *(const ConstUtf8 *)"\x0E\x00""java/lang/Math";
const ConstUtf8 &floatClass = *(const ConstUtf8 *)"\x0F\x00""java/lang/Float";
const ConstUtf8 &doubleClass = *(const ConstUtf8 *)"\x10\x00""java/lang/Double";
const ConstUtf8 &objectClass = *(const ConstUtf8 *)"\x10\x00""java/lang/Object";
const ConstUtf8 &systemClass = *(const ConstUtf8 *)"\x10\x00""java/lang/System";
const ConstUtf8 &characterClass = *(const ConstUtf8 *)"\x13\x00""java/lang/Character";
const ConstUtf8 &printStreamClass = *(const ConstUtf8 *)"\x13\x00""java/io/PrintStream";
const ConstUtf8 &stringClassName = *(const ConstUtf8 *)"\x10\x00""java/lang/String";
const ConstUtf8 &nullPtrExcpClassName = *(const ConstUtf8 *)"\x1E\x00""java/lang/NullPointerException";
const ConstUtf8 &arrayStoreExceptionClassName = *(const ConstUtf8 *)"\x1D\x00""java/lang/ArrayStoreException";
const ConstUtf8 &arithmeticExceptionClassName = *(const ConstUtf8 *)"\x1D\x00""java/lang/ArithmeticException";
const ConstUtf8 &classNotFoundExceptionClassName = *(const ConstUtf8 *)"\x20\x00""java/lang/ClassNotFoundException";
const ConstUtf8 &cloneNotSupportedExceptionClassName = *(const ConstUtf8 *)"\x24\x00""java/lang/CloneNotSupportedException";
const ConstUtf8 &negativeArraySizeExceptionClassName = *(const ConstUtf8 *)"\x24\x00""java/lang/NegativeArraySizeException";
const ConstUtf8 &arrayIndexOutOfBoundsExceptionClassName = *(const ConstUtf8 *)"\x28\x00""java/lang/ArrayIndexOutOfBoundsException";
