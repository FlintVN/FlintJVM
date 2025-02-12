
#include "flint_java_throwable.h"
#include "flint_const_name.h"
#include "flint_fields_data.h"

FlintJavaString *FlintJavaThrowable::getDetailMessage(void) const {
    return (FlintJavaString *)getFields().getFieldObject(*(const FlintConstUtf8 *)"\x0D\x00\xE6\x62""detailMessage").object;
}

void FlintJavaThrowable::setDetailMessage(FlintJavaString &strObj) {
    getFields().getFieldObject(*(const FlintConstUtf8 *)"\x0D\x00\xE6\x62""detailMessage").object = &strObj;
}
