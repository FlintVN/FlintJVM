
#include "flint_throwable.h"
#include "flint_const_name.h"
#include "flint_fields_data.h"

FlintString &FlintThrowable::getDetailMessage(void) const {
    return *(FlintString *)((FlintFieldsData *)data)->getFieldObject(*(FlintConstNameAndType *)exceptionDetailMessageFieldName).object;
}
