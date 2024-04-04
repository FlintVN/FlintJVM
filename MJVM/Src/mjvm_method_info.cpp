
#include <string.h>
#include "mjvm_heap.h"
#include "mjvm_method_info.h"

MethodInfo::MethodInfo(const ClassLoader &classLoader, MethodAccessFlag accessFlag, const ConstUtf8 &name, const ConstUtf8 &descriptor) :
classLoader(classLoader), accessFlag(accessFlag), name(name), descriptor(descriptor), attributesCount(0), attributes(0) {

}

void MethodInfo::setAttributes(AttributeInfo **attributes, uint16_t length) {
    this->attributes = (const AttributeInfo **)attributes;
    *(uint16_t *)&attributesCount = length;
}

const AttributeInfo &MethodInfo::getAttribute(uint16_t index) const {
    if(index < attributesCount)
        return *attributes[index];
    throw "index for const field attribute is invalid";
}

const AttributeInfo &MethodInfo::getAttribute(AttributeType type) const {
    for(uint16_t i = 0; i < attributesCount; i++) {
        if(attributes[i]->attributeType == type)
            return *attributes[i];
    }
    throw "can't find the attribute";
}

const AttributeCode &MethodInfo::getAttributeCode(void) const {
    return *(AttributeCode *)&getAttribute(ATTRIBUTE_CODE);
}

ParamInfo MethodInfo::parseParamInfo(void) const {
    const char *text = descriptor.getText();
    ParamInfo retVal = {0, 0};
    if(*text != '(')
        throw "the descriptor is not a description of the method";
    text++;
    while(*text) {
        if(*text == ')') {
            retVal.retType = text[1];
            return retVal;
        }
        else {
            retVal.argc += (*text == 'J' || *text == 'D') ? 2 : 1;
            if(*text++ == 'L') {
                while(*text) {
                    if(*text == ')') {
                        retVal.retType = text[1];
                        return retVal;
                    }
                    else if(*text == ';') {
                        text++;
                        break;
                    }
                    text++;
                }
            }
        }
    }
    throw "descriptor is invalid";
}

MethodInfo::~MethodInfo(void) {
    if(attributes) {
        for(uint16_t i = 0; i < attributesCount; i++) {
            attributes[i]->~AttributeInfo();
            MjvmHeap::free((void *)attributes[i]);
        }
        MjvmHeap::free((void *)attributes);
    }
}
