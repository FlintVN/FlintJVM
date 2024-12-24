
#include "flint_throwable.h"
#include "flint_const_name.h"
#include "flint_fields_data.h"

FlintString *FlintThrowable::getDetailMessage(void) const {
    return (FlintString *)getFields().getFieldObject(*(const FlintConstUtf8 *)"\x0D\x00\xE6\x62""detailMessage").object;
}

void FlintThrowable::setDetailMessage(FlintString &strObj) {
    getFields().getFieldObject(*(const FlintConstUtf8 *)"\x0D\x00\xE6\x62""detailMessage").object = &strObj;
}
