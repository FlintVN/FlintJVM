
#include "mjvm_string.h"
#include "mjvm_const_name.h"
#include "mjvm_fields_data.h"

static const uint8_t utf8ByteCount[] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6
};

uint8_t MjvmString::getUtf8DecodeSize(char c) {
    return (c & 0x80) ? utf8ByteCount[((uint8_t)c - 0xC0) & 0xFC] : 1;
}

uint8_t MjvmString::getUtf8EncodeSize(uint16_t c) {
    return (c < 0x80) ? 1 : ((c < 0x0800) ? 2 : 3);
}

uint32_t MjvmString::utf8Decode(const char *c) {
    if(*c & 0x80) {
        uint8_t byteCount = getUtf8DecodeSize(*c);

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

uint8_t MjvmString::utf8Encode(uint16_t c, char *buff) {
    if(c < 0x80) {
        buff[0] = (uint8_t)c;
        return 1;
    }
    else if(c < 0x0800) {
        buff[0] = 0xC0 | (c >> 6);
        buff[1] = 0x80 | (c & 0x3F);
        return 2;
    }
    else {
        buff[0] = 0xE0 | (c >> 12);
        buff[1] = 0x80 | ((c >> 6) & 0x3F);
        buff[2] = 0x80 | (c & 0x3F);
        return 3;
    }
}

uint32_t MjvmString::utf8StrLen(const char *utf8) {
    uint32_t len = 0;
    while(*utf8) {
        utf8 += getUtf8DecodeSize(*utf8);
        len++;
    }
    return len;
}

uint32_t MjvmString::getUft8BuffSize(MjvmString &str) {
    uint32_t length = str.getLength();
    const char *text = str.getText();
    uint32_t ret = 0;
    if(str.getCoder() == 0) {
        for(uint32_t i = 0; i < length; i++)
            ret += getUtf8EncodeSize(text[i]);
    }
    else {
        for(uint32_t i = 0; i < length; i++)
            ret += getUtf8EncodeSize(((uint16_t *)text)[i]);
    }
    return ret;
}

bool MjvmString::isLatin1(const char *utf8) {
    while(*utf8) {
        if((int8_t)*utf8 < 0) {
            uint8_t byteCount = getUtf8DecodeSize(*utf8);
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
    MjvmObject *byteArray = ((MjvmFieldsData *)data)->getFieldObject(*(MjvmConstNameAndType *)stringValueFieldName).object;
    return (const char *)((MjvmString *)byteArray)->data;
}

uint32_t MjvmString::getLength(void) const {
    MjvmString *byteArray = (MjvmString *)((MjvmFieldsData *)data)->getFieldObject(*(MjvmConstNameAndType *)stringValueFieldName).object;
    if(getCoder() == 0)
        return byteArray->size / sizeof(int8_t);
    else
        return byteArray->size / (sizeof(int8_t) << 1);
}

uint8_t MjvmString::getCoder(void) const {
    return ((MjvmFieldsData *)data)->getFieldData32(*(MjvmConstNameAndType *)stringCoderFieldName).value;
}

bool MjvmString::equals(const char *text, uint32_t length) const {
    uint8_t coder1 = getCoder();
    if((getLength() != length) || (coder1 != 0))
        return false;
    const char *value1 = getText();
    for(uint32_t i = 0; i < length; i++) {
        if(value1[i] != text[i])
            return false;
    }
    return true;
}

bool MjvmString::equals(const MjvmConstUtf8 &utf8) const {
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
            value2 += getUtf8DecodeSize(*value2);
        }
    }
    else {
        const char *value2 = utf8.text;
        for(uint32_t i = 0; i < len2; i++) {
            uint16_t c1 = ((uint16_t *)value1)[i];
            uint16_t c2 = utf8Decode(value2);
            if(c1 != c2)
                return false;
            value2 += getUtf8DecodeSize(*value2);
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
