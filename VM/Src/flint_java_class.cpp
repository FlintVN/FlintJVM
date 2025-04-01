
#include <string.h>
#include "flint_java_class.h"
#include "flint_const_name_base.h"
#include "flint_fields_data.h"
#include "flint.h"

FlintJavaString &FlintJavaClass::getName(void) const {
    return *(FlintJavaString *)getFields().getFieldObject(nameFieldName).object;
}

void FlintJavaClass::setName(FlintJavaString *name) {
    getFields().getFieldObject(nameFieldName).object = name;
}

bool FlintJavaClass::isArray(void) const {
    FlintJavaString &name = getName();
    const char *text = name.getText();
    uint8_t coder = name.getCoder();
    uint32_t length = name.getLength();
    if(length > 1 && coder == 0 && text[0] == '[')
        return true;
    else
        return false;
}

bool FlintJavaClass::isPrimitive(void) const {
    FlintJavaString &name = getName();
    uint8_t coder = name.getCoder();
    uint32_t length = name.getLength();
    if(coder == 0) {
        switch(length) {
            case 3:
                return (strncmp(name.getText(), "int", length) == 0);
            case 4: {
                const char *text = name.getText();
                if(strncmp(text, "void", length) == 0)
                    return true;
                else if(strncmp(text, "byte", length) == 0)
                    return true;
                else if(strncmp(text, "char", length) == 0)
                    return true;
                else if(strncmp(text, "long", length) == 0)
                    return true;
                return false;
            }
            case 5: {
                const char *text = name.getText();
                if(strncmp(text, "float", length) == 0)
                    return true;
                else if(strncmp(text, "short", length) == 0)
                    return true;
                return false;
            }
            case 6:
                return (strncmp(name.getText(), "double", length) == 0);
            case 7:
                return (strncmp(name.getText(), "boolean", length) == 0);
            default:
                return false;
        }
    }
    return false;
}

const FlintConstUtf8 &FlintJavaClass::getBaseTypeName(Flint &flint, uint32_t *dimensions) const {
    uint32_t dims = 0;
    FlintJavaString &name = getName();
    const char *typeText = name.getText();
    uint32_t typeLength = name.getLength();
    while((*typeText == '[') && typeLength) {
        typeText++;
        typeLength--;
        dims++;
    }
    if(dimensions)
        *dimensions = dims;
    if(dims == 0) {
        switch(typeLength) {
            case 3:
                if(strncmp(typeText, "int", typeLength) == 0)
                    return integerPrimTypeName;
                break;
            case 4: {
                if(strncmp(typeText, "void", typeLength) == 0)
                    return voidPrimTypeName;
                else if(strncmp(typeText, "byte", typeLength) == 0)
                    return bytePrimTypeName;
                else if(strncmp(typeText, "char", typeLength) == 0)
                    return charPrimTypeName;
                else if(strncmp(typeText, "long", typeLength) == 0)
                    return longPrimTypeName;
                break;
            }
            case 5: {
                if(strncmp(typeText, "float", typeLength) == 0)
                    return floatPrimTypeName;
                else if(strncmp(typeText, "short", typeLength) == 0)
                    return shortPrimTypeName;
                break;
            }
            case 6:
                if(strncmp(typeText, "double", typeLength) == 0)
                    return doublePrimTypeName;
                break;
            case 7:
                if(strncmp(typeText, "boolean", typeLength) == 0)
                    return booleanPrimTypeName;
                break;
            default:
                break;
        }
    }
    else if(*typeText == 'L') {
        typeLength -= (typeText[typeLength - 1] == ';') ? 2 : 1;
        typeText++;
    }
    uint8_t atype = FlintJavaObject::convertToAType(typeText[0]);
    if((dims != 0) && (atype != 0))
        return *primTypeConstUtf8List[atype - 4];
    return flint.getTypeNameConstUtf8(typeText, typeLength);
}

FlintConstClass::FlintConstClass(FlintJavaClass &flintClass) : flintClass(flintClass) {
    next1 = 0;
    next2 = 0;
}
