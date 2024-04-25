
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
        case 10:
            if(strncmp(name.text, "SourceFile", name.length) == 0)
                return ATTRIBUTE_SOURCE_FILE;
            break;
        case 11:
            if(strncmp(name.text, "NestMembers", name.length) == 0)
                return ATTRIBUTE_NEST_MEMBERS;
            break;
        case 12:
            if(strncmp(name.text, "InnerClasses", name.length) == 0)
                return ATTRIBUTE_INNER_CLASSES;
            break;
        case 13:
            if(strncmp(name.text, "StackMapTable", name.length) == 0)
                return ATTRIBUTE_STACK_MAP_TABLE;
            break;
        case 15:
            if(strncmp(name.text, "LineNumberTable", name.length) == 0)
                return ATTRIBUTE_LINE_NUMBER_TABLE;
            break;
        case 16:
            if(strncmp(name.text, "BootstrapMethods", name.length) == 0)
                return ATTRIBUTE_BOOTSTRAP_METHODS;
            break;
        case 18:
            if(strncmp(name.text, "LocalVariableTable", name.length) == 0)
                return ATTRIBUTE_LOCAL_VARIABLE_TABLE;
            break;
        default:
            break;
    }
    return ATTRIBUTE_UNKNOW;
}

AttributeInfo::AttributeInfo(AttributeType type) : attributeType(type) {

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

ExceptionTable::ExceptionTable(uint16_t startPc, uint16_t endPc, uint16_t handlerPc, uint16_t catchType) :
startPc(startPc), endPc(endPc), handlerPc(handlerPc), catchType(catchType) {

}

AttributeCode::AttributeCode(uint16_t maxStack, uint16_t maxLocals) :
AttributeInfo(ATTRIBUTE_CODE), maxStack(maxStack), maxLocals(maxLocals), codeLength(0),
exceptionTableLength(0), attributesCount(0), code(0), exceptionTable(0), attributes(0) {

}

void AttributeCode::setCode(uint8_t *code, uint32_t length) {
    this->code = code;
    *(uint32_t *)&codeLength = length;
}

void AttributeCode::setExceptionTable(ExceptionTable *exceptionTable, uint16_t length) {
    this->exceptionTable = exceptionTable;
    *(uint16_t *)&exceptionTableLength = length;
}

void AttributeCode::setAttributes(AttributeInfo **attributes, uint16_t length) {
    this->attributes = (const AttributeInfo **)attributes;
    *(uint16_t *)&attributesCount = length;
}

const ExceptionTable &AttributeCode::getException(uint16_t index) const {
    if(index < exceptionTableLength)
        return exceptionTable[index];
    throw "index for ExceptionTable is invalid";
}

const AttributeInfo &AttributeCode::getAttributes(uint16_t index) const {
    if(index < attributesCount)
        return *attributes[index];
    throw "index for attribute is invalid";
}

AttributeCode::~AttributeCode(void) {
    if(code)
        Mjvm::free((void *)code);
    if(exceptionTable)
        Mjvm::free((void *)exceptionTable);
    if(attributes) {
        for(uint16_t i = 0; i < attributesCount; i++) {
            attributes[i]->~AttributeInfo();
            Mjvm::free((void *)attributes[i]);
        }
        Mjvm::free((void *)attributes);
    }
}

LineNumber::LineNumber(uint16_t startPc, uint16_t lineNumber) : startPc(startPc), lineNumber(lineNumber) {

}

AttributeLineNumberTable::AttributeLineNumberTable(uint16_t length) : AttributeInfo(ATTRIBUTE_LINE_NUMBER_TABLE), LineNumberLenght(length) {

}

const LineNumber &AttributeLineNumberTable::getLineNumber(uint16_t index) const {
    if(index < LineNumberLenght)
        return lineNumberTable[index];
    throw "index for LineNumberTable is invalid";
}

AttributeLineNumberTable::~AttributeLineNumberTable(void) {

}

LocalVariable::LocalVariable(uint16_t startPc, uint16_t length, const ConstUtf8 &name, const ConstUtf8 &descriptor, uint16_t index) :
startPc(startPc), length(length), name(name), descriptor(descriptor), index(index) {

}

AttributeLocalVariableTable::AttributeLocalVariableTable(uint16_t length) : AttributeInfo(ATTRIBUTE_LOCAL_VARIABLE_TABLE), localVariableLength(length) {

}

const LocalVariable &AttributeLocalVariableTable::getLocalVariable(uint16_t index) const {
    if(index < localVariableLength)
        return lineNumberTable[index];
    throw "index for LocalVariableTable is invalid";
}

AttributeLocalVariableTable::~AttributeLocalVariableTable(void) {

}

BootstrapMethod::BootstrapMethod(uint16_t bootstrapMethodRef, uint16_t numBootstrapArguments) :
bootstrapMethodRef(bootstrapMethodRef), numBootstrapArguments(numBootstrapArguments) {

}

const uint16_t BootstrapMethod::getBootstrapArgument(uint16_t index) const {
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
