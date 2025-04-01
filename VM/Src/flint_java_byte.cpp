
#include "flint_java_byte.h"
#include "flint_const_name_base.h"
#include "flint_fields_data.h"

int8_t FlintJavaByte::getValue(void) const {
    return (int8_t)getFields().getFieldData32ByIndex(0).value;
}

void FlintJavaByte::setValue(int8_t value) {
    getFields().getFieldData32ByIndex(0).value = value;
}
