
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
classLoader(classLoader), name((FlintConstUtf8 &)name), descriptor((FlintConstUtf8 &)descriptor), code(0) {

}

void FlintMethodInfo::setCode(FlintAttribute *attributeCode) {
    code = attributeCode;
}

uint8_t *FlintMethodInfo::getCode(void) {
    if(accessFlag & METHOD_NATIVE) {
        if(code == 0)
            code = (void *)findNativeMethod(*this);
        return (uint8_t *)code;
    }
    return (uint8_t *)(((FlintCodeAttribute *)code)->code);
}

uint32_t FlintMethodInfo::getCodeLength(void) const {
    if(accessFlag & METHOD_NATIVE)
        return 0;
    return ((FlintCodeAttribute *)code)->codeLength;
}

uint16_t FlintMethodInfo::getMaxLocals(void) const {
    if(accessFlag & METHOD_NATIVE)
        return 0;
    return ((FlintCodeAttribute *)code)->maxLocals;
}

uint16_t FlintMethodInfo::getMaxStack(void) const {
    if(accessFlag & METHOD_NATIVE)
        return 0;
    return ((FlintCodeAttribute *)code)->maxStack;
}

uint16_t FlintMethodInfo::getExceptionLength(void) const {
    if(accessFlag & METHOD_NATIVE)
        return 0;
    return ((FlintCodeAttribute *)code)->exceptionTableLength;
}

FlintExceptionTable *FlintMethodInfo::getException(uint16_t index) const {
    if(accessFlag & METHOD_NATIVE)
        return NULL;
    return ((FlintCodeAttribute *)code)->getException(index);
}

bool FlintMethodInfo::isStaticCtor(void) {
    return &name == &staticConstructorName;
}

FlintMethodInfo::~FlintMethodInfo(void) {
    if((!(accessFlag & METHOD_NATIVE)) && code) {
        ((FlintAttribute *)code)->~FlintAttribute();
        Flint::free(code);
    }
}
