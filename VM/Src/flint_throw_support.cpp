
#include <string.h>
#include <iostream>
#include "flint_throw_support.h"

static FlintError throwThrowable(FlintExecution &execution, const FlintConstUtf8 &excpType, const char *msg) {
    FlintJavaString *strObj = NULL;
    if(msg)
        RETURN_IF_ERR(execution.flint.newString(msg, strObj));
    FlintJavaThrowable *excp;
    FlintError err = execution.flint.newThrowable(strObj, excpType, excp);
    if(err != ERR_OK) {
        if(strObj) {
            execution.flint.freeObject(*strObj->getValue());
            execution.flint.freeObject(*strObj);
        }
        return err;
    }
    execution.stackPushObject(excp);
    return ERR_THROW;
}

static FlintError throwThrowable(FlintExecution &execution, const FlintConstUtf8 &excpType, FlintJavaString *str) {
    FlintJavaThrowable *excp;
    RETURN_IF_ERR(execution.flint.newThrowable(str, excpType, excp));
    execution.stackPushObject(excp);
    return ERR_THROW;
}

static uint8_t countDigits(int32_t num) {
    uint8_t count = 0;
    if(num < 0)
        count++;
    do {
        count++;
        num /= 10;
    } while(num);
    return count;
}

static uint32_t sprint(char *buff, uint32_t index, const char *txt) {
    uint32_t i = 0;
    while(txt[i]) {
        buff[index + i] = txt[i];
        i++;
    }
    return i + index;
}

static uint32_t sprint(char *buff, uint32_t index, const char *txt, char oldChar, char newChar) {
    uint32_t i = 0;
    while(txt[i]) {
        buff[index + i] = (txt[i] == oldChar) ? newChar : txt[i];
        i++;
    }
    return i + index;
}

static uint32_t sprint(char *buff, uint32_t index, int32_t num) {
    int32_t tmp = num;
    uint8_t count = countDigits(tmp);
    index += count;
    if(tmp < 0)
        tmp = -tmp;
    do {
        buff[--index] = (tmp % 10) + '0';
        tmp /= 10;
    } while(tmp);
    if(num < 0)
        buff[--index] = '-';
    return count + index;
}

FlintError throwException(FlintExecution &execution, const char *msg) {
    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)exceptionClassName, msg);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)exceptionClassName);
}

FlintError throwIOException(FlintExecution &execution, const char *msg) {
    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)ioExceptionClassName, msg);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)ioExceptionClassName);
}

FlintError throwErrorException(FlintExecution &execution, const char *msg) {
    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)errorClassName, msg);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)errorClassName);
}

FlintError throwClassCastException(FlintExecution &execution, FlintJavaObject *obj, const FlintConstUtf8 &type) {
    uint32_t strLen = strlen("Class '") + obj->dimensions + obj->type.length + strlen("' cannot be cast to class '") + type.length + strlen("'");
    bool isPrimType = FlintJavaObject::isPrimType(obj->type);
    if(!isPrimType)
        strLen += 2;

    FlintJavaString *str;
    RETURN_IF_ERR(execution.flint.newString(strLen, 0, str));

    uint32_t i = 0;
    char *txt = str->getText();
    i = sprint(txt, i, "Class '");
    for(uint32_t i = 0; i < obj->dimensions; i++)
        txt[i++] = '[';
    if(!isPrimType)
        txt[i++] = 'L';
    i = sprint(txt, i, obj->type.text, '/', '.');
    if(!isPrimType)
        txt[i++] = ';';
    i = sprint(txt, i, "' cannot be cast to class '");
    i = sprint(txt, i, type.text, '/', '.');
    i = sprint(txt, i, "'");

    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)classCastExceptionClassName, str);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)classCastExceptionClassName);
}

FlintError throwArrayStoreException(FlintExecution &execution, const char *msg) {
    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)arrayStoreExceptionClassName, msg);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)arrayStoreExceptionClassName);
}

FlintError throwArithmeticException(FlintExecution &execution, const char *msg) {
    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)arithmeticExceptionClassName, msg);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)arithmeticExceptionClassName);
}

FlintError throwNullPointerException(FlintExecution &execution, const char *msg) {
    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)nullPointerExceptionClassName, msg);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)nullPointerExceptionClassName);
}

FlintError throwNullPointerException(FlintExecution &execution, FlintConstMethod &constMethod) {
    FlintConstMethod &cm = constMethod;
    uint32_t strLen = strlen("Cannot invoke ") + cm.className.length + strlen(".") + cm.nameAndType.name.length + strlen(" by null object");
    FlintJavaString *str;
    RETURN_IF_ERR(execution.flint.newString(strLen, 0, str));

    uint32_t i = 0;
    char *txt = str->getText();
    i = sprint(txt, i, "Cannot invoke ");
    i = sprint(txt, i, cm.className.text, '/', '.');
    i = sprint(txt, i, ".");
    i = sprint(txt, i, cm.nameAndType.name.text);
    i = sprint(txt, i, " by null object");

    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)nullPointerExceptionClassName, str);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)nullPointerExceptionClassName);
}

FlintError throwNullPointerException(FlintExecution &execution, FlintConstField &constField) {
    FlintConstField &cm = constField;
    uint32_t strLen = strlen("Cannot access field ") + cm.className.length + strlen(".") + cm.nameAndType.name.length + strlen(" from null object");
    FlintJavaString *str;
    RETURN_IF_ERR(execution.flint.newString(strLen, 0, str));

    uint32_t i = 0;
    char *txt = str->getText();
    i = sprint(txt, i, "Cannot access field ");
    i = sprint(txt, i, constField.className.text, '/', '.');
    i = sprint(txt, i, ".");
    i = sprint(txt, i, constField.nameAndType.name.text, '/', '.');
    i = sprint(txt, i, " from null object");

    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)nullPointerExceptionClassName, str);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)nullPointerExceptionClassName);
}

FlintError throwInterruptedException(FlintExecution &execution, const char *msg) {
    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)interruptedExceptionClassName, msg);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)interruptedExceptionClassName);
}

FlintError throwClassNotFoundException(FlintExecution &execution, const char *msg) {
    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)classNotFoundExceptionClassName, msg);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)classNotFoundExceptionClassName);
}

FlintError throwClassNotFoundException(FlintExecution &execution, FlintJavaString *str) {
    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)classNotFoundExceptionClassName, str);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)classNotFoundExceptionClassName);
}

FlintError throwIllegalArgumentException(FlintExecution &execution, const char *msg) {
    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)illegalArgumentExceptionClassName, msg);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)illegalArgumentExceptionClassName);
}

FlintError throwCloneNotSupportedException(FlintExecution &execution, const char *msg) {
    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)cloneNotSupportedExceptionClassName, msg);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)cloneNotSupportedExceptionClassName);
}

FlintError throwNegativeArraySizeException(FlintExecution &execution, const char *msg) {
    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)negativeArraySizeExceptionClassName, msg);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)negativeArraySizeExceptionClassName);
}

FlintError throwArrayIndexOutOfBoundsException(FlintExecution &execution, int32_t index, int32_t length) {
    uint32_t strLen = strlen("Index ") + countDigits(index) + strlen(" out of bounds for length ") + countDigits(length);
    FlintJavaString *str;
    RETURN_IF_ERR(execution.flint.newString(strLen, 0, str));

    uint32_t i = 0;
    char *txt = str->getText();
    i = sprint(txt, i, "Index ");
    i = sprint(txt, i, index);
    i = sprint(txt, i, " out of bounds for length ");
    i = sprint(txt, i, length);

    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)arrayIndexOutOfBoundsExceptionClassName, str);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)arrayIndexOutOfBoundsExceptionClassName);
}

FlintError throwUnsupportedOperationException(FlintExecution &execution, const char *msg) {
    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)unsupportedOperationExceptionClassName, msg);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)unsupportedOperationExceptionClassName);
}

FlintError throwUnsatisfiedLinkErrorException(FlintExecution &execution, const char *msg) {
    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)unsatisfiedLinkErrorClassName, msg);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)unsatisfiedLinkErrorClassName);
}

FlintError throwNoSuchMethodError(FlintExecution &execution, const char *className, const char *methodName) {
    uint32_t strLen = strlen("Could not find the method ") + strlen(className) + strlen(".") + strlen(methodName);
    FlintJavaString *str;
    RETURN_IF_ERR(execution.flint.newString(strLen, 0, str));

    uint32_t i = 0;
    char *txt = str->getText();
    i = sprint(txt, i, "Could not find the method ");
    i = sprint(txt, i, className, '/', '.');
    i = sprint(txt, i, ".");
    i = sprint(txt, i, methodName);

    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)noSuchMethodErrorExceptionClassName, str);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)noSuchMethodErrorExceptionClassName);
}

FlintError throwNoSuchFieldError(FlintExecution &execution, const char *className, const char *fieldName) {
    uint32_t strLen = strlen("Could not find the field ") + strlen(className) + strlen(".") + strlen(fieldName);
    FlintJavaString *str;
    RETURN_IF_ERR(execution.flint.newString(strLen, 0, str));

    uint32_t i = 0;
    char *txt = str->getText();
    i = sprint(txt, i, "Could not find the field ");
    i = sprint(txt, i,  className, '/', '.');
    i = sprint(txt, i, ".");
    i = sprint(txt, i, fieldName);

    FlintError err = throwThrowable(execution, *(FlintConstUtf8 *)noSuchFieldErrorExceptionClassName, str);
    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)noSuchFieldErrorExceptionClassName);
}

FlintError throwClassFormatError(FlintExecution &execution, const char *className) {
    uint32_t strLen = strlen("Invalid class file format: ") + strlen(className);
    FlintJavaString *str;
    RETURN_IF_ERR(execution.flint.newString(strLen, 0, str));

    uint32_t i = 0;
    char *txt = str->getText();
    i = sprint(txt, i, "Invalid class file format: ");
    i = sprint(txt, i, className, '/', '.');

    return throwThrowable(execution, *(FlintConstUtf8 *)classFormatErrorExceptionClassName, str);
}

FlintError checkAndThrowForFlintError(FlintExecution &execution, FlintError err, const FlintConstUtf8 *className) {
    return checkAndThrowForFlintError(execution, err, className->text, className->length);
}

FlintError checkAndThrowForFlintError(FlintExecution &execution, FlintError err, const char *className, uint16_t length) {
    if(err == ERR_CLASS_NOT_FOUND || err == ERR_CLASS_LOAD_FAIL) {
        FlintJavaString *str;
        RETURN_IF_ERR(execution.flint.newString(length, 0, str));

        char *txt = str->getText();
        for(uint32_t i = 0; i < length; i++)
            txt[i] = (className[i] != '/') ? className[i] : '.';

        if(err == ERR_CLASS_NOT_FOUND)
            return throwThrowable(execution, *(FlintConstUtf8 *)classNotFoundExceptionClassName, str);
        else if(err == ERR_CLASS_LOAD_FAIL)
            return throwThrowable(execution, *(FlintConstUtf8 *)classFormatErrorExceptionClassName, str);
    }
    return err;
}
