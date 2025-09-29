
#include <string.h>
#include "flint_std.h"
#include "flint_const_pool.h"

uint8_t parseArgc(const char *desc) {
    uint8_t argc = 0;
    while(*desc == '(')
        desc++;
    while(*desc) {
        if(*desc == ')')
            return argc;
        else if(*desc == '[')
            desc++;
        else {
            argc += (*desc == 'J' || *desc == 'D') ? 2 : 1;
            if(*desc++ == 'L') {
                while(*desc) {
                    if(*desc == ')')
                        return argc;
                    else if(*desc == ';') {
                        desc++;
                        break;
                    }
                    desc++;
                }
            }
        }
    }
    return argc;
}

bool ConstNameAndType::operator==(ConstNameAndType &another) const {
    if(
        hash == another.hash &&
        strcmp(name, another.name) == 0 &&
        strcmp(desc, another.desc) == 0
    ) {
        return true;
    }
    return false;
}

bool ConstNameAndType::operator!=(ConstNameAndType &another) const {
    return !(*this == another);
}

ConstField::ConstField(const char *className, ConstNameAndType *nameAndType) :
className(className), loader(NULL), nameAndType(nameAndType), fieldIndex(0) {

}

ConstMethod::ConstMethod(const char *className, ConstNameAndType *nameAndType) :
className(className), nameAndType(nameAndType), methodInfo(NULL) {
    argc = parseArgc(nameAndType->desc);
}

uint8_t ConstMethod::getArgc(void) const {
    return argc;
}

bool ConstInvokeDynamic::isLinked(void) const {
    return (tag & 0x80) ? false : true;
}

void ConstInvokeDynamic::linkTo(JObject *obj) {
    *(uint32_t *)&value = (uint32_t)obj;
    *(ConstPoolTag *)&tag = CONST_INVOKE_DYNAMIC;
}

JObject *ConstInvokeDynamic::getCallSite(void) const {
    return (JObject *)value;
}

uint16_t ConstInvokeDynamic::getBootstrapMethodAttrIndex(void) const {
    return ((uint16_t *)&value)[0];
}

uint16_t ConstInvokeDynamic::getNameAndTypeIndex(void) const {
    return ((uint16_t *)&value)[1];
}

ConstMethodHandle::ConstMethodHandle(ConstPoolTag tag, RefKind refKind, uint16_t refIndex) :
tag(tag), refKind(refKind), refIndex(refIndex), value(NULL) {

}
