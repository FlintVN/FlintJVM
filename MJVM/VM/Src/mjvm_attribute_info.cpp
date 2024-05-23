
#include <string.h>
#include "mjvm.h"
#include "mjvm_common.h"
#include "mjvm_attribute_info.h"

AttributeType AttributeInfo::parseAttributeType(const ConstUtf8 &name) {
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

AttributeInfo::AttributeInfo(AttributeType type) : next(0), attributeType(type) {

}

AttributeInfo::~AttributeInfo(void) {

}

AttributeRaw::AttributeRaw(AttributeType type, uint16_t length) : AttributeInfo(type), length(length) {

}

const uint8_t *AttributeRaw::getRaw(void) const {
    return (const uint8_t *)raw;
}

AttributeRaw::~AttributeRaw(void) {

}

AttributeNative::AttributeNative(NativeMethodPtr nativeMethod) : AttributeInfo(ATTRIBUTE_NATIVE), nativeMethod(nativeMethod) {

}

AttributeNative::~AttributeNative(void) {

}

ExceptionTable::ExceptionTable(uint16_t startPc, uint16_t endPc, uint16_t handlerPc, uint16_t catchType) :
startPc(startPc), endPc(endPc), handlerPc(handlerPc), catchType(catchType) {

}

AttributeCode::AttributeCode(uint16_t maxStack, uint16_t maxLocals) :
AttributeInfo(ATTRIBUTE_CODE), maxStack(maxStack), maxLocals(maxLocals), codeLength(0),
exceptionTableLength(0), code(0), exceptionTable(0), attributes(0) {

}

void AttributeCode::setCode(uint8_t *code, uint32_t length) {
    this->code = code;
    *(uint32_t *)&codeLength = length;
}

void AttributeCode::setExceptionTable(ExceptionTable *exceptionTable, uint16_t length) {
    this->exceptionTable = exceptionTable;
    *(uint16_t *)&exceptionTableLength = length;
}

void AttributeCode::addAttribute(AttributeInfo *attribute) {
    attribute->next = this->attributes;
    this->attributes = attribute;
}

const ExceptionTable &AttributeCode::getException(uint16_t index) const {
    if(index < exceptionTableLength)
        return exceptionTable[index];
    throw "index for ExceptionTable is invalid";
}

AttributeCode::~AttributeCode(void) {
    if(code)
        Mjvm::free((void *)code);
    if(exceptionTable)
        Mjvm::free((void *)exceptionTable);
    for(AttributeInfo *node = attributes; node != 0;) {
        AttributeInfo *next = node->next;
        node->~AttributeInfo();
        Mjvm::free(node);
        node = next;
    }
}

BootstrapMethod::BootstrapMethod(uint16_t bootstrapMethodRef, uint16_t numBootstrapArguments) :
bootstrapMethodRef(bootstrapMethodRef), numBootstrapArguments(numBootstrapArguments) {

}

uint16_t BootstrapMethod::getBootstrapArgument(uint16_t index) const {
    if(index < numBootstrapArguments)
        return bootstrapArguments[index];
    throw "index for BootstrapArgument is invalid";
}

AttributeBootstrapMethods::AttributeBootstrapMethods(uint16_t numBootstrapMethods) :
AttributeInfo(ATTRIBUTE_BOOTSTRAP_METHODS), numBootstrapMethods(numBootstrapMethods) {
    bootstrapMethods = (const BootstrapMethod **)Mjvm::malloc(numBootstrapMethods * sizeof(BootstrapMethod *));
}

const BootstrapMethod &AttributeBootstrapMethods::getBootstrapMethod(uint16_t index) {
    if(index < numBootstrapMethods)
        return *bootstrapMethods[index];
    throw "index for BootstrapMethod is invalid";
}

void AttributeBootstrapMethods::setBootstrapMethod(uint16_t index, const BootstrapMethod &bootstrapMethod) {
    bootstrapMethods[index] = &bootstrapMethod;
}

AttributeBootstrapMethods::~AttributeBootstrapMethods(void) {
    for(uint16_t i = 0; i < numBootstrapMethods; i++)
        Mjvm::free((void *)bootstrapMethods[i]);
    Mjvm::free(bootstrapMethods);
}
