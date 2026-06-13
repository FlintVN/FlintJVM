
#include "flint_fields_data.h"
#include "flint_java_throwable.h"

JString *JThrowable::getDetailMessage(void) const {
    return (JString *)getFieldByIndex(0)->getObj();
}

void JThrowable::setDetailMessage(JString *strObj) {
    getFieldByIndex(0)->setObj(strObj);
}

JThrowable *JThrowable::getCause(void) const {
    return (JThrowable *)getFieldByIndex(1)->getObj();
}

void JThrowable::setCause(JThrowable *cause) {
    getFieldByIndex(1)->setObj(cause);
}
