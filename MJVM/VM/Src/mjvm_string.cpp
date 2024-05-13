
#include "mjvm_string.h"
#include "mjvm_const_name.h"
#include "mjvm_fields_data.h"

static const uint8_t utf8ByteCount[] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6
};

uint8_t MjvmString::getUtf8ByteCount(char c) {
    return (c & 0x80) ? utf8ByteCount[((uint8_t)c - 0xC0) & 0xFC] : 1;
}

uint32_t MjvmString::utf8Decode(const char *c) {
    if(*c & 0x80) {
        uint8_t byteCount = getUtf8ByteCount(*c);

        uint32_t code = *c & (0xFF >> (byteCount + 1));
        while(--byteCount) {
            c++;
            code <<= 6;
            code |= *c & 0x3F;
        }
        return code;
    }
    return *c;
}

uint32_t MjvmString::utf8StrLen(const char *utf8) {
    uint32_t len = 0;
    while(*utf8) {
        utf8 += getUtf8ByteCount(*utf8);
        len++;
    }
    return len;
}

bool MjvmString::isLatin1(const char *utf8) {
    while(*utf8) {
        if((int8_t)*utf8 < 0) {
            uint8_t byteCount = getUtf8ByteCount(*utf8);
            if(utf8Decode(utf8) > 255)
                return false;
            utf8 += byteCount;
        }
        else
            utf8++;
    }
    return true;
}

const char *MjvmString::getText(void) const {
    MjvmObject *byteArray = ((FieldsData *)data)->getFieldObject(*(ConstNameAndType *)stringValueFieldName).object;
    return (const char *)((MjvmString *)byteArray)->data;
}

uint32_t MjvmString::getLength(void) const {
    MjvmString *byteArray = (MjvmString *)((FieldsData *)data)->getFieldObject(*(ConstNameAndType *)stringValueFieldName).object;
    if(getCoder() == 0)
        return byteArray->size / sizeof(int8_t);
    else
        return byteArray->size / (sizeof(int8_t) << 1);
}

uint8_t MjvmString::getCoder(void) const {
    return ((FieldsData *)data)->getFieldData32(*(ConstNameAndType *)stringCoderFieldName).value;
}

bool MjvmString::equals(const ConstUtf8 &utf8) const {
    uint32_t len2 = utf8StrLen(utf8.text);
    uint8_t coder1 = getCoder();
    uint8_t coder2 = isLatin1(utf8.text) ? 0 : 1;
    if((getLength() != len2) || (coder1 != coder2))
        return false;
    const char *value1 = getText();
    if(coder1 == 0) {
        const char *value2 = utf8.text;
        for(uint32_t i = 0; i < len2; i++) {
            uint16_t c1 = value1[i];
            uint16_t c2 = utf8Decode(value2);
            if(c1 != c2)
                return false;
            value2 += getUtf8ByteCount(*value2);
        }
    }
    else {
        const char *value2 = utf8.text;
        for(uint32_t i = 0; i < len2; i++) {
            uint16_t c1 = ((uint16_t *)value1)[i];
            uint16_t c2 = utf8Decode(value2);
            if(c1 != c2)
                return false;
            value2 += getUtf8ByteCount(*value2);
        }
    }
    return true;
}

bool MjvmString::equals(MjvmString &str) const {
    if(this == &str)
        return true;
    uint32_t len1 = getLength();
    uint32_t len2 = str.getLength();
    uint8_t coder1 = getCoder();
    uint8_t coder2 = str.getCoder();
    if((len1 != len2) || (coder1 != coder2))
        return false;
    const char *value1 = getText();
    const char *value2 = str.getText();
    uint32_t len = len1 << coder1;
    for(uint32_t i = 0; i < len; i++) {
        if(value1[i] != value2[i])
            return false;
    }
    return true;
}

MjvmConstString::MjvmConstString(MjvmString &mjvmString) : mjvmString(mjvmString) {
    next = 0;
}
