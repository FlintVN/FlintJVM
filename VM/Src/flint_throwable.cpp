
#include "flint_throwable.h"
#include "flint_const_name.h"
#include "flint_fields_data.h"

static const uint32_t exceptionDetailMessageFieldName[] = {
    (uint32_t)"\x0D\x00\xE6\x62""detailMessage",        /* field name */
    (uint32_t)"\x12\x00\xC0\x24""Ljava/lang/String;"    /* field type */
};

FlintString *FlintThrowable::getDetailMessage(void) const {
    return (FlintString *)getFields().getFieldObject(*(FlintConstNameAndType *)exceptionDetailMessageFieldName).object;
}

void FlintThrowable::setDetailMessage(FlintString &strObj) {
    getFields().getFieldObject(*(FlintConstNameAndType *)exceptionDetailMessageFieldName).object = &strObj;
}
