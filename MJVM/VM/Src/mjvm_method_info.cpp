
#include <string.h>
#include "mjvm.h"
#include "mjvm_method_info.h"
#include "mjvm_native_class.h"

static NativeMethodPtr findNativeMathod(const MethodInfo &methodInfo) {
    ConstUtf8 &className = methodInfo.classLoader.getThisClass();
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

MethodInfo::MethodInfo(ClassLoader &classLoader, MethodAccessFlag accessFlag, ConstUtf8 &name, ConstUtf8 &descriptor) :
accessFlag(accessFlag), classLoader(classLoader), name(name), descriptor(descriptor), attributes(0) {

}

void MethodInfo::addAttribute(AttributeInfo *attribute) {
    attribute->next = this->attributes;
    this->attributes = attribute;
}

AttributeInfo &MethodInfo::getAttribute(AttributeType type) const {
    for(AttributeInfo *node = attributes; node != 0;) {
        if(node->attributeType == type) {
            if(type != ATTRIBUTE_NATIVE)
                return *node;
            else {
                AttributeNative *attrNative = (AttributeNative *)node;
                if(attrNative->nativeMethod == 0)
                    *(void **)&attrNative->nativeMethod = (void *)findNativeMathod(*this);
                return *attrNative;
            }
        }
    }
    throw "can't find the attribute";
}

AttributeCode &MethodInfo::getAttributeCode(void) const {
    return *(AttributeCode *)&getAttribute(ATTRIBUTE_CODE);
}

AttributeNative &MethodInfo::getAttributeNative(void) const {
    return *(AttributeNative *)&getAttribute(ATTRIBUTE_NATIVE);
}

ParamInfo MethodInfo::parseParamInfo(void) const {
    return ::parseParamInfo(descriptor);
}

MethodInfo::~MethodInfo(void) {
    for(AttributeInfo *node = attributes; node != 0;) {
        AttributeInfo *next = node->next;
        node->~AttributeInfo();
        Mjvm::free(node);
        node = next;
    }
}
