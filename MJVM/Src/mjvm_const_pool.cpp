
#include <string.h>
#include "mjvm_const_pool.h"

ConstUtf8::ConstUtf8(uint16_t length) : length(length) {

}

const char *ConstUtf8::getText(void) const {
    return (const char *)text;
}

bool ConstUtf8::operator==(const ConstUtf8 &another) const {
    if(this == &another || (length == another.length && strncmp(text, another.text, length) == 0))
        return true;
    return false;
}

bool ConstUtf8::operator!=(const ConstUtf8 &another) const {
    if(length != another.length || strncmp(text, another.text, length) != 0)
        return true;
    return false;
}

ConstNameAndType::ConstNameAndType(const ConstUtf8 &name, const ConstUtf8 &descriptor) :
name(name),
descriptor(descriptor) {

}

ConstField::ConstField(const ConstUtf8 &className, const ConstNameAndType &nameAndType) :
className(className),
nameAndType(nameAndType) {

}

ConstMethod::ConstMethod(const ConstUtf8 &className, const ConstNameAndType &nameAndType) :
className(className),
nameAndType(nameAndType) {

}

ParamInfo parseParamInfo(const ConstUtf8 &descriptor) {
    const char *text = descriptor.getText();
    ParamInfo retVal = {0, 0};
    if(*text != '(')
        throw "the descriptor is not a description of the method";
    text++;
    while(*text) {
        if(*text == ')') {
            retVal.retType = text[1];
            return retVal;
        }
        else if(*text == '[')
            text++;
        else {
            retVal.argc += (*text == 'J' || *text == 'D') ? 2 : 1;
            if(*text++ == 'L') {
                while(*text) {
                    if(*text == ')') {
                        retVal.retType = text[1];
                        return retVal;
                    }
                    else if(*text == ';') {
                        text++;
                        break;
                    }
                    text++;
                }
            }
        }
    }
    throw "descriptor is invalid";
}

ParamInfo ConstMethod::parseParamInfo(void) const {
    return ::parseParamInfo(nameAndType.descriptor);
}

bool ConstNameAndType::operator==(const ConstNameAndType &another) const {
    if(
        name.length == another.name.length &&
        descriptor.length == another.descriptor.length &&
        strncmp(name.getText(), another.name.getText(), name.length) == 0 &&
        strncmp(descriptor.getText(), another.descriptor.getText(), descriptor.length) == 0
    ) {
        return true;
    }
    return false;
}

bool ConstNameAndType::operator!=(const ConstNameAndType &another) const {
    if(
        name.length != another.name.length ||
        descriptor.length != another.descriptor.length ||
        strncmp(name.getText(), another.name.getText(), name.length) != 0 ||
        strncmp(descriptor.getText(), another.descriptor.getText(), descriptor.length) != 0
    ) {
        return true;
    }
    return false;
}
