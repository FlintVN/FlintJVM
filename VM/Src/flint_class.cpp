
#include "flint_class.h"
#include "flint_const_name.h"
#include "flint_fields_data.h"

FlintString &FlintClass::getName(void) const {
    return *(FlintString *)((FlintFieldsData *)data)->getFieldObject(*(FlintConstNameAndType *)stringNameFieldName).object;
}

FlintConstClass::FlintConstClass(FlintClass &flintClass) : flintClass(flintClass) {

}
