
#include "flint.h"
#include "flint_field_info.h"

FlintFieldInfo::FlintFieldInfo(FlintClassLoader &classLoader, FlintFieldAccessFlag accessFlag, FlintConstUtf8 &name, FlintConstUtf8 &descriptor) :
accessFlag(accessFlag), classLoader(classLoader), name(name), descriptor(descriptor) {

}
