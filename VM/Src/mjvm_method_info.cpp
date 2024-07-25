
#include <string.h>
#include "mjvm.h"
#include "mjvm_method_info.h"
#include "mjvm_native_class.h"

static MjvmNativeMethodPtr findNativeMathod(const MjvmMethodInfo &methodInfo) {
    MjvmConstUtf8 &className = methodInfo.classLoader.getThisClass();
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

MjvmMethodInfo::MjvmMethodInfo(MjvmClassLoader &classLoader, MjvmMethodAccessFlag accessFlag, MjvmConstUtf8 &name, MjvmConstUtf8 &descriptor) :
accessFlag(accessFlag), classLoader(classLoader), name(name), descriptor(descriptor), attributes(0) {

}

void MjvmMethodInfo::addAttribute(MjvmAttribute *attribute) {
    attribute->next = this->attributes;
    this->attributes = attribute;
}

MjvmAttribute &MjvmMethodInfo::getAttribute(MjvmAttributeType type) const {
    for(MjvmAttribute *node = attributes; node != 0;) {
        if(node->attributeType == type) {
            if(type != ATTRIBUTE_NATIVE)
                return *node;
            else {
                MjvmNativeAttribute *attrNative = (MjvmNativeAttribute *)node;
                if(attrNative->nativeMethod == 0)
                    *(void **)&attrNative->nativeMethod = (void *)findNativeMathod(*this);
                return *attrNative;
            }
        }
    }
    throw "can't find the attribute";
}

MjvmCodeAttribute &MjvmMethodInfo::getAttributeCode(void) const {
    for(MjvmAttribute *node = attributes; node != 0;) {
        if(node->attributeType == ATTRIBUTE_CODE)
            return *(MjvmCodeAttribute *)node;
    }
    throw "can't find the code attribute";
}

MjvmNativeAttribute &MjvmMethodInfo::getAttributeNative(void) const {
    for(MjvmAttribute *node = attributes; node != 0;) {
        if(node->attributeType == ATTRIBUTE_NATIVE) {
            MjvmNativeAttribute *attrNative = (MjvmNativeAttribute *)node;
            if(attrNative->nativeMethod == 0)
                *(void **)&attrNative->nativeMethod = (void *)findNativeMathod(*this);
            return *(MjvmNativeAttribute *)attrNative;
        }
    }
    throw "can't find the native attribute";
}

MjvmMethodInfo::~MjvmMethodInfo(void) {
    for(MjvmAttribute *node = attributes; node != 0;) {
        MjvmAttribute *next = node->next;
        node->~MjvmAttribute();
        Mjvm::free(node);
        node = next;
    }
}
