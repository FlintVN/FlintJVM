
#include "flint.h"
#include "flint_field_info.h"

FlintFieldInfo::FlintFieldInfo(FlintClassLoader &classLoader, FlintFieldAccessFlag accessFlag, uint16_t nameIndex, uint16_t descIndex) :
accessFlag(accessFlag), classLoader(classLoader), nameIndex(nameIndex), descIndex(descIndex) {

}

FlintConstUtf8 &FlintFieldInfo::getName(void) const {
    return classLoader.getConstUtf8(nameIndex);
}

FlintConstUtf8 &FlintFieldInfo::getDescriptor(void) const {
    return classLoader.getConstUtf8(descIndex);
}
