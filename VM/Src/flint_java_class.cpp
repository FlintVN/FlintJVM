
#include <string.h>
#include "flint_java_class.h"
#include "flint_const_name.h"
#include "flint_fields_data.h"
#include "flint.h"

FlintJavaString &FlintJavaClass::getName(void) const {
    return *(FlintJavaString *)getFields().getFieldObject(*(const FlintConstUtf8 *)"\x04\x00\x5E\x56""name").object;
}

void FlintJavaClass::setName(FlintJavaString *name) {
    getFields().getFieldObject(*(const FlintConstUtf8 *)"\x04\x00\x5E\x56""name").object = name;
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

const FlintConstUtf8 *FlintJavaClass::getComponentTypeName(Flint &flint, uint32_t *dimensions) const {
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
    const FlintConstUtf8 *type = NULL;
    if(dims == 0) {
        switch(typeLength) {
            case 3:
                if(strncmp(typeText, "int", typeLength) == 0)
                    return primTypeConstUtf8List[6];
                break;
            case 4: {
                if(strncmp(typeText, "byte", typeLength) == 0)
                    return primTypeConstUtf8List[4];
                else if(strncmp(typeText, "char", typeLength) == 0)
                    return primTypeConstUtf8List[1];
                else if(strncmp(typeText, "long", typeLength) == 0)
                    return primTypeConstUtf8List[7];
                break;
            }
            case 5: {
                if(strncmp(typeText, "float", typeLength) == 0)
                    return primTypeConstUtf8List[2];
                else if(strncmp(typeText, "short", typeLength) == 0)
                    return primTypeConstUtf8List[5];
                break;
            }
            case 6:
                if(strncmp(typeText, "double", typeLength) == 0)
                    return primTypeConstUtf8List[3];
                break;
            case 7:
                if(strncmp(typeText, "boolean", typeLength) == 0)
                    return primTypeConstUtf8List[0];
                break;
            default:
                break;
        }
    }
    else if(*typeText == 'L') {
        typeText++;
        typeLength -= 2;
    }
    if(type == NULL) {
        uint8_t atype = FlintJavaObject::convertToAType(typeText[0]);
        if((dims != 0) && (atype != 0))
            return primTypeConstUtf8List[atype - 4];
        else {
            char *text = (char *)Flint::malloc(typeLength + sizeof(".class"));
            for(uint32_t i = 0; i < typeLength; i++)
                text[i] = (typeText[i] == '.') ? '/' : typeText[i];
            memcpy(&text[typeLength], ".class", sizeof(".class"));
            try {
                if(FlintAPI::IO::finfo(text, NULL, NULL) != FILE_RESULT_OK) {
                    Flint::free(text);
                    return NULL;
                }
                type = &flint.getConstUtf8(text, typeLength);
            }
            catch(...) {
                Flint::free(text);
                throw;
            }
            Flint::free(text);
        }
    }
    return type;
}

FlintConstClass::FlintConstClass(FlintJavaClass &flintClass) : flintClass(flintClass) {

}
