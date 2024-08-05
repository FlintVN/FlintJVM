
#include "flint_string.h"
#include "flint_const_name.h"
#include "flint_fields_data.h"

static const uint8_t utf8ByteCount[] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6
};

static const uint32_t stringValueFieldName[] = {
    (uint32_t)"\x05\x00\x1D\x02""value",                /* field name */
    (uint32_t)"\x02\x00\x9D\x00""[B"                    /* field type */
};

static const uint32_t stringCoderFieldName[] = {
    (uint32_t)"\x05\x00\x0D\x02""coder",                /* field name */
    (uint32_t)"\x01\x00\x42\x00""B"                     /* field type */
};

uint8_t FlintString::getUtf8DecodeSize(char c) {
    return (c & 0x80) ? utf8ByteCount[((uint8_t)c - 0xC0) & 0xFC] : 1;
}

uint8_t FlintString::getUtf8EncodeSize(uint16_t c) {
    return (c < 0x80) ? 1 : ((c < 0x0800) ? 2 : 3);
}

uint32_t FlintString::utf8Decode(const char *c) {
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

uint8_t FlintString::utf8Encode(uint16_t c, char *buff) {
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

uint32_t FlintString::utf8StrLen(const char *utf8) {
    uint32_t len = 0;
    while(*utf8) {
        utf8 += getUtf8DecodeSize(*utf8);
        len++;
    }
    return len;
}

uint32_t FlintString::getUft8BuffSize(FlintString &str) {
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

bool FlintString::isLatin1(const char *utf8) {
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

FlintObject *FlintString::getValue(void) const {
    return ((FlintFieldsData *)data)->getFieldObject(*(FlintConstNameAndType *)stringValueFieldName).object;
}

void FlintString::setValue(FlintObject &byteArray) {
    ((FlintFieldsData *)data)->getFieldObject(*(FlintConstNameAndType *)stringValueFieldName).object = &byteArray;
}

const char *FlintString::getText(void) const {
    FlintObject *byteArray = getValue();
    return (const char *)byteArray->data;
}

uint32_t FlintString::getLength(void) const {
    FlintObject *byteArray = getValue();
    if(getCoder() == 0)
        return byteArray->size / sizeof(int8_t);
    else
        return byteArray->size / (sizeof(int8_t) << 1);
}

uint8_t FlintString::getCoder(void) const {
    return ((FlintFieldsData *)data)->getFieldData32(*(FlintConstNameAndType *)stringCoderFieldName).value;
}

void FlintString::setCoder(uint8_t coder) {
    ((FlintFieldsData *)data)->getFieldData32(*(FlintConstNameAndType *)stringCoderFieldName).value = coder;
}

bool FlintString::equals(const char *text, uint32_t length) const {
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

bool FlintString::equals(const FlintConstUtf8 &utf8) const {
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

bool FlintString::equals(FlintString &str) const {
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

FlintConstString::FlintConstString(FlintString &flintString) : flintString(flintString) {
    next = 0;
}
