
#include <string.h>
#include "mjvm_heap.h"
#include "mjvm_common.h"
#include "mjvm_attribute_info.h"

AttributeType AttributeInfo::parseAttributeType(const ConstUtf8 &name) {
    switch(name.length) {
        case 4: {
            if(strncmp(name.getText(), "Code", name.length) == 0)
                return ATTRIBUTE_CODE;
            break;
        }
        case 10:
            if(strncmp(name.getText(), "SourceFile", name.length) == 0)
                return ATTRIBUTE_SOURCE_FILE;
            break;
        case 11:
            if(strncmp(name.getText(), "NestMembers", name.length) == 0)
                return ATTRIBUTE_NEST_MEMBERS;
            break;
        case 12:
            if(strncmp(name.getText(), "InnerClasses", name.length) == 0)
                return ATTRIBUTE_INNER_CLASSES;
            break;
        case 13:
            if(strncmp(name.getText(), "StackMapTable", name.length) == 0)
                return ATTRIBUTE_STACK_MAP_TABLE;
            break;
        case 15:
            if(strncmp(name.getText(), "LineNumberTable", name.length) == 0)
                return ATTRIBUTE_LINE_NUMBER_TABLE;
            break;
        case 18:
            if(strncmp(name.getText(), "LocalVariableTable", name.length) == 0)
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

ExceptionTable::ExceptionTable(uint16_t startPc, uint16_t endPc, uint16_t handlerPc, const ConstUtf8 &catchType) :
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
    throw "index for exception table is invalid";
}

const AttributeInfo &AttributeCode::getAttributes(uint16_t index) const {
    if(index < attributesCount)
        return *attributes[index];
    throw "index for attribute is invalid";
}

AttributeCode::~AttributeCode(void) {
    if(code)
        MjvmHeap::free((void *)code);
    if(exceptionTable)
        MjvmHeap::free((void *)exceptionTable);
    if(attributes) {
        for(uint16_t i = 0; i < attributesCount; i++) {
            attributes[i]->~AttributeInfo();
            MjvmHeap::free((void *)attributes[i]);
        }
        MjvmHeap::free((void *)attributes);
    }
}

LineNumber::LineNumber(uint16_t startPc, uint16_t lineNumber) : startPc(startPc), lineNumber(lineNumber) {

}

AttributeLineNumberTable::AttributeLineNumberTable(uint16_t length) : AttributeInfo(ATTRIBUTE_LINE_NUMBER_TABLE), LineNumberLenght(length) {

}

const LineNumber &AttributeLineNumberTable::getLineNumber(uint16_t index) const {
    if(index < LineNumberLenght)
        return lineNumberTable[index];
    throw "index for line number table is invalid";
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
    throw "index for local variable table is invalid";
}

AttributeLocalVariableTable::~AttributeLocalVariableTable(void) {

}
