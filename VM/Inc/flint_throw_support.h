#ifndef __FLINT_THROW_SUPPORT_H
#define __FLINT_THROW_SUPPORT_H

#include "flint.h"

FlintError throwException(FlintExecution *exec, const char *msg = 0, uint32_t length = 0);
FlintError throwIOException(FlintExecution *exec, const char *msg = 0, uint32_t length = 0);
FlintError throwErrorException(FlintExecution *exec, const char *msg = 0, uint32_t length = 0);
FlintError throwClassCastException(FlintExecution *exec, JObject *obj, FlintConstUtf8 &type);
FlintError throwArrayStoreException(FlintExecution *exec, const char *msg = 0, uint32_t length = 0);
FlintError throwArithmeticException(FlintExecution *exec, const char *msg = 0, uint32_t length = 0);
FlintError throwNullPointerException(FlintExecution *exec, const char *msg = 0, uint32_t length = 0);
FlintError throwNullPointerException(FlintExecution *exec, FlintConstMethod *constMethod);
FlintError throwNullPointerException(FlintExecution *exec, FlintConstField *constField);
FlintError throwInterruptedException(FlintExecution *exec, const char *msg = 0, uint32_t length = 0);
FlintError throwClassNotFoundException(FlintExecution *exec, const char *msg = 0, uint32_t length = 0);
FlintError throwClassNotFoundException(FlintExecution *exec, JString *str);
FlintError throwIllegalArgumentException(FlintExecution *exec, const char *msg = 0, uint32_t length = 0);
FlintError throwCloneNotSupportedException(FlintExecution *exec, const char *msg = 0, uint32_t length = 0);
FlintError throwNegativeArraySizeException(FlintExecution *exec, const char *msg = 0, uint32_t length = 0);
FlintError throwArrayIndexOutOfBoundsException(FlintExecution *exec, int32_t index, int32_t length);
FlintError throwUnsupportedOperationException(FlintExecution *exec, const char *msg = 0, uint32_t length = 0);
FlintError throwUnsatisfiedLinkErrorException(FlintExecution *exec, const char *msg = 0, uint32_t length = 0);
FlintError throwNoSuchMethodError(FlintExecution *exec, const char *className, const char *methodName);
FlintError throwNoSuchFieldError(FlintExecution *exec, const char *className, const char *fieldName);
FlintError throwClassFormatError(FlintExecution *exec, const char *className);

FlintError checkAndThrowForFlintError(FlintExecution *exec, FlintError err, FlintConstUtf8 *className);
FlintError checkAndThrowForFlintError(FlintExecution *exec, FlintError err, const char *className, uint16_t length);

#endif /* __FLINT_THROW_SUPPORT_H */
