
#include <string.h>
#include "mjvm_heap.h"
#include "mjvm_common.h"
#include "mjvm_attribute_info.h"

AttributeType AttributeInfo::parseAttributeType(const ConstUtf8 &name) {
    switch(name.length) {
        case 4: {
            if(strncmp(name.text, "Code", name.length) == 0)
                return ATTRIBUTE_CODE;
            break;
        }
        case 10:
            if(strncmp(name.text, "SourceFile", name.length) == 0)
                return ATTRIBUTE_SOURCE_FILE;
            break;
        case 13:
            if(strncmp(name.text, "StackMapTable", name.length) == 0)
                return ATTRIBUTE_STACK_MAP_TABLE;
            break;
        case 15:
            if(strncmp(name.text, "LineNumberTable", name.length) == 0)
                return ATTRIBUTE_LINE_NUMBER_TABLE;
            break;
        case 18:
            if(strncmp(name.text, "LocalVariableTable", name.length) == 0)
                return ATTRIBUTE_LOCAL_VARIABLE_TABLE;
            break;
        default:
            break;
    }
    throw "unkown the attribute the name";
}

AttributeInfo::AttributeInfo(AttributeType attributeType) : attributeType(attributeType) {

}

AttributeInfo::~AttributeInfo() {

}

ExceptionTable::ExceptionTable(uint16_t startPc, uint16_t endPc, uint16_t handlerPc, const ConstUtf8 &catchType) :
startPc(startPc), endPc(endPc), handlerPc(handlerPc), catchType(catchType) {

}

AttributeCode::AttributeCode(uint16_t maxStack, uint16_t maxLocals) :
AttributeInfo(ATTRIBUTE_CODE), maxStack(maxStack), maxLocals(maxLocals), codeLength(0),
exceptionTableLength(0), attributesCount(0) {
    code = (uint8_t *)MJVM_Malloc(codeLength);
    exceptionTable = (ExceptionTable *)MJVM_Malloc(exceptionTableLength * sizeof(ExceptionTable));
    attributes = (const AttributeInfo **)MJVM_Malloc(exceptionTableLength * sizeof(AttributeInfo *));
}

void AttributeCode::setCode(uint8_t *code, uint32_t length) {
    this->code = code;
    *(uint32_t *)&codeLength = codeLength;
}

void AttributeCode::setExceptionTable(ExceptionTable *exceptionTable, uint16_t length) {
    this->exceptionTable = exceptionTable;
    *(uint16_t *)&exceptionTableLength = exceptionTableLength;
}

void AttributeCode::setAttributes(AttributeInfo **attributes, uint16_t length) {
    this->attributes = (const AttributeInfo **)attributes;
    *(uint16_t *)&attributesCount = attributesCount;
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
        MJVM_Free((void *)code);
    if(exceptionTable)
        MJVM_Free((void *)exceptionTable);
    if(attributes) {
        for(uint16_t i = 0; i < attributesCount; i++)
            attributes[i]->~AttributeInfo();
        MJVM_Free((void *)attributes);
    }
}
