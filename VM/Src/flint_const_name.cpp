
#include "flint_const_name.h"

const FlintConstUtf8 * const primTypeConstUtf8List[] = {
    (FlintConstUtf8 *)"\x01\x00\xC0\x84""Z",                 /* boolean */
    (FlintConstUtf8 *)"\x01\x00\x01\x4E""C",                 /* char */
    (FlintConstUtf8 *)"\x01\x00\xC1\x4D""F",                 /* float */
    (FlintConstUtf8 *)"\x01\x00\x40\x8C""D",                 /* double */
    (FlintConstUtf8 *)"\x01\x00\xC0\x8E""B",                 /* byte */
    (FlintConstUtf8 *)"\x01\x00\x00\x82""S",                 /* short */
    (FlintConstUtf8 *)"\x01\x00\x81\x49""I",                 /* integer */
    (FlintConstUtf8 *)"\x01\x00\xC1\x48""J",                 /* long */
    (FlintConstUtf8 *)"\x01\x00\xC0\x81""V",                 /* void */
};

const FlintConstUtf8 &mathClassName = *(const FlintConstUtf8 *)"\x0E\x00\x37\xC8""java/lang/Math";
const FlintConstUtf8 &byteClassName = *(const FlintConstUtf8 *)"\x0E\x00\x75\x1E""java/lang/Byte";
const FlintConstUtf8 &longClassName = *(const FlintConstUtf8 *)"\x0E\x00\x1C\x93""java/lang/Long";
const FlintConstUtf8 &shortClassName = *(const FlintConstUtf8 *)"\x0F\x00\x19\xB7""java/lang/Short";
const FlintConstUtf8 &errorClassName = *(const FlintConstUtf8 *)"\x0F\x00\x4E\x38""java/lang/Error";
const FlintConstUtf8 &classClassName = *(const FlintConstUtf8 *)"\x0F\x00\xF8\xD5""java/lang/Class";
const FlintConstUtf8 &floatClassName = *(const FlintConstUtf8 *)"\x0F\x00\x18\x74""java/lang/Float";
const FlintConstUtf8 &doubleClassName = *(const FlintConstUtf8 *)"\x10\x00\x4C\x64""java/lang/Double";
const FlintConstUtf8 &objectClassName = *(const FlintConstUtf8 *)"\x10\x00\x13\x37""java/lang/Object";
const FlintConstUtf8 &systemClassName = *(const FlintConstUtf8 *)"\x10\x00\xE0\x5A""java/lang/System";
const FlintConstUtf8 &stringClassName = *(const FlintConstUtf8 *)"\x10\x00\xDB\x56""java/lang/String";
const FlintConstUtf8 &threadClassName = *(const FlintConstUtf8 *)"\x10\x00\x8E\xD1""java/lang/Thread";
const FlintConstUtf8 &booleanClassName = *(const FlintConstUtf8 *)"\x11\x00\x4B\x4E""java/lang/Boolean";
const FlintConstUtf8 &integerClassName = *(const FlintConstUtf8 *)"\x11\x00\x35\x08""java/lang/Integer";
const FlintConstUtf8 &arrayClassName = *(const FlintConstUtf8 *)"\x17\x00\xBD\xBC""java/lang/reflect/Array";
const FlintConstUtf8 &characterClassName = *(const FlintConstUtf8 *)"\x13\x00\x92\x49""java/lang/Character";
const FlintConstUtf8 &throwableClassName = *(const FlintConstUtf8 *)"\x13\x00\xB7\x14""java/lang/Throwable";
const FlintConstUtf8 &exceptionClassName = *(const FlintConstUtf8 *)"\x13\x00\xF6\xDA""java/lang/Exception";
const FlintConstUtf8 &bigIntegerClassName = *(const FlintConstUtf8 *)"\x14\x00\x72\xF5""java/math/BigInteger";
const FlintConstUtf8 &printStreamClassName = *(const FlintConstUtf8 *)"\x13\x00\x34\x38""java/io/PrintStream";
const FlintConstUtf8 &ioExceptionClassName = *(const FlintConstUtf8 *)"\x15\x00\xE3\x8E""java/lang/IOException";
const FlintConstUtf8 &flintGraphicsClassName = *(const FlintConstUtf8 *)"\x16\x00\x9C\xA2""flint/drawing/Graphics";
const FlintConstUtf8 &classCastExceptionClassName = *(const FlintConstUtf8 *)"\x1C\x00\x79\xFD""java/lang/ClassCastException";
const FlintConstUtf8 &arrayStoreExceptionClassName = *(const FlintConstUtf8 *)"\x1D\x00\xCA\x26""java/lang/ArrayStoreException";
const FlintConstUtf8 &arithmeticExceptionClassName = *(const FlintConstUtf8 *)"\x1D\x00\x19\x72""java/lang/ArithmeticException";
const FlintConstUtf8 &nullPointerExceptionClassName = *(const FlintConstUtf8 *)"\x1E\x00\x0F\xCB""java/lang/NullPointerException";
const FlintConstUtf8 &unsatisfiedLinkErrorClassName = *(const FlintConstUtf8 *)"\x1E\x00\x77\x94""java/lang/UnsatisfiedLinkError";
const FlintConstUtf8 &interruptedExceptionClassName = *(const FlintConstUtf8 *)"\x1E\x00\x19\x97""java/lang/InterruptedException";
const FlintConstUtf8 &classNotFoundExceptionClassName = *(const FlintConstUtf8 *)"\x20\x00\xFD\xFC""java/lang/ClassNotFoundException";
const FlintConstUtf8 &illegalArgumentExceptionClassName = *(const FlintConstUtf8 *)"\x22\x00\x6D\x2A""java/lang/IllegalArgumentException";
const FlintConstUtf8 &cloneNotSupportedExceptionClassName = *(const FlintConstUtf8 *)"\x24\x00\xF3\xB9""java/lang/CloneNotSupportedException";
const FlintConstUtf8 &negativeArraySizeExceptionClassName = *(const FlintConstUtf8 *)"\x24\x00\x7F\xE4""java/lang/NegativeArraySizeException";
const FlintConstUtf8 &unsupportedOperationExceptionClassName = *(const FlintConstUtf8 *)"\x27\x00\xE6\x1D""java/lang/UnsupportedOperationException";
const FlintConstUtf8 &arrayIndexOutOfBoundsExceptionClassName = *(const FlintConstUtf8 *)"\x28\x00\x90\x1F""java/lang/ArrayIndexOutOfBoundsException";

const FlintConstUtf8 &constructorName = *(const FlintConstUtf8 *)"\x06\x00\xCC\xF5""<init>";
const FlintConstUtf8 &staticConstructorName = *(const FlintConstUtf8 *)"\x08\x00\xD0\xF2""<clinit>";
