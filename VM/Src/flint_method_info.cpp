
#include <string.h>
#include "flint.h"
#include "flint_system_api.h"
#include "flint_method_info.h"
#include "flint_native_class.h"

static FlintNativeMethodPtr findNativeMethod(const FlintMethodInfo &methodInfo) {
    FlintConstUtf8 &className = methodInfo.classLoader.getThisClass();
    for(uint32_t i = 0; i < LENGTH(BASE_NATIVE_CLASS_LIST); i++) {
        if(BASE_NATIVE_CLASS_LIST[i]->className == className) {
            for(uint32_t k = 0; k < BASE_NATIVE_CLASS_LIST[i]->methodCount; k++) {
                if(
                    BASE_NATIVE_CLASS_LIST[i]->methods[k].name == methodInfo.name &&
                    BASE_NATIVE_CLASS_LIST[i]->methods[k].descriptor == methodInfo.descriptor
                ) {
                    return BASE_NATIVE_CLASS_LIST[i]->methods[k].nativeMathod;
                }
            }
            break;
        }
    }
    FlintNativeMethodPtr nativeMethod = FlintAPI::System::findNativeMethod(methodInfo);
    if(nativeMethod)
        return nativeMethod;
    throw (FlintFindNativeError *)"can't find the native method";
}

FlintMethodInfo::FlintMethodInfo(FlintClassLoader &classLoader, FlintMethodAccessFlag accessFlag, const FlintConstUtf8 &name, const FlintConstUtf8 &descriptor) :
accessFlag((&name != &staticConstructorName) ? accessFlag : (FlintMethodAccessFlag)(accessFlag | METHOD_SYNCHRONIZED)),
classLoader(classLoader), name((FlintConstUtf8 &)name), descriptor((FlintConstUtf8 &)descriptor), attributes(0) {

}

void FlintMethodInfo::addAttribute(FlintAttribute *attribute) {
    attribute->next = this->attributes;
    this->attributes = attribute;
}

FlintAttribute &FlintMethodInfo::getAttribute(FlintAttributeType type) const {
    for(FlintAttribute *node = attributes; node != 0; node = node->next) {
        if(node->attributeType == type) {
            if(type != ATTRIBUTE_NATIVE)
                return *node;
            else {
                FlintNativeAttribute *attrNative = (FlintNativeAttribute *)node;
                if(attrNative->nativeMethod == 0)
                    *(void **)&attrNative->nativeMethod = (void *)findNativeMethod(*this);
                return *attrNative;
            }
        }
    }
    throw "can't find the attribute";
}

FlintCodeAttribute &FlintMethodInfo::getAttributeCode(void) const {
    for(FlintAttribute *node = attributes; node != 0; node = node->next) {
        if(node->attributeType == ATTRIBUTE_CODE)
            return *(FlintCodeAttribute *)node;
    }
    throw "can't find the code attribute";
}

FlintNativeAttribute &FlintMethodInfo::getAttributeNative(void) const {
    for(FlintAttribute *node = attributes; node != 0; node = node->next) {
        if(node->attributeType == ATTRIBUTE_NATIVE) {
            FlintNativeAttribute *attrNative = (FlintNativeAttribute *)node;
            if(attrNative->nativeMethod == 0)
                *(void **)&attrNative->nativeMethod = (void *)findNativeMethod(*this);
            return *(FlintNativeAttribute *)attrNative;
        }
    }
    throw "can't find the native attribute";
}

bool FlintMethodInfo::hasAttributeCode(void) const {
    for(FlintAttribute *node = attributes; node != 0; node = node->next) {
        if(node->attributeType == ATTRIBUTE_CODE)
            return true;
    }
    return false;
}

bool FlintMethodInfo::isStaticCtor(void) {
    return &name == &staticConstructorName;
}

FlintMethodInfo::~FlintMethodInfo(void) {
    for(FlintAttribute *node = attributes; node != 0;) {
        FlintAttribute *next = node->next;
        node->~FlintAttribute();
        Flint::free(node);
        node = next;
    }
}
