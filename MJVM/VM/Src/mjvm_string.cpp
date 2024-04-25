
#include "mjvm_string.h"
#include "mjvm_const_name.h"
#include "mjvm_fields_data.h"

const char *MjvmString::getText(void) const {
    MjvmObject *byteArray = ((FieldsData *)data)->getFieldObject(*(ConstNameAndType *)stringValueFieldName).object;
    return (const char *)((MjvmString *)byteArray)->data;
}

uint32_t MjvmString::getLength(void) const {
    MjvmString *byteArray = (MjvmString *)((FieldsData *)data)->getFieldObject(*(ConstNameAndType *)stringValueFieldName).object;
    return byteArray->size / sizeof(int8_t);
}
