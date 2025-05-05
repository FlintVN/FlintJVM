
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
    return throwThrowable(execution, exceptionClassName, msg);
}

FlintError throwIOException(FlintExecution &execution, const char *msg) {
    return throwThrowable(execution, exceptionClassName, msg);
}

FlintError throwErrorException(FlintExecution &execution, const char *msg) {
    return throwThrowable(execution, errorClassName, msg);
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

    return throwThrowable(execution, classCastExceptionClassName, str);
}

FlintError throwArrayStoreException(FlintExecution &execution, const char *msg) {
    return throwThrowable(execution, arrayStoreExceptionClassName, msg);
}

FlintError throwArithmeticException(FlintExecution &execution, const char *msg) {
    return throwThrowable(execution, arithmeticExceptionClassName, msg);
}

FlintError throwNullPointerException(FlintExecution &execution, const char *msg) {
    return throwThrowable(execution, nullPointerExceptionClassName, msg);
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

    return throwThrowable(execution, nullPointerExceptionClassName, str);
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

    return throwThrowable(execution, nullPointerExceptionClassName, str);
}

FlintError throwInterruptedException(FlintExecution &execution, const char *msg) {
    return throwThrowable(execution, interruptedExceptionClassName, msg);
}

FlintError throwClassNotFoundException(FlintExecution &execution, const char *msg) {
    return throwThrowable(execution, classNotFoundExceptionClassName, msg);
}

FlintError throwClassNotFoundException(FlintExecution &execution, FlintJavaString *str) {
    return throwThrowable(execution, classNotFoundExceptionClassName, str);
}

FlintError throwIllegalArgumentException(FlintExecution &execution, const char *msg) {
    return throwThrowable(execution, illegalArgumentExceptionClassName, msg);
}

FlintError throwCloneNotSupportedException(FlintExecution &execution, const char *msg) {
    return throwThrowable(execution, cloneNotSupportedExceptionClassName, msg);
}

FlintError throwNegativeArraySizeException(FlintExecution &execution, const char *msg) {
    return throwThrowable(execution, negativeArraySizeExceptionClassName, msg);
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

    return throwThrowable(execution, arrayIndexOutOfBoundsExceptionClassName, str);
}

FlintError throwUnsupportedOperationException(FlintExecution &execution, const char *msg) {
    return throwThrowable(execution, unsupportedOperationExceptionClassName, msg);
}

FlintError throwUnsatisfiedLinkErrorException(FlintExecution &execution, const char *msg) {
    return throwThrowable(execution, unsatisfiedLinkErrorClassName, msg);
}

FlintError throwNoSuchMethodError(FlintExecution &execution, FlintConstMethod &constMethod) {
    uint32_t strLen = strlen("Could not find the method ") + constMethod.className.length + strlen(".") + constMethod.nameAndType.name.length;
    FlintJavaString *str;
    RETURN_IF_ERR(execution.flint.newString(strLen, 0, str));

    uint32_t i = 0;
    char *txt = str->getText();
    i = sprint(txt, i, "Could not find the method ");
    i = sprint(txt, i, constMethod.className.text, '/', '.');
    i = sprint(txt, i, ".");
    i = sprint(txt, i, constMethod.nameAndType.name.text);

    return throwThrowable(execution, noSuchMethodErrorExceptionClassName, str);
}

FlintError throwNoSuchMethodError(FlintExecution &execution, FlintClassData &classData) {
    uint32_t strLen = strlen("Could not find the method ") + classData.thisClass->length + strlen(".") + staticConstructorName.length;
    FlintJavaString *str;
    RETURN_IF_ERR(execution.flint.newString(strLen, 0, str));

    uint32_t i = 0;
    char *txt = str->getText();
    i = sprint(txt, i, "Could not find the method ");
    i = sprint(txt, i, classData.thisClass->text, '/', '.');
    i = sprint(txt, i, ".");
    i = sprint(txt, i, staticConstructorName.text);

    return throwThrowable(execution, noSuchMethodErrorExceptionClassName, str);
}

FlintError throwNoSuchFieldError(FlintExecution &execution, FlintConstField &constField) {
    uint32_t strLen = strlen("Could not find the field ") + constField.className.length + strlen(".") + constField.nameAndType.name.length;
    FlintJavaString *str;
    RETURN_IF_ERR(execution.flint.newString(strLen, 0, str));

    uint32_t i = 0;
    char *txt = str->getText();
    i = sprint(txt, i, "Could not find the field ");
    i = sprint(txt, i,  constField.className.text, '/', '.');
    i = sprint(txt, i, ".");
    i = sprint(txt, i, constField.nameAndType.name.text);

    return throwThrowable(execution, noSuchFieldErrorExceptionClassName, str);
}

FlintError throwNoSuchFieldError(FlintExecution &execution, FlintConstUtf8 &className, const char *fieldName) {
    uint32_t strLen = strlen("Could not find the field ") + className.length + strlen(".") + strlen(fieldName);
    FlintJavaString *str;
    RETURN_IF_ERR(execution.flint.newString(strLen, 0, str));

    uint32_t i = 0;
    char *txt = str->getText();
    i = sprint(txt, i, "Could not find the field ");
    i = sprint(txt, i,  className.text, '/', '.');
    i = sprint(txt, i, ".");
    i = sprint(txt, i, fieldName);

    return throwThrowable(execution, noSuchFieldErrorExceptionClassName, str);
}
