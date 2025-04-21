
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

FlintExceptionTable *FlintCodeAttribute::getException(uint16_t index) const {
    return &exceptionTable[index];
}

FlintCodeAttribute::~FlintCodeAttribute(void) {
    if(code)
        Flint::free((void *)code);
    if(exceptionTable)
        Flint::free((void *)exceptionTable);
}
