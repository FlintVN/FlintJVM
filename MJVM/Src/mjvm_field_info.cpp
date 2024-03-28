
#include <string.h>
#include "mjvm_heap.h"
#include "mjvm_field_info.h"

FieldInfo::FieldInfo(FieldAccessFlag accessFlag, const ConstUtf8 &name, const ConstUtf8 &descriptor) :
accessFlag(accessFlag), name(name), descriptor(descriptor), attributesCount(0), attributes(0) {

}

void FieldInfo::setAttributes(AttributeInfo **attributes, uint16_t length) {
    this->attributes = (const AttributeInfo **)attributes;
    *(uint16_t *)&attributesCount = length;
}

const AttributeInfo &FieldInfo::getAttribute(uint16_t index) const {
    if(index < attributesCount)
        return *attributes[index];
    throw "index for const field attribute is invalid";
}

const AttributeInfo &FieldInfo::getAttribute(AttributeType type) const {
    for(uint16_t i = 0; i < attributesCount; i++) {
        if(attributes[i]->attributeType == type)
            return *attributes[i];
    }
    throw "can't find the attribute";
}

FieldInfo::~FieldInfo(void) {
    if(attributes) {
        for(uint16_t i = 0; i < attributesCount; i++)
            attributes[i]->~AttributeInfo();
        MJVM_Free((void *)attributes);
    }
}
