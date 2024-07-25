
#include <string.h>
#include "flint_common.h"
#include "flint_const_pool.h"

static FlintParamInfo parseParamInfo(const FlintConstUtf8 &descriptor) {
    const char *text = descriptor.text;
    FlintParamInfo retVal = {0, 0};
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

bool FlintConstUtf8::operator==(const FlintConstUtf8 &another) const {
    if(CONST_UTF8_HASH(*this) == CONST_UTF8_HASH(another)) {
        if(strncmp(text, another.text, length) == 0)
            return true;
    }
    return false;
}

bool FlintConstUtf8::operator!=(const FlintConstUtf8 &another) const {
    if(CONST_UTF8_HASH(*this) == CONST_UTF8_HASH(another)) {
        if(strncmp(text, another.text, length) == 0)
            return false;
    }
    return true;
}

FlintConstNameAndType::FlintConstNameAndType(FlintConstUtf8 &name, FlintConstUtf8 &descriptor) :
name(name), descriptor(descriptor) {

}

FlintConstField::FlintConstField(FlintConstUtf8 &className, FlintConstNameAndType &nameAndType) :
className(className), nameAndType(nameAndType) {

}

FlintConstMethod::FlintConstMethod(FlintConstUtf8 &className, FlintConstNameAndType &nameAndType) :
className(className), nameAndType(nameAndType) {
    paramInfo = parseParamInfo(nameAndType.descriptor);
}

FlintConstMethod::FlintConstMethod(FlintConstUtf8 &className, FlintConstNameAndType &nameAndType, uint8_t argc, uint8_t retType) :
className(className), nameAndType(nameAndType) {
    paramInfo.argc = argc;
    paramInfo.retType = retType;
}

const FlintParamInfo &FlintConstMethod::getParmInfo() {
    return paramInfo;
}

bool FlintConstNameAndType::operator==(const FlintConstNameAndType &another) const {
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

bool FlintConstNameAndType::operator!=(const FlintConstNameAndType &another) const {
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
