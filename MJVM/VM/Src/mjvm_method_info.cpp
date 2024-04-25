
#include <string.h>
#include "mjvm.h"
#include "mjvm_method_info.h"
#include "mjvm_native_class.h"

static NativeMethodPtr findNaiveMathod(const MethodInfo &methodInfo) {
    const ConstUtf8 &className = methodInfo.classLoader.getThisClass();
    for(uint32_t i = 0; i < NATIVE_CLASS_COUNT; i++) {
        if(NATIVE_CLASS_LIST[i]->className == className) {
            for(uint32_t k = 0; k < NATIVE_CLASS_LIST[i]->methodCount; k++) {
                if(
                    NATIVE_CLASS_LIST[i]->methods[k].name == methodInfo.name &&
                    NATIVE_CLASS_LIST[i]->methods[k].descriptor == methodInfo.descriptor
                ) {
                    return NATIVE_CLASS_LIST[i]->methods[k].nativeMathod;
                }
            }
            break;
        }
    }
    throw "can't find the native method";
}

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
        if(attributes[i]->attributeType == type) {
            if(type != ATTRIBUTE_NATIVE)
                return *attributes[i];
            else {
                AttributeNative *attrNative = (AttributeNative *)attributes[i];
                if(attrNative->nativeMethod == 0)
                    *(void **)&attrNative->nativeMethod = (void *)findNaiveMathod(*this);
                return *attrNative;
            }
        }
    }
    throw "can't find the attribute";
}

const AttributeCode &MethodInfo::getAttributeCode(void) const {
    return *(AttributeCode *)&getAttribute(ATTRIBUTE_CODE);
}

const AttributeNative &MethodInfo::getAttributeNative(void) const {
    return *(AttributeNative *)&getAttribute(ATTRIBUTE_NATIVE);
}

ParamInfo MethodInfo::parseParamInfo(void) const {
    return ::parseParamInfo(descriptor);
}

MethodInfo::~MethodInfo(void) {
    if(attributes) {
        for(uint16_t i = 0; i < attributesCount; i++) {
            attributes[i]->~AttributeInfo();
            Mjvm::free((void *)attributes[i]);
        }
        Mjvm::free((void *)attributes);
    }
}
