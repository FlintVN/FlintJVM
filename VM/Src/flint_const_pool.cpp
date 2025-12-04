
#include <string.h>
#include "flint_std.h"
#include "flint_const_pool.h"

bool ConstNameAndType::operator==(ConstNameAndType &another) const {
    if(
        hash == another.hash &&
        strcmp(name, another.name) == 0 &&
        strcmp(desc, another.desc) == 0
    ) {
        return true;
    }
    return false;
}

bool ConstNameAndType::operator!=(ConstNameAndType &another) const {
    return !(*this == another);
}

ConstField::ConstField(const char *className, ConstNameAndType *nameAndType) :
className(className), loader(NULL), nameAndType(nameAndType), fieldIndex(0) {

}

ConstMethod::ConstMethod(const char *className, ConstNameAndType *nameAndType) :
className(className), nameAndType(nameAndType), methodInfo(NULL) {
    argc = GetArgSlotCount(nameAndType->desc);
}

uint8_t ConstMethod::getArgc(void) const {
    return argc;
}
