
#include "flint_java_character.h"
#include "flint_const_name_base.h"
#include "flint_fields_data.h"

uint16_t FlintJavaCharacter::getValue(void) const {
    return (uint16_t)getFields().getFieldData32ByIndex(0).value;
}

void FlintJavaCharacter::setValue(uint16_t value) {
    getFields().getFieldData32ByIndex(0).value = value;
}
