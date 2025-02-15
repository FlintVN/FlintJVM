
#include "flint_java_char.h"
#include "flint_const_name.h"
#include "flint_fields_data.h"

uint16_t FlintJavaChar::getValue(void) const {
    return (uint16_t)getFields().getFieldData32ByIndex(0).value;
}

void FlintJavaChar::setValue(uint16_t value) {
    getFields().getFieldData32ByIndex(0).value = value;
}
