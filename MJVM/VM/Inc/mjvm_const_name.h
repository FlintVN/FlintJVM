
#ifndef __MJVM_CONST_NAME_H
#define __MJVM_CONST_NAME_H

#include "mjvm_const_pool.h"

extern const MjvmConstUtf8 * const primTypeConstUtf8List[];
extern const uint32_t stringNameFieldName[];
extern const uint32_t stringValueFieldName[];
extern const uint32_t stringCoderFieldName[];
extern const uint32_t exceptionDetailMessageFieldName[];

extern const MjvmConstUtf8 &mathClassName;
extern const MjvmConstUtf8 &classClassName;
extern const MjvmConstUtf8 &floatClassName;
extern const MjvmConstUtf8 &doubleClassName;
extern const MjvmConstUtf8 &objectClassName;
extern const MjvmConstUtf8 &systemClassName;
extern const MjvmConstUtf8 &stringClassName;
extern const MjvmConstUtf8 &characterClassName;
extern const MjvmConstUtf8 &printStreamClassName;
extern const MjvmConstUtf8 &nullPtrExcpClassName;
extern const MjvmConstUtf8 &arrayStoreExceptionClassName;
extern const MjvmConstUtf8 &arithmeticExceptionClassName;
extern const MjvmConstUtf8 &classNotFoundExceptionClassName;
extern const MjvmConstUtf8 &cloneNotSupportedExceptionClassName;
extern const MjvmConstUtf8 &negativeArraySizeExceptionClassName;
extern const MjvmConstUtf8 &arrayIndexOutOfBoundsExceptionClassName;

#endif /* __MJVM_CONST_NAME_H */
