
#include <string.h>
#include "mjvm.h"
#include "mjvm_common.h"
#include "mjvm_attribute_info.h"

MjvmAttributeType MjvmAttribute::parseAttributeType(const MjvmConstUtf8 &name) {
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

MjvmAttribute::MjvmAttribute(MjvmAttributeType type) : next(0), attributeType(type) {

}

MjvmAttribute::~MjvmAttribute(void) {

}

MjvmNativeAttribute::MjvmNativeAttribute(MjvmNativeMethodPtr nativeMethod) : MjvmAttribute(ATTRIBUTE_NATIVE), nativeMethod(nativeMethod) {

}

MjvmNativeAttribute::~MjvmNativeAttribute(void) {

}

MjvmExceptionTable::MjvmExceptionTable(uint16_t startPc, uint16_t endPc, uint16_t handlerPc, uint16_t catchType) :
startPc(startPc), endPc(endPc), handlerPc(handlerPc), catchType(catchType) {

}

MjvmCodeAttribute::MjvmCodeAttribute(uint16_t maxStack, uint16_t maxLocals) :
MjvmAttribute(ATTRIBUTE_CODE), maxStack(maxStack), maxLocals(maxLocals), codeLength(0),
exceptionTableLength(0), code(0), exceptionTable(0), attributes(0) {

}

void MjvmCodeAttribute::setCode(uint8_t *code, uint32_t length) {
    this->code = code;
    *(uint32_t *)&codeLength = length;
}

void MjvmCodeAttribute::setExceptionTable(MjvmExceptionTable *exceptionTable, uint16_t length) {
    this->exceptionTable = exceptionTable;
    *(uint16_t *)&exceptionTableLength = length;
}

void MjvmCodeAttribute::addAttribute(MjvmAttribute *attribute) {
    attribute->next = this->attributes;
    this->attributes = attribute;
}

MjvmExceptionTable &MjvmCodeAttribute::getException(uint16_t index) const {
    if(index < exceptionTableLength)
        return exceptionTable[index];
    throw "index for MjvmExceptionTable is invalid";
}

MjvmCodeAttribute::~MjvmCodeAttribute(void) {
    if(code)
        Mjvm::free((void *)code);
    if(exceptionTable)
        Mjvm::free((void *)exceptionTable);
    for(MjvmAttribute *node = attributes; node != 0;) {
        MjvmAttribute *next = node->next;
        node->~MjvmAttribute();
        Mjvm::free(node);
        node = next;
    }
}

MjvmBootstrapMethod::MjvmBootstrapMethod(uint16_t bootstrapMethodRef, uint16_t numBootstrapArguments) :
bootstrapMethodRef(bootstrapMethodRef), numBootstrapArguments(numBootstrapArguments) {

}

uint16_t MjvmBootstrapMethod::getBootstrapArgument(uint16_t index) const {
    if(index < numBootstrapArguments)
        return bootstrapArguments[index];
    throw "index for BootstrapArgument is invalid";
}

AttributeBootstrapMethods::AttributeBootstrapMethods(uint16_t numBootstrapMethods) :
MjvmAttribute(ATTRIBUTE_BOOTSTRAP_METHODS), numBootstrapMethods(numBootstrapMethods) {
    bootstrapMethods = (MjvmBootstrapMethod **)Mjvm::malloc(numBootstrapMethods * sizeof(MjvmBootstrapMethod *));
}

MjvmBootstrapMethod &AttributeBootstrapMethods::getBootstrapMethod(uint16_t index) {
    if(index < numBootstrapMethods)
        return *bootstrapMethods[index];
    throw "index for MjvmBootstrapMethod is invalid";
}

void AttributeBootstrapMethods::setBootstrapMethod(uint16_t index, MjvmBootstrapMethod &bootstrapMethod) {
    bootstrapMethods[index] = &bootstrapMethod;
}

AttributeBootstrapMethods::~AttributeBootstrapMethods(void) {
    for(uint16_t i = 0; i < numBootstrapMethods; i++)
        Mjvm::free((void *)bootstrapMethods[i]);
    Mjvm::free(bootstrapMethods);
}
