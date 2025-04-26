
#include <string.h>
#include "flint_throw_support.h"

FlintError throwIllegalArgumentException(FlintExecution &execution, const char *msg) {
    FlintJavaThrowable *excp;
    if(msg) {
        int32_t len = strlen(msg);
        FlintJavaString *strObj = &execution.flint.newString(msg, len);
        excp = &execution.flint.newIllegalArgumentException(strObj);
    }
    else
        excp = &execution.flint.newIllegalArgumentException();
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwNullPointerException(FlintExecution &execution, const char *msg) {
    FlintJavaThrowable *excp;
    if(msg) {
        int32_t len = strlen(msg);
        FlintJavaString *strObj = &execution.flint.newString(msg, len);
        excp = &execution.flint.newNullPointerException(strObj);
    }
    else
        excp = &execution.flint.newNullPointerException();
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwArrayIndexOutOfBoundsException(FlintExecution &execution) {
    FlintJavaThrowable *excp = &execution.flint.newArrayIndexOutOfBoundsException();
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwNegativeArraySizeException(FlintExecution &execution) {
    FlintJavaThrowable *excp = &execution.flint.newNegativeArraySizeException();
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwClassNotFoundException(FlintExecution &execution, FlintJavaString *msg) {
    FlintJavaThrowable *excp = &execution.flint.newClassNotFoundException(msg);
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwClassNotFoundException(FlintExecution &execution, const char *msg) {
    int32_t len = strlen(msg);
    FlintJavaString *strObj = &execution.flint.newString(msg, len);
    return throwClassNotFoundException(execution, strObj);
}

FlintError throwCloneNotSupportedException(FlintExecution &execution, const char *msg) {
    int32_t len = strlen(msg);
    FlintJavaString *strObj = &execution.flint.newString(msg, len);
    FlintJavaThrowable *excp = &execution.flint.newCloneNotSupportedException(strObj);
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwArrayStoreException(FlintExecution &execution, const char *msg) {
    int32_t len = strlen(msg);
    FlintJavaString *strObj = &execution.flint.newString(msg, len);
    FlintJavaThrowable *excp = &execution.flint.newArrayStoreException(strObj);
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwInterruptedException(FlintExecution &execution) {
    FlintJavaThrowable *excp = &execution.flint.newInterruptedException();
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwUnsupportedOperationException(FlintExecution &execution, const char *msg) {
    int32_t len = strlen(msg);
    FlintJavaString *strObj = &execution.flint.newString(msg, len);
    FlintJavaThrowable *excp = &execution.flint.newUnsupportedOperationException(strObj);
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwIOException(FlintExecution &execution, const char *msg) {
    int32_t len = strlen(msg);
    FlintJavaString *strObj = &execution.flint.newString(msg, len);
    FlintJavaThrowable *excp = &execution.flint.newIOException(strObj);
    execution.stackPushObject(excp);
    return ERR_THROW;
}
