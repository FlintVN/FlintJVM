
#include "flint.h"
#include "flint_common.h"
#include "flint_field_info.h"

FieldInfo::FieldInfo(FieldAccessFlag accessFlag, const char *name, const char *desc) :
accessFlag(accessFlag), name(name), desc(desc), hash((Hash(name) & 0xFFFF) | (Hash(desc) << 16)) {

}

void FieldInfo::free(FieldInfo *fieldInfo) {
    Flint::free(fieldInfo);
}
