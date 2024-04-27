
#ifndef __MJVM_CONST_NAME_H
#define __MJVM_CONST_NAME_H

#include "mjvm_const_pool.h"

extern const ConstUtf8 *primTypeConstUtf8List[];
extern const uint32_t stringValueFieldName[];
extern const uint32_t stringCoderFieldName[];
extern const uint32_t exceptionDetailMessageFieldName[];

extern const ConstUtf8 &mathClass;
extern const ConstUtf8 &floatClass;
extern const ConstUtf8 &doubleClass;
extern const ConstUtf8 &objectClass;
extern const ConstUtf8 &systemClass;
extern const ConstUtf8 &printStreamClass;
extern const ConstUtf8 &stringClassName;
extern const ConstUtf8 &nullPtrExcpClassName;
extern const ConstUtf8 &arrayStoreExceptionClassName;
extern const ConstUtf8 &arithmeticExceptionClassName;
extern const ConstUtf8 &classNotFoundExceptionClassName;
extern const ConstUtf8 &cloneNotSupportedExceptionClassName;
extern const ConstUtf8 &negativeArraySizeExceptionClassName;
extern const ConstUtf8 &arrayIndexOutOfBoundsExceptionClassName;

#endif /* __MJVM_CONST_NAME_H */
