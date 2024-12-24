
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
    if(this == &another)
        return true;
    else if(CONST_UTF8_HASH(*this) == CONST_UTF8_HASH(another)) {
        if(strncmp(text, another.text, length) == 0)
            return true;
    }
    return false;
}

bool FlintConstUtf8::operator!=(const FlintConstUtf8 &another) const {
    if(this == &another)
        return false;
    else if(CONST_UTF8_HASH(*this) == CONST_UTF8_HASH(another)) {
        if(strncmp(text, another.text, length) == 0)
            return false;
    }
    return true;
}

FlintConstNameAndType::FlintConstNameAndType(FlintConstUtf8 &name, FlintConstUtf8 &descriptor) :
name(name), descriptor(descriptor) {

}

FlintConstField::FlintConstField(FlintConstUtf8 &className, FlintConstNameAndType &nameAndType) :
className(className), nameAndType(nameAndType), fieldIndex(0) {

}

FlintConstMethod::FlintConstMethod(FlintConstUtf8 &className, FlintConstNameAndType &nameAndType) :
className(className), nameAndType(nameAndType), methodInfo(0) {
    paramInfo = parseParamInfo(nameAndType.descriptor);
}

FlintConstMethod::FlintConstMethod(FlintConstUtf8 &className, FlintConstNameAndType &nameAndType, uint8_t argc, uint8_t retType) :
className(className), nameAndType(nameAndType), methodInfo(0) {
    paramInfo.argc = argc;
    paramInfo.retType = retType;
}

const FlintParamInfo &FlintConstMethod::getParmInfo() {
    return paramInfo;
}

bool FlintConstNameAndType::operator==(const FlintConstNameAndType &another) const {
    if((name == another.name) &&  (descriptor == another.descriptor))
        return true;
    return false;
}

bool FlintConstNameAndType::operator!=(const FlintConstNameAndType &another) const {
    if((name == another.name) && (descriptor == another.descriptor))
        return false;
    return true;
}
