
#include "flint_fields_data.h"
#include "flint_java_throwable.h"

JString *JThrowable::getDetailMessage(void) const {
    return (JString *)getFields()->getFieldObjByIndex(0)->value;
}

void JThrowable::setDetailMessage(JString *strObj) {
    getFields()->getFieldObjByIndex(0)->value = strObj;
}

JThrowable *JThrowable::getCause(void) const {
    return (JThrowable *)getFields()->getFieldObjByIndex(1)->value;
}

void JThrowable::setCause(JThrowable *cause) {
    getFields()->getFieldObjByIndex(1)->value = cause;
}
