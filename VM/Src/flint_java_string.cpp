
#include "flint_java_string.h"
#include "flint_const_name.h"
#include "flint_fields_data.h"

static const uint8_t utf8ByteCount[] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6
};

uint8_t FlintJavaString::getUtf8DecodeSize(char c) {
    return (c & 0x80) ? utf8ByteCount[((uint8_t)c - 0xC0) & 0xFC] : 1;
}

uint8_t FlintJavaString::getUtf8EncodeSize(uint16_t c) {
    return (c < 0x80) ? 1 : ((c < 0x0800) ? 2 : 3);
}

uint32_t FlintJavaString::utf8Decode(const char *c) {
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

uint8_t FlintJavaString::utf8Encode(uint16_t c, char *buff) {
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

uint32_t FlintJavaString::utf8StrLen(const char *utf8) {
    uint32_t len = 0;
    while(*utf8) {
        utf8 += getUtf8DecodeSize(*utf8);
        len++;
    }
    return len;
}

uint32_t FlintJavaString::getUft8BuffSize(void) {
    uint32_t length = getLength();
    const char *text = getText();
    uint32_t ret = 0;
    if(getCoder() == 0) {
        for(uint32_t i = 0; i < length; i++)
            ret += getUtf8EncodeSize(text[i]);
    }
    else {
        for(uint32_t i = 0; i < length; i++)
            ret += getUtf8EncodeSize(((uint16_t *)text)[i]);
    }
    return ret;
}

bool FlintJavaString::isLatin1(const char *utf8) {
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

FlintInt8Array *FlintJavaString::getValue(void) const {
    return (FlintInt8Array *)getFields().getFieldObject(*(const FlintConstUtf8 *)"\x05\x00\x2B\x6E""value").object;
}

void FlintJavaString::setValue(FlintInt8Array &byteArray) {
    getFields().getFieldObject(*(const FlintConstUtf8 *)"\x05\x00\x2B\x6E""value").object = &byteArray;
}

const char *FlintJavaString::getText(void) const {
    FlintInt8Array *byteArray = getValue();
    return (const char *)byteArray->getData();
}

uint32_t FlintJavaString::getLength(void) const {
    FlintInt8Array *byteArray = getValue();
    if(getCoder() == 0)
        return byteArray->getLength();
    else
        return byteArray->getLength() / 2;
}

uint8_t FlintJavaString::getCoder(void) const {
    return getFields().getFieldData32(*(const FlintConstUtf8 *)"\x05\x00\xE8\x49""coder").value;
}

void FlintJavaString::setCoder(uint8_t coder) {
    getFields().getFieldData32(*(const FlintConstUtf8 *)"\x05\x00\xE8\x49""coder").value = coder;
}

bool FlintJavaString::equals(const char *text, uint32_t length) const {
    if((getLength() != length) || (getCoder() != 0))
        return false;
    const char *value = getText();
    for(uint32_t i = 0; i < length; i++) {
        if(value[i] != text[i])
            return false;
    }
    return true;
}

bool FlintJavaString::equals(const FlintConstUtf8 &utf8) const {
    uint32_t len2 = utf8StrLen(utf8.text);
    if(getLength() != len2)
        return false;
    uint8_t coder1 = getCoder();
    uint8_t coder2 = isLatin1(utf8.text) ? 0 : 1;
    if(coder1 != coder2)
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

bool FlintJavaString::equals(FlintJavaString &str) const {
    if(this == &str)
        return true;
    uint32_t len = getLength();
    if(len != str.getLength())
        return false;
    uint8_t coder = getCoder();
    if(coder != str.getCoder())
        return false;
    const char *value1 = getText();
    const char *value2 = str.getText();
    len <<= coder;
    for(uint32_t i = 0; i < len; i++) {
        if(value1[i] != value2[i])
            return false;
    }
    return true;
}

FlintConstString::FlintConstString(FlintJavaString &flintString) : flintString(flintString) {
    next = 0;
}
