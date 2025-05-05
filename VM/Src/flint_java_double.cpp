
#include "flint_java_double.h"
#include "flint_const_name_base.h"
#include "flint_fields_data.h"

double FlintJavaDouble::getValue(void) const {
    return *(double *)&getFields().getFieldData64ByIndex(0)->value;
}

void FlintJavaDouble::setValue(double value) {
    getFields().getFieldData64ByIndex(0)->value = *(double *)&value;
}
