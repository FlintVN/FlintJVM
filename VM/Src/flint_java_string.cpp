
#include "flint_java_string.h"
#include "flint_const_name_base.h"
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
    return (FlintInt8Array *)getFields().getFieldObjectByIndex(0).object;
}

void FlintJavaString::setValue(FlintInt8Array &byteArray) {
    getFields().getFieldObjectByIndex(0).object = &byteArray;
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
    return getFields().getFieldData32ByIndex(0).value;
}

void FlintJavaString::setCoder(uint8_t coder) {
    getFields().getFieldData32ByIndex(0).value = coder;
}

int32_t FlintJavaString::compareTo(FlintJavaString &another) const {
    if(this == &another)
        return 0;
    uint32_t len1 = getLength();
    if(getLength() != another.getLength())
        return len1 - another.getLength();
    uint8_t coder1 = getCoder();
    if(coder1 != another.getCoder())
        return coder1 ? 1 : -1;
    const char *txt1 = getText();
    const char *txt2 = another.getText();
    if(txt1 == txt2)
        return 0;
    for(uint32_t i = 0; i < len1; i++) {
        if(txt1[i] != txt2[i])
            return txt1[i] - txt2[i];
    }
    return 0;
}

int32_t FlintJavaString::compareTo(const FlintConstUtf8 &utf8) const {
    uint32_t len2 = utf8StrLen(utf8.text);
    if(getLength() != len2)
        return getLength() - len2;
    uint8_t coder2 = isLatin1(utf8.text) ? 0 : 1;
    if(getCoder() != coder2)
        return getCoder() - coder2;
    const char *txt1 = getText();
    const char *txt2 = utf8.text;
    if(txt1 == txt2)
        return 0;
    for(uint32_t i = 0; i < len2; i++) {
        uint16_t c1 = coder2 ? ((uint16_t *)txt1)[i] : txt1[i];
        uint16_t c2 = utf8Decode(txt2);
        if(c1 != c2)
            return c1 - c2;
        txt2 += getUtf8DecodeSize(*txt2);
    }
    return 0;
}
