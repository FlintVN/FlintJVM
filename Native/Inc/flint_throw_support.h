#ifndef __FLINT_THROW_SUPPORT_H
#define __FLINT_THROW_SUPPORT_H

#include "flint.h"

FlintError throwIllegalArgumentException(FlintExecution &execution, const char *msg = 0);
FlintError throwNullPointerException(FlintExecution &execution, const char *msg = 0);
FlintError throwArrayIndexOutOfBoundsException(FlintExecution &execution);
FlintError throwNegativeArraySizeException(FlintExecution &execution);
FlintError throwClassNotFoundException(FlintExecution &execution, FlintJavaString *msg);
FlintError throwClassNotFoundException(FlintExecution &execution, const char *msg);
FlintError throwCloneNotSupportedException(FlintExecution &execution, const char *msg);
FlintError throwArrayStoreException(FlintExecution &execution, const char *msg);
FlintError throwInterruptedException(FlintExecution &execution);
FlintError throwUnsupportedOperationException(FlintExecution &execution, const char *msg);
FlintError throwIOException(FlintExecution &execution, const char *msg);

#endif /* __FLINT_THROW_SUPPORT_H */
