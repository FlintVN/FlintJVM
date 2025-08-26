
#include "flint_java_throwable.h"
#include "flint_const_name_base.h"
#include "flint_fields_data.h"

JString *JThrowable::getDetailMessage(void) const {
    return (JString *)getFields().getFieldObjectByIndex(0)->object;
}

void JThrowable::setDetailMessage(JString *strObj) {
    getFields().getFieldObjectByIndex(0)->object = strObj;
}
