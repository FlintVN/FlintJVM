
#include "mjvm_class.h"
#include "mjvm_const_name.h"
#include "mjvm_fields_data.h"

MjvmString &MjvmClass::getName(void) const {
    return *(MjvmString *)((FieldsData *)data)->getFieldObject(*(ConstNameAndType *)stringNameFieldName).object;
}

MjvmConstClass::MjvmConstClass(MjvmClass &mjvmClass) : mjvmClass(mjvmClass) {

}
 