
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
    uint8_t result = 0;
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
    const FlintConstUtf8 *type = NULL;
    if(dims == 0) {
        switch(typeLength) {
            case 3:
                if(strncmp(typeText, "int", typeLength) == 0)
                    type = primTypeConstUtf8List[6];
                else
                    return NULL;
                break;
            case 4: {
                if(strncmp(typeText, "byte", typeLength) == 0)
                    type = primTypeConstUtf8List[4];
                else if(strncmp(typeText, "char", typeLength) == 0)
                    type = primTypeConstUtf8List[1];
                else if(strncmp(typeText, "long", typeLength) == 0)
                    type = primTypeConstUtf8List[7];
                else
                    return NULL;
                break;
            }
            case 5: {
                if(strncmp(typeText, "float", typeLength) == 0)
                    type = primTypeConstUtf8List[2];
                else if(strncmp(typeText, "short", typeLength) == 0)
                    type = primTypeConstUtf8List[5];
                else
                    return NULL;
                break;
            }
            case 6:
                if(strncmp(typeText, "double", typeLength) == 0)
                    type = primTypeConstUtf8List[3];
                else
                    return NULL;
                break;
            case 7:
                if(strncmp(typeText, "boolean", typeLength) == 0)
                    type = primTypeConstUtf8List[0];
                else
                    return NULL;
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
        char *text = (char *)Flint::malloc(typeLength);
        for(uint32_t i = 0; i < typeLength; i++)
            text[i] = (typeText[i] == '.') ? '/' : typeText[i];
        try {
            type = &flint.getConstUtf8(text, typeLength);
        }
        catch(...) {
            Flint::free(text);
            throw;
        }
        Flint::free(text);
    }
    if(dimensions)
        *dimensions = dims;
    return type;
}

FlintConstClass::FlintConstClass(FlintJavaClass &flintClass) : flintClass(flintClass) {

}
