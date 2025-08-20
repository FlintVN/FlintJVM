
#include <string.h>
#include "flint.h"
#include "flint_system_api.h"
#include "flint_method_info.h"
#include "flint_native_class.h"

FlintExceptionTable::FlintExceptionTable(uint16_t startPc, uint16_t endPc, uint16_t handlerPc, uint16_t catchType) :
startPc(startPc), endPc(endPc), handlerPc(handlerPc), catchType(catchType) {

}

static FlintNativeMethodPtr findNativeMethod(FlintMethodInfo *methodInfo) {
    FlintConstUtf8 &className = *methodInfo->classLoader.thisClass;
    FlintConstUtf8 &methodName = methodInfo->getName();
    FlintConstUtf8 &methodDesc = methodInfo->getDescriptor();
    for(uint32_t i = 0; i < LENGTH(BASE_NATIVE_CLASS_LIST); i++) {
        if(BASE_NATIVE_CLASS_LIST[i]->className == className) {
            for(uint32_t k = 0; k < BASE_NATIVE_CLASS_LIST[i]->methodCount; k++) {
                if(
                    BASE_NATIVE_CLASS_LIST[i]->methods[k].name == methodName &&
                    BASE_NATIVE_CLASS_LIST[i]->methods[k].descriptor == methodDesc
                ) {
                    return BASE_NATIVE_CLASS_LIST[i]->methods[k].nativeMathod;
                }
            }
            break;
        }
    }
    return FlintAPI::System::findNativeMethod(methodInfo);
}

FlintMethodInfo::FlintMethodInfo(FlintClassLoader &classLoader, FlintMethodAccessFlag accessFlag, uint16_t nameIndex, uint16_t descIndex) :
accessFlag(accessFlag),
classLoader(classLoader),
nameIndex(nameIndex),
descIndex(descIndex),
code(NULL_PTR) {

}

FlintConstUtf8 &FlintMethodInfo::getName(void) const {
    return classLoader.getConstUtf8(nameIndex);
}

FlintConstUtf8 &FlintMethodInfo::getDescriptor(void) const {
    return classLoader.getConstUtf8(descIndex);
}

uint8_t *FlintMethodInfo::getCode(void) {
    if(accessFlag & METHOD_NATIVE) {
        if(code == 0)
            code = (uint8_t *)findNativeMethod(this);
        return (uint8_t *)code;
    }
    FlintCodeAttribute *codeAttr = (FlintCodeAttribute *)code;
    return (uint8_t *)&((FlintExceptionTable *)codeAttr->data)[codeAttr->exceptionLength];
}

uint32_t FlintMethodInfo::getCodeLength(void) const {
    return (accessFlag & METHOD_NATIVE) ? 0 : ((FlintCodeAttribute *)code)->codeLength;
}

uint16_t FlintMethodInfo::getMaxLocals(void) const {
    return (accessFlag & METHOD_NATIVE) ? 0 : ((FlintCodeAttribute *)code)->maxLocals;
}

uint16_t FlintMethodInfo::getMaxStack(void) const {
    return (accessFlag & METHOD_NATIVE) ? 0 : ((FlintCodeAttribute *)code)->maxStack;
}

uint16_t FlintMethodInfo::getExceptionLength(void) const {
    return (accessFlag & METHOD_NATIVE) ? 0 : ((FlintCodeAttribute *)code)->exceptionLength;
}

FlintExceptionTable *FlintMethodInfo::getException(uint16_t index) const {
    if(accessFlag & METHOD_NATIVE)
        return NULL_PTR;
    FlintCodeAttribute *codeAttr = (FlintCodeAttribute *)code;
    return &((FlintExceptionTable *)codeAttr->data)[index];
}

bool FlintMethodInfo::isStaticCtor(void) {
    return &getName() == (FlintConstUtf8 *)staticConstructorName;
}

FlintMethodInfo::~FlintMethodInfo(void) {
    if(!(accessFlag & (METHOD_NATIVE | METHOD_UNLOADED))) {
        if(code)
            Flint::free(code);
    }
}
