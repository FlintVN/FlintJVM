
#ifndef __MJVM_CONST_NAME_H
#define __MJVM_CONST_NAME_H

#include "mjvm_const_pool.h"

extern const ConstUtf8 *primTypeConstUtf8List[];
extern const uint32_t stringValueFieldName[];
extern const uint32_t exceptionDetailMessageFieldName[];
extern const ConstUtf8 &stringClassName;
extern const ConstUtf8 &nullPtrExcpClassName;
extern const ConstUtf8 &arithmeticExceptionClassName;
extern const ConstUtf8 &negativeArraySizeExceptionClassName;
extern const ConstUtf8 &arrayIndexOutOfBoundsExceptionClassName;

#endif /* __MJVM_CONST_NAME_H */
