
#include "flint_java_integer.h"
#include "flint_const_name_base.h"
#include "flint_fields_data.h"

int32_t FlintJavaInteger::getValue(void) const {
    return getFields().getFieldData32ByIndex(0)->value;
}

void FlintJavaInteger::setValue(int32_t value) {
    getFields().getFieldData32ByIndex(0)->value = value;
}
