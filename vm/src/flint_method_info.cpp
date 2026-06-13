
#include "flint.h"
#include "flint_common.h"
#include "flint_native.h"
#include "flint_method_info.h"

ExceptionTable::ExceptionTable(uint16_t startPc, uint16_t endPc, uint16_t handlerPc, uint16_t catchType) :
startPc(startPc), endPc(endPc), handlerPc(handlerPc), catchType(catchType) {

}

MethodInfo::MethodInfo(ClassLoader *loader, MethodAccessFlag accessFlag, const char *name, const char *desc) :
accessFlag(accessFlag), loader(loader), name(name), desc(desc),
hash((Hash(name) & 0xFFFF) | (Hash(desc) << 16)), retType(NULL), code(NULL) {

}

const char *MethodInfo::getReturnType(void) {
    if(retType == NULL) {
        const char *tmp = desc;
        while(*tmp != ')')
            tmp++;
        retType = ++tmp;
    }
    return retType;
}

uint8_t *MethodInfo::getCode(void) {
    if(accessFlag & METHOD_NATIVE) {
        if(code == 0)
            code = (uint8_t *)NativeClass::findNativeMethod(this);
        return (uint8_t *)code;
    }
    CodeAttribute *codeAttr = (CodeAttribute *)code;
    return (uint8_t *)&((ExceptionTable *)codeAttr->data)[codeAttr->exceptionLength];
}

uint32_t MethodInfo::getCodeLength(void) const {
    return (accessFlag & METHOD_NATIVE) ? 0 : ((CodeAttribute *)code)->codeLength;
}

uint16_t MethodInfo::getMaxLocals(void) const {
    return (accessFlag & METHOD_NATIVE) ? 0 : ((CodeAttribute *)code)->maxLocals;
}

uint16_t MethodInfo::getMaxStack(void) const {
    return (accessFlag & METHOD_NATIVE) ? 0 : ((CodeAttribute *)code)->maxStack;
}

uint16_t MethodInfo::getExceptionLength(void) const {
    return (accessFlag & METHOD_NATIVE) ? 0 : ((CodeAttribute *)code)->exceptionLength;
}

ExceptionTable *MethodInfo::getException(uint16_t index) const {
    if(accessFlag & METHOD_NATIVE)
        return NULL;
    CodeAttribute *codeAttr = (CodeAttribute *)code;
    return &((ExceptionTable *)codeAttr->data)[index];
}
