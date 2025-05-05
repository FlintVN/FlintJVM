#ifndef __FLINT_THROW_SUPPORT_H
#define __FLINT_THROW_SUPPORT_H

#include "flint.h"

FlintError throwException(FlintExecution &execution, const char *msg = 0);
FlintError throwIOException(FlintExecution &execution, const char *msg = 0);
FlintError throwErrorException(FlintExecution &execution, const char *msg = 0);
FlintError throwClassCastException(FlintExecution &execution, FlintJavaObject *obj, const FlintConstUtf8 &type);
FlintError throwArrayStoreException(FlintExecution &execution, const char *msg = 0);
FlintError throwArithmeticException(FlintExecution &execution, const char *msg = 0);
FlintError throwNullPointerException(FlintExecution &execution, const char *msg = 0);
FlintError throwNullPointerException(FlintExecution &execution, FlintConstMethod &constMethod);
FlintError throwNullPointerException(FlintExecution &execution, FlintConstField &constField);
FlintError throwInterruptedException(FlintExecution &execution, const char *msg = 0);
FlintError throwClassNotFoundException(FlintExecution &execution, const char *msg = 0);
FlintError throwClassNotFoundException(FlintExecution &execution, FlintJavaString *str);
FlintError throwIllegalArgumentException(FlintExecution &execution, const char *msg = 0);
FlintError throwCloneNotSupportedException(FlintExecution &execution, const char *msg = 0);
FlintError throwNegativeArraySizeException(FlintExecution &execution, const char *msg = 0);
FlintError throwArrayIndexOutOfBoundsException(FlintExecution &execution, int32_t index, int32_t length);
FlintError throwUnsupportedOperationException(FlintExecution &execution, const char *msg = 0);
FlintError throwUnsatisfiedLinkErrorException(FlintExecution &execution, const char *msg = 0);
FlintError throwNoSuchMethodError(FlintExecution &execution, const char *className, const char *methodName);
FlintError throwNoSuchFieldError(FlintExecution &execution, const char *className, const char *fieldName);

#endif /* __FLINT_THROW_SUPPORT_H */
