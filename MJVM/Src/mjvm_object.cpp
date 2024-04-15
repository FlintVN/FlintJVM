
#include "mjvm_object.h"
#include "mjvm_const_name.h"
#include "mjvm_fields_data.h"

MjvmObject::MjvmObject(uint32_t size, const ConstUtf8 &type, uint8_t dimensions) :
size(size), prot(0), type(type), dimensions(dimensions), monitorCount(0), ownId(0) {

}

uint8_t MjvmObject::parseTypeSize(void) const {
    switch(this->type.getText()[0]) {
        case 'Z':
        case 'B':
            return 1;
        case 'C':
        case 'S':
            return 2;
        case 'J':
        case 'D':
            return 8;
        default:
            return 4;
    }
}

void MjvmObject::setProtected(void) {
    prot = 1;
}

bool MjvmObject::isProtected(void) const {
    return prot;
}

MjvmObject *MjvmObjectNode::getMjvmObject(void) const {
    return (MjvmObject *)data;
}
