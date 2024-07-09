
#include "mjvm.h"
#include "mjvm_field_info.h"

MjvmFieldInfo::MjvmFieldInfo(MjvmClassLoader &classLoader, MjvmFieldAccessFlag accessFlag, MjvmConstUtf8 &name, MjvmConstUtf8 &descriptor) :
accessFlag(accessFlag), classLoader(classLoader), name(name), descriptor(descriptor) {

}
