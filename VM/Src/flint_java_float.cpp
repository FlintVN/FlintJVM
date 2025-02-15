
#include "flint_java_float.h"
#include "flint_const_name.h"
#include "flint_fields_data.h"

float FlintJavaFloat::getValue(void) const {
    return *(float *)&getFields().getFieldData32ByIndex(0).value;
}

void FlintJavaFloat::setValue(float value) {
    getFields().getFieldData32ByIndex(0).value = *(float *)&value;
}
