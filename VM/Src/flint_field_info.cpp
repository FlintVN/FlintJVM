
#include "flint.h"
#include "flint_field_info.h"

FlintFieldInfo::FlintFieldInfo(FlintClassLoader &classLoader, FlintFieldAccessFlag accessFlag, const FlintConstUtf8 &name, const FlintConstUtf8 &descriptor) :
accessFlag(accessFlag), classLoader(classLoader), name((FlintConstUtf8 &)name), descriptor((FlintConstUtf8 &)descriptor) {

}
