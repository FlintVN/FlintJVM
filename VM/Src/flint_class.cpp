
#include "flint_class.h"
#include "flint_const_name.h"
#include "flint_fields_data.h"

static const uint32_t stringNameFieldName[] = {
    (uint32_t)"\x04\x00\xA1\x01""name",                 /* field name */
    (uint32_t)"\x12\x00\xA0\x06""Ljava/lang/String;"    /* field type */
};

FlintString &FlintClass::getName(void) const {
    return *(FlintString *)((FlintFieldsData *)data)->getFieldObject(*(FlintConstNameAndType *)stringNameFieldName).object;
}

void FlintClass::setName(FlintString *name) {
    ((FlintFieldsData *)data)->getFieldObject(*(FlintConstNameAndType *)stringNameFieldName).object = name;
}

FlintConstClass::FlintConstClass(FlintClass &flintClass) : flintClass(flintClass) {

}
