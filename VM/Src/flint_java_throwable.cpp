
#include "flint_java_throwable.h"
#include "flint_const_name_base.h"
#include "flint_fields_data.h"

FlintJavaString *FlintJavaThrowable::getDetailMessage(void) const {
    return (FlintJavaString *)getFields().getFieldObjectByIndex(0)->object;
}

void FlintJavaThrowable::setDetailMessage(FlintJavaString &strObj) {
    getFields().getFieldObjectByIndex(0)->object = &strObj;
}
