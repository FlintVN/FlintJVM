
#include "flint_java_short.h"
#include "flint_const_name.h"
#include "flint_fields_data.h"

int16_t FlintJavaShort::getValue(void) const {
    return (int16_t)getFields().getFieldData32ByIndex(0).value;
}

void FlintJavaShort::setValue(int16_t value) {
    getFields().getFieldData32ByIndex(0).value = value;
}
