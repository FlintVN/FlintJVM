
#include <string.h>
#include "mjvm_common.h"
#include "mjvm_const_pool.h"

static ParamInfo parseParamInfo(const ConstUtf8 &descriptor) {
    const char *text = descriptor.text;
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

bool ConstUtf8::operator==(const ConstUtf8 &another) const {
    if(CONST_UTF8_HASH(*this) == CONST_UTF8_HASH(another)) {
        if(strncmp(text, another.text, length) == 0)
            return true;
    }
    return false;
}

bool ConstUtf8::operator!=(const ConstUtf8 &another) const {
    if(CONST_UTF8_HASH(*this) == CONST_UTF8_HASH(another)) {
        if(strncmp(text, another.text, length) == 0)
            return false;
    }
    return true;
}

ConstNameAndType::ConstNameAndType(ConstUtf8 &name, ConstUtf8 &descriptor) :
name(name), descriptor(descriptor) {

}

ConstField::ConstField(ConstUtf8 &className, ConstNameAndType &nameAndType) :
className(className), nameAndType(nameAndType) {

}

ConstMethod::ConstMethod(ConstUtf8 &className, ConstNameAndType &nameAndType) :
className(className), nameAndType(nameAndType) {
    paramInfo = parseParamInfo(nameAndType.descriptor);
}

ConstMethod::ConstMethod(ConstUtf8 &className, ConstNameAndType &nameAndType, uint8_t argc, uint8_t retType) :
className(className), nameAndType(nameAndType) {
    paramInfo.argc = argc;
    paramInfo.retType = retType;
}

const ParamInfo &ConstMethod::getParmInfo() {
    return paramInfo;
}

bool ConstNameAndType::operator==(const ConstNameAndType &another) const {
    if(
        (CONST_UTF8_HASH(name) == CONST_UTF8_HASH(another.name)) &&
        (CONST_UTF8_HASH(descriptor) == CONST_UTF8_HASH(another.descriptor)) &&
        (strncmp(name.text, another.name.text, name.length) == 0) &&
        (strncmp(descriptor.text, another.descriptor.text, descriptor.length) == 0)
    ) {
        return true;
    }
    return false;
}

bool ConstNameAndType::operator!=(const ConstNameAndType &another) const {
    if(
        (CONST_UTF8_HASH(name) == CONST_UTF8_HASH(another.name)) &&
        (CONST_UTF8_HASH(descriptor) == CONST_UTF8_HASH(another.descriptor)) &&
        (strncmp(name.text, another.name.text, name.length) == 0) &&
        (strncmp(descriptor.text, another.descriptor.text, descriptor.length) == 0)
    ) {
        return false;
    }
    return true;
}
