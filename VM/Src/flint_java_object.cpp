
#include "flint_java_object.h"
#include "flint_const_name_base.h"
#include "flint_fields_data.h"

static const uint8_t primitiveTypeSize[9] = {
    sizeof(int8_t), sizeof(int16_t), sizeof(float), sizeof(double),
    sizeof(int8_t), sizeof(int16_t), sizeof(int32_t), sizeof(int64_t),
    0
};

uint8_t FlintJavaObject::getPrimitiveTypeSize(uint8_t atype) {
    return primitiveTypeSize[atype - 4];
}

uint8_t FlintJavaObject::convertToAType(char type) {
    switch(type) {
        case 'Z':
            return 4;
        case 'C':
            return 5;
        case 'F':
            return 6;
        case 'D':
            return 7;
        case 'B':
            return 8;
        case 'S':
            return 9;
        case 'I':
            return 10;
        case 'J':
            return 11;
        case 'V':
            return 12;
    }
    return 0;
}

uint8_t FlintJavaObject::isPrimType(FlintConstUtf8 &type) {
    if(type.length == 1)
        return convertToAType(type.text[0]);
    return 0;
}

FlintJavaObject::FlintJavaObject(uint32_t size, FlintConstUtf8 &type, uint8_t dimensions) :
size(size), prot(0x02), type((FlintConstUtf8 &)type), dimensions(dimensions), monitorCount(0), ownId(0) {

}

uint8_t FlintJavaObject::parseTypeSize(void) const {
    switch(this->type.text[0]) {
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

FlintFieldsData &FlintJavaObject::getFields(void) const {
    return *(FlintFieldsData *)data;
}

void FlintJavaObject::setProtected(void) {
    prot = 1;
}

void FlintJavaObject::clearProtected(void) {
    prot = 0;
}

uint8_t FlintJavaObject::getProtected(void) const {
    return prot;
}
