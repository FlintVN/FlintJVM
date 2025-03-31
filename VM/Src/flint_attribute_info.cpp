
#include <string.h>
#include "flint.h"
#include "flint_common.h"
#include "flint_attribute_info.h"

FlintAttributeType FlintAttribute::parseAttributeType(const FlintConstUtf8 &name) {
    switch(name.length) {
        case 4:
            if(strncmp(name.text, "Code", name.length) == 0)
                return ATTRIBUTE_CODE;
            break;
        case 13:
            if(strncmp(name.text, "ConstantValue", name.length) == 0)
                return ATTRIBUTE_CONSTANT_VALUE;
            break;
        case 16:
            if(strncmp(name.text, "BootstrapMethods", name.length) == 0)
                return ATTRIBUTE_BOOTSTRAP_METHODS;
            break;
        default:
            break;
    }
    return ATTRIBUTE_UNKNOW;
}

FlintAttribute::FlintAttribute(FlintAttributeType type) : attributeType(type) {

}

FlintAttribute::~FlintAttribute(void) {

}

FlintNativeAttribute::FlintNativeAttribute(FlintNativeMethodPtr nativeMethod) : FlintAttribute(ATTRIBUTE_NATIVE), nativeMethod(nativeMethod) {

}

FlintNativeAttribute::~FlintNativeAttribute(void) {

}

FlintExceptionTable::FlintExceptionTable(uint16_t startPc, uint16_t endPc, uint16_t handlerPc, uint16_t catchType) :
startPc(startPc), endPc(endPc), handlerPc(handlerPc), catchType(catchType) {

}

FlintCodeAttribute::FlintCodeAttribute(uint16_t maxStack, uint16_t maxLocals) :
FlintAttribute(ATTRIBUTE_CODE), maxStack(maxStack), maxLocals(maxLocals), codeLength(0),
exceptionTableLength(0), code(0), exceptionTable(0) {

}

void FlintCodeAttribute::setCode(uint8_t *code, uint32_t length) {
    this->code = code;
    *(uint32_t *)&codeLength = length;
}

void FlintCodeAttribute::setExceptionTable(FlintExceptionTable *exceptionTable, uint16_t length) {
    this->exceptionTable = exceptionTable;
    *(uint16_t *)&exceptionTableLength = length;
}

FlintExceptionTable &FlintCodeAttribute::getException(uint16_t index) const {
    if(index < exceptionTableLength)
        return exceptionTable[index];
    throw "index for FlintExceptionTable is invalid";
}

FlintCodeAttribute::~FlintCodeAttribute(void) {
    if(code)
        Flint::free((void *)code);
    if(exceptionTable)
        Flint::free((void *)exceptionTable);
}

FlintBootstrapMethod::FlintBootstrapMethod(uint16_t bootstrapMethodRef, uint16_t numBootstrapArguments) :
bootstrapMethodRef(bootstrapMethodRef), numBootstrapArguments(numBootstrapArguments) {

}

uint16_t FlintBootstrapMethod::getBootstrapArgument(uint16_t index) const {
    if(index < numBootstrapArguments)
        return bootstrapArguments[index];
    throw "index for BootstrapArgument is invalid";
}

AttributeBootstrapMethods::AttributeBootstrapMethods(uint16_t numBootstrapMethods) :
FlintAttribute(ATTRIBUTE_BOOTSTRAP_METHODS), numBootstrapMethods(numBootstrapMethods) {
    bootstrapMethods = (FlintBootstrapMethod **)Flint::malloc(numBootstrapMethods * sizeof(FlintBootstrapMethod *));
}

FlintBootstrapMethod &AttributeBootstrapMethods::getBootstrapMethod(uint16_t index) {
    if(index < numBootstrapMethods)
        return *bootstrapMethods[index];
    throw "index for FlintBootstrapMethod is invalid";
}

void AttributeBootstrapMethods::setBootstrapMethod(uint16_t index, FlintBootstrapMethod &bootstrapMethod) {
    bootstrapMethods[index] = &bootstrapMethod;
}

AttributeBootstrapMethods::~AttributeBootstrapMethods(void) {
    for(uint16_t i = 0; i < numBootstrapMethods; i++)
        Flint::free((void *)bootstrapMethods[i]);
    Flint::free(bootstrapMethods);
}
