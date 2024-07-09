
#include "mjvm_class.h"
#include "mjvm_const_name.h"
#include "mjvm_fields_data.h"

MjvmString &MjvmClass::getName(void) const {
    return *(MjvmString *)((MjvmFieldsData *)data)->getFieldObject(*(MjvmConstNameAndType *)stringNameFieldName).object;
}

MjvmConstClass::MjvmConstClass(MjvmClass &mjvmClass) : mjvmClass(mjvmClass) {

}
