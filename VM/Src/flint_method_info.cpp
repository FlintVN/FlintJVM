
#include <string.h>
#include "flint.h"
#include "flint_method_info.h"
#include "flint_native_class.h"

static FlintNativeMethodPtr findNativeMathod(const FlintMethodInfo &methodInfo) {
    FlintConstUtf8 &className = methodInfo.classLoader.getThisClass();
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

FlintMethodInfo::FlintMethodInfo(FlintClassLoader &classLoader, FlintMethodAccessFlag accessFlag, FlintConstUtf8 &name, FlintConstUtf8 &descriptor) :
accessFlag(accessFlag), classLoader(classLoader), name(name), descriptor(descriptor), attributes(0) {

}

void FlintMethodInfo::addAttribute(FlintAttribute *attribute) {
    attribute->next = this->attributes;
    this->attributes = attribute;
}

FlintAttribute &FlintMethodInfo::getAttribute(FlintAttributeType type) const {
    for(FlintAttribute *node = attributes; node != 0;) {
        if(node->attributeType == type) {
            if(type != ATTRIBUTE_NATIVE)
                return *node;
            else {
                FlintNativeAttribute *attrNative = (FlintNativeAttribute *)node;
                if(attrNative->nativeMethod == 0)
                    *(void **)&attrNative->nativeMethod = (void *)findNativeMathod(*this);
                return *attrNative;
            }
        }
    }
    throw "can't find the attribute";
}

FlintCodeAttribute &FlintMethodInfo::getAttributeCode(void) const {
    for(FlintAttribute *node = attributes; node != 0;) {
        if(node->attributeType == ATTRIBUTE_CODE)
            return *(FlintCodeAttribute *)node;
    }
    throw "can't find the code attribute";
}

FlintNativeAttribute &FlintMethodInfo::getAttributeNative(void) const {
    for(FlintAttribute *node = attributes; node != 0;) {
        if(node->attributeType == ATTRIBUTE_NATIVE) {
            FlintNativeAttribute *attrNative = (FlintNativeAttribute *)node;
            if(attrNative->nativeMethod == 0)
                *(void **)&attrNative->nativeMethod = (void *)findNativeMathod(*this);
            return *(FlintNativeAttribute *)attrNative;
        }
    }
    throw "can't find the native attribute";
}

FlintMethodInfo::~FlintMethodInfo(void) {
    for(FlintAttribute *node = attributes; node != 0;) {
        FlintAttribute *next = node->next;
        node->~FlintAttribute();
        Flint::free(node);
        node = next;
    }
}
