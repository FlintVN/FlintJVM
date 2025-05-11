
#include "flint_const_name_base.h"

alignas(4) const char booleanPrimTypeName[] = "\x01\x00\xC0\x84""Z";
alignas(4) const char charPrimTypeName[] = "\x01\x00\x01\x4E""C";
alignas(4) const char floatPrimTypeName[] = "\x01\x00\xC1\x4D""F";
alignas(4) const char doublePrimTypeName[] = "\x01\x00\x40\x8C""D";
alignas(4) const char bytePrimTypeName[] = "\x01\x00\xC0\x8E""B";
alignas(4) const char shortPrimTypeName[] = "\x01\x00\x00\x82""S";
alignas(4) const char integerPrimTypeName[] = "\x01\x00\x81\x49""I";
alignas(4) const char longPrimTypeName[] = "\x01\x00\xC1\x48""J";
alignas(4) const char voidPrimTypeName[] = "\x01\x00\xC0\x81""V";

alignas(4) const char mathClassName[] = "\x0E\x00\x37\xC8""java/lang/Math";
alignas(4) const char byteClassName[] = "\x0E\x00\x75\x1E""java/lang/Byte";
alignas(4) const char longClassName[] = "\x0E\x00\x1C\x93""java/lang/Long";
alignas(4) const char shortClassName[] = "\x0F\x00\x19\xB7""java/lang/Short";
alignas(4) const char errorClassName[] = "\x0F\x00\x4E\x38""java/lang/Error";
alignas(4) const char classClassName[] = "\x0F\x00\xF8\xD5""java/lang/Class";
alignas(4) const char floatClassName[] = "\x0F\x00\x18\x74""java/lang/Float";
alignas(4) const char doubleClassName[] = "\x10\x00\x4C\x64""java/lang/Double";
alignas(4) const char objectClassName[] = "\x10\x00\x13\x37""java/lang/Object";
alignas(4) const char systemClassName[] = "\x10\x00\xE0\x5A""java/lang/System";
alignas(4) const char stringClassName[] = "\x10\x00\xDB\x56""java/lang/String";
alignas(4) const char threadClassName[] = "\x10\x00\x8E\xD1""java/lang/Thread";
alignas(4) const char booleanClassName[] = "\x11\x00\x4B\x4E""java/lang/Boolean";
alignas(4) const char integerClassName[] = "\x11\x00\x35\x08""java/lang/Integer";
alignas(4) const char arrayClassName[] = "\x17\x00\xBD\xBC""java/lang/reflect/Array";
alignas(4) const char characterClassName[] = "\x13\x00\x92\x49""java/lang/Character";
alignas(4) const char throwableClassName[] = "\x13\x00\xB7\x14""java/lang/Throwable";
alignas(4) const char exceptionClassName[] = "\x13\x00\xF6\xDA""java/lang/Exception";
alignas(4) const char fieldClassName[] = "\x17\x00\x7A\x05""java/lang/reflect/Field";
alignas(4) const char bigIntegerClassName[] = "\x14\x00\x72\xF5""java/math/BigInteger";
alignas(4) const char printStreamClassName[] = "\x13\x00\x34\x38""java/io/PrintStream";
alignas(4) const char methodClassName[] = "\x18\x00\x97\x00""java/lang/reflect/Method";
alignas(4) const char ioExceptionClassName[] = "\x15\x00\xE3\x8E""java/lang/IOException";
alignas(4) const char flintGraphicsClassName[] = "\x16\x00\x9C\xA2""flint/drawing/Graphics";
alignas(4) const char constructorClassName[] = "\x1D\x00\x9E\x82""java/lang/reflect/Constructor";
alignas(4) const char classCastExceptionClassName[] = "\x1C\x00\x79\xFD""java/lang/ClassCastException";
alignas(4) const char arrayStoreExceptionClassName[] = "\x1D\x00\xCA\x26""java/lang/ArrayStoreException";
alignas(4) const char arithmeticExceptionClassName[] = "\x1D\x00\x19\x72""java/lang/ArithmeticException";
alignas(4) const char nullPointerExceptionClassName[] = "\x1E\x00\x0F\xCB""java/lang/NullPointerException";
alignas(4) const char unsatisfiedLinkErrorClassName[] = "\x1E\x00\x77\x94""java/lang/UnsatisfiedLinkError";
alignas(4) const char interruptedExceptionClassName[] = "\x1E\x00\x19\x97""java/lang/InterruptedException";
alignas(4) const char classFormatErrorExceptionClassName[] = "\x1A\x00\xA6\x1E""java/lang/ClassFormatError";
alignas(4) const char noSuchFieldErrorExceptionClassName[] = "\x1A\x00\xF5\x2A""java/lang/NoSuchFieldError";
alignas(4) const char noSuchMethodErrorExceptionClassName[] = "\x1B\x00\xDE\x86""java/lang/NoSuchMethodError";
alignas(4) const char classNotFoundExceptionClassName[] = "\x20\x00\xFD\xFC""java/lang/ClassNotFoundException";
alignas(4) const char illegalArgumentExceptionClassName[] = "\x22\x00\x6D\x2A""java/lang/IllegalArgumentException";
alignas(4) const char cloneNotSupportedExceptionClassName[] = "\x24\x00\xF3\xB9""java/lang/CloneNotSupportedException";
alignas(4) const char negativeArraySizeExceptionClassName[] = "\x24\x00\x7F\xE4""java/lang/NegativeArraySizeException";
alignas(4) const char unsupportedOperationExceptionClassName[] = "\x27\x00\xE6\x1D""java/lang/UnsupportedOperationException";
alignas(4) const char arrayIndexOutOfBoundsExceptionClassName[] = "\x28\x00\x90\x1F""java/lang/ArrayIndexOutOfBoundsException";

alignas(4) const char constructorName[] = "\x06\x00\xCC\xF5""<init>";
alignas(4) const char staticConstructorName[] = "\x08\x00\xD0\xF2""<clinit>";

alignas(4) const char nameFieldName[] = "\x04\x00\x5E\x56""name";
alignas(4) const char typeFieldName[] = "\x04\x00\xD0\x19""type";
alignas(4) const char clazzFieldName[] = "\x05\x00\xF1\xFA""clazz";
alignas(4) const char returnTypeFieldName[] = "\x0A\x00\x5E\x42""returnType";
alignas(4) const char parameterTypesFieldName[] = "\x0E\x00\x9B\xC1""parameterTypes";
alignas(4) const char exceptionTypesFieldName[] = "\x0E\x00\xBD\xB7""exceptionTypes";
alignas(4) const char modifiersFieldName[] = "\x09\x00\x0F\xC4""modifiers";

const FlintConstUtf8 * const primTypeConstUtf8List[] = {
    (FlintConstUtf8 *)booleanPrimTypeName,
    (FlintConstUtf8 *)charPrimTypeName,
    (FlintConstUtf8 *)floatPrimTypeName,
    (FlintConstUtf8 *)doublePrimTypeName,
    (FlintConstUtf8 *)bytePrimTypeName,
    (FlintConstUtf8 *)shortPrimTypeName,
    (FlintConstUtf8 *)integerPrimTypeName,
    (FlintConstUtf8 *)longPrimTypeName,
    (FlintConstUtf8 *)voidPrimTypeName,
};
