
#include "flint_java_class.h"
#include "flint_const_name.h"
#include "flint_fields_data.h"

FlintJavaString &FlintJavaClass::getName(void) const {
    return *(FlintJavaString *)getFields().getFieldObject(*(const FlintConstUtf8 *)"\x04\x00\x5E\x56""name").object;
}

void FlintJavaClass::setName(FlintJavaString *name) {
    getFields().getFieldObject(*(const FlintConstUtf8 *)"\x04\x00\x5E\x56""name").object = name;
}

FlintConstClass::FlintConstClass(FlintJavaClass &flintClass) : flintClass(flintClass) {

}
