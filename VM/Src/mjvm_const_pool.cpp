
#include <string.h>
#include "mjvm_common.h"
#include "mjvm_const_pool.h"

static MjvmParamInfo parseParamInfo(const MjvmConstUtf8 &descriptor) {
    const char *text = descriptor.text;
    MjvmParamInfo retVal = {0, 0};
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

bool MjvmConstUtf8::operator==(const MjvmConstUtf8 &another) const {
    if(CONST_UTF8_HASH(*this) == CONST_UTF8_HASH(another)) {
        if(strncmp(text, another.text, length) == 0)
            return true;
    }
    return false;
}

bool MjvmConstUtf8::operator!=(const MjvmConstUtf8 &another) const {
    if(CONST_UTF8_HASH(*this) == CONST_UTF8_HASH(another)) {
        if(strncmp(text, another.text, length) == 0)
            return false;
    }
    return true;
}

MjvmConstNameAndType::MjvmConstNameAndType(MjvmConstUtf8 &name, MjvmConstUtf8 &descriptor) :
name(name), descriptor(descriptor) {

}

MjvmConstField::MjvmConstField(MjvmConstUtf8 &className, MjvmConstNameAndType &nameAndType) :
className(className), nameAndType(nameAndType) {

}

MjvmConstMethod::MjvmConstMethod(MjvmConstUtf8 &className, MjvmConstNameAndType &nameAndType) :
className(className), nameAndType(nameAndType) {
    paramInfo = parseParamInfo(nameAndType.descriptor);
}

MjvmConstMethod::MjvmConstMethod(MjvmConstUtf8 &className, MjvmConstNameAndType &nameAndType, uint8_t argc, uint8_t retType) :
className(className), nameAndType(nameAndType) {
    paramInfo.argc = argc;
    paramInfo.retType = retType;
}

const MjvmParamInfo &MjvmConstMethod::getParmInfo() {
    return paramInfo;
}

bool MjvmConstNameAndType::operator==(const MjvmConstNameAndType &another) const {
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

bool MjvmConstNameAndType::operator!=(const MjvmConstNameAndType &another) const {
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
