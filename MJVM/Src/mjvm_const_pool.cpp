
#include <string.h>
#include "mjvm_const_pool.h"

ConstUtf8::ConstUtf8(uint16_t length) : length(length) {

}

const char *ConstUtf8::getText(void) const {
    return (const char *)text;
}

ConstNameAndType::ConstNameAndType(const ConstUtf8 &name, const ConstUtf8 &descriptor) :
name(name),
descriptor(descriptor) {
    
}

ConstField::ConstField(const ConstUtf8 &className, const ConstNameAndType &nameAndType) :
className(className),
nameAndType(nameAndType) {

}
