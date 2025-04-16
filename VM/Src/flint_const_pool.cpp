
#include <string.h>
#include "flint_common.h"
#include "flint_const_pool.h"

uint8_t parseArgc(const FlintConstUtf8 &descriptor) {
    const char *text = descriptor.text;
    uint8_t argc = 0;
    if(*text != '(')
        throw "the descriptor is not a description of the method";
    text++;
    while(*text) {
        if(*text == ')')
            return argc;
        else if(*text == '[')
            text++;
        else {
            argc += (*text == 'J' || *text == 'D') ? 2 : 1;
            if(*text++ == 'L') {
                while(*text) {
                    if(*text == ')')
                        return argc;
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

FlintConstNameAndType::FlintConstNameAndType(const FlintConstUtf8 &name, const FlintConstUtf8 &descriptor) :
name((FlintConstUtf8 &)name), descriptor((FlintConstUtf8 &)descriptor) {

}

FlintConstField::FlintConstField(const FlintConstUtf8 &className, FlintConstNameAndType &nameAndType) :
className((FlintConstUtf8 &)className), nameAndType(nameAndType), fieldIndex(0) {

}

FlintConstMethod::FlintConstMethod(const FlintConstUtf8 &className, FlintConstNameAndType &nameAndType) :
className((FlintConstUtf8 &)className), nameAndType(nameAndType), methodInfo(0) {
    argc = parseArgc(nameAndType.descriptor);
}

uint8_t FlintConstMethod::getArgc(void) const {
    return argc;
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
