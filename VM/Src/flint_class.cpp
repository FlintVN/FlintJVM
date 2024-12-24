
#include "flint_class.h"
#include "flint_const_name.h"
#include "flint_fields_data.h"

FlintString &FlintClass::getName(void) const {
    return *(FlintString *)getFields().getFieldObject(*(const FlintConstUtf8 *)"\x04\x00\x5E\x56""name").object;
}

void FlintClass::setName(FlintString *name) {
    getFields().getFieldObject(*(const FlintConstUtf8 *)"\x04\x00\x5E\x56""name").object = name;
}

FlintConstClass::FlintConstClass(FlintClass &flintClass) : flintClass(flintClass) {

}
