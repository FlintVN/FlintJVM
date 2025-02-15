
#include "flint_java_long.h"
#include "flint_const_name.h"
#include "flint_fields_data.h"

int64_t FlintJavaLong::getValue(void) const {
    return getFields().getFieldData64ByIndex(0).value;
}

void FlintJavaLong::setValue(int64_t value) {
    getFields().getFieldData64ByIndex(0).value = value;
}
