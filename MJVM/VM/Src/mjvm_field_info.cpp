
#include <string.h>
#include "mjvm.h"
#include "mjvm_field_info.h"

FieldInfo::FieldInfo(const ClassLoader &classLoader, FieldAccessFlag accessFlag, const ConstUtf8 &name, const ConstUtf8 &descriptor) :
classLoader(classLoader), accessFlag(accessFlag), name(name), descriptor(descriptor) {

}
