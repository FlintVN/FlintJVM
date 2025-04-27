
#include <string.h>
#include <iostream>
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

FlintError throwArrayIndexOutOfBoundsException(FlintExecution &execution, int32_t index, int32_t length) {
    char indexStrBuff[11];
    char lengthStrBuff[11];
    sprintf(indexStrBuff, "%d", (int)index);
    sprintf(lengthStrBuff, "%d", (int)length);
    const char *msg[] = {"Index ", indexStrBuff, " out of bounds for length ", lengthStrBuff};
    FlintJavaString &strObj = execution.flint.newString(msg, LENGTH(msg));
    FlintJavaThrowable &excpObj = execution.flint.newArrayIndexOutOfBoundsException(&strObj);
    execution.stackPushObject(&excpObj);
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
