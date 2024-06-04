
#include "mjvm.h"
#include "mjvm_field_info.h"

FieldInfo::FieldInfo(ClassLoader &classLoader, FieldAccessFlag accessFlag, ConstUtf8 &name, ConstUtf8 &descriptor) :
accessFlag(accessFlag), classLoader(classLoader), name(name), descriptor(descriptor) {

}
