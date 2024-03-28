
#include <string.h>
#include "mjvm_const_pool.h"

ConstNameAndType::ConstNameAndType(const ConstUtf8 &name, const ConstUtf8 &descriptor) :
name(name),
descriptor(descriptor) {
    
}

ConstField::ConstField(const ConstUtf8 &className, const ConstNameAndType &nameAndType) :
className(className),
nameAndType(nameAndType) {

}
