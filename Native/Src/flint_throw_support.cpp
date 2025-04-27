
#include <string.h>
#include "flint_throw_support.h"

FlintError throwIllegalArgumentException(FlintExecution &execution, const char *msg) {
    FlintJavaThrowable *excp;
    if(msg) {
        FlintJavaString *strObj = &execution.flint.newString(msg);
        excp = &execution.flint.newIllegalArgumentException(strObj);
    }
    else
        excp = &execution.flint.newIllegalArgumentException(NULL);
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwNullPointerException(FlintExecution &execution, const char *msg) {
    FlintJavaThrowable *excp;
    if(msg) {
        FlintJavaString *strObj = &execution.flint.newString(msg);
        excp = &execution.flint.newNullPointerException(strObj);
    }
    else
        excp = &execution.flint.newNullPointerException(NULL);
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwArrayIndexOutOfBoundsException(FlintExecution &execution) {
    FlintJavaThrowable *excp = &execution.flint.newArrayIndexOutOfBoundsException(NULL);
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwNegativeArraySizeException(FlintExecution &execution) {
    FlintJavaThrowable *excp = &execution.flint.newNegativeArraySizeException(NULL);
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwClassNotFoundException(FlintExecution &execution, FlintJavaString *msg) {
    FlintJavaThrowable *excp = &execution.flint.newClassNotFoundException(msg);
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwClassNotFoundException(FlintExecution &execution, const char *msg) {
    FlintJavaString *strObj = &execution.flint.newString(msg);
    return throwClassNotFoundException(execution, strObj);
}

FlintError throwCloneNotSupportedException(FlintExecution &execution, const char *msg) {
    FlintJavaString *strObj = &execution.flint.newString(msg);
    FlintJavaThrowable *excp = &execution.flint.newCloneNotSupportedException(strObj);
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwArrayStoreException(FlintExecution &execution, const char *msg) {
    FlintJavaString *strObj = &execution.flint.newString(msg);
    FlintJavaThrowable *excp = &execution.flint.newArrayStoreException(strObj);
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwInterruptedException(FlintExecution &execution) {
    FlintJavaThrowable *excp = &execution.flint.newInterruptedException(NULL);
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwUnsupportedOperationException(FlintExecution &execution, const char *msg) {
    FlintJavaString *strObj = &execution.flint.newString(msg);
    FlintJavaThrowable *excp = &execution.flint.newUnsupportedOperationException(strObj);
    execution.stackPushObject(excp);
    return ERR_THROW;
}

FlintError throwIOException(FlintExecution &execution, const char *msg) {
    FlintJavaString *strObj = &execution.flint.newString(msg);
    FlintJavaThrowable *excp = &execution.flint.newIOException(strObj);
    execution.stackPushObject(excp);
    return ERR_THROW;
}
