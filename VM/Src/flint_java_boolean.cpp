
#include "flint_java_boolean.h"
#include "flint_const_name_base.h"
#include "flint_fields_data.h"

bool FlintJavaBoolean::getValue(void) const {
    return (bool)getFields().getFieldData32ByIndex(0).value;
}

void FlintJavaBoolean::setValue(bool value) {
    getFields().getFieldData32ByIndex(0).value = value;
}
