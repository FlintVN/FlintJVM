
#include "mjvm_throwable.h"
#include "mjvm_const_name.h"
#include "mjvm_fields_data.h"

MjvmString &MjvmThrowable::getDetailMessage(void) const {
    return *(MjvmString *)((FieldsData *)data)->getFieldObject(*(ConstNameAndType *)exceptionDetailMessageFieldName).object;
}
