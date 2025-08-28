
#include "stdio.h"
#include "flint.h"
#include "flint_java_string.h"
#include "flint_fields_data.h"

#define LATIN1          0
#define UTF16           1

static const uint8_t utf8ByteCount[] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6
};

uint8_t Utf8DecodeSizeOneChar(char c) {
    return (c & 0x80) ? utf8ByteCount[((uint8_t)c - 0xC0) & 0xFC] : 1;
}

uint8_t Utf8EncodeSize(uint16_t c) {
    return (c < 0x80) ? 1 : ((c < 0x0800) ? 2 : 3);
}

uint32_t Utf8DecodeOneChar(const char *c) {
    if(*c & 0x80) {
        uint8_t byteCount = Utf8DecodeSizeOneChar(*c);
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

uint8_t Utf8EncodeOneChar(uint16_t c, char *buff) {
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

uint32_t Utf8StrLen(const char *utf8) {
    uint32_t len = 0;
    while(*utf8) {
        utf8 += Utf8DecodeSizeOneChar(*utf8);
        len++;
    }
    return len;
}

static bool IsLatin1(const char *utf8) {
    while(*utf8) {
        if((int8_t)*utf8 < 0) {
            if(Utf8DecodeOneChar(utf8) > 255)
                return false;
            utf8 += Utf8DecodeSizeOneChar(*utf8);
        }
        else
            utf8++;
    }
    return true;
}

bool JString::setUtf8(FExec *ctx, const char *utf8) {
    uint32_t index = 0;
    uint8_t coder = IsLatin1(utf8) ? 0 : 1;
    uint32_t strLen = Utf8StrLen(utf8);
    JByteArray *value = (JByteArray *)Flint::newArray(ctx, Flint::findClass(ctx, "[B"), strLen << coder);
    if(value == NULL) return false;
    uint8_t *valueData = (uint8_t *)value->getData();
    if(coder == 0) while(*utf8) {
        uint32_t c = Utf8DecodeOneChar(utf8);
        valueData[index] = c;
        utf8 += Utf8DecodeSizeOneChar(*utf8);
        index++;
    }
    else while(*utf8) {
        uint32_t c = Utf8DecodeOneChar(utf8);
        valueData[index] = (uint8_t)c;
        valueData[index + 1] = (uint8_t)(c >> 8);
        utf8 += Utf8DecodeSizeOneChar(*utf8);
        index += 2;
    }
    setValue(value);
    setCoder(coder);
    setHash(0);
    setHashIsZero(false);
    return true;
}

bool JString::setAscii(FExec *ctx, const char *format, va_list args) {
    uint32_t strLen = vsnprintf(NULL, 0, format, args);
    JByteArray *value = (JByteArray *)Flint::newArray(ctx, Flint::findClass(ctx, "[B"), strLen);
    if(value == NULL) return false;
    vsprintf((char *)value->getData(), format, args);
    setValue(value);
    setCoder(0);
    setHash(0);
    setHashIsZero(false);
    return true;
}

JByteArray *JString::getValue(void) const {
    return (JInt8Array *)getFields()->getFieldObjByIndex(0)->value;
}

const char *JString::getAscii(void) const {
    return (char *)getValue()->getData();
}

void JString::setValue(JByteArray *value) {
    getFields()->getFieldObjByIndex(0)->value = value;
}

uint32_t JString::getLength(void) const {
    JInt8Array *byteArray = getValue();
    if(getCoder() == 0)
        return byteArray->getLength();
    else
        return byteArray->getLength() / 2;
}

uint8_t JString::getCoder(void) const {
    return getFields()->getField32ByIndex(0)->value;
}

void JString::setCoder(uint8_t coder) {
    getFields()->getField32ByIndex(0)->value = coder;
}

uint32_t JString::getHashCode(void) {
    uint32_t hash = getHash();
    if(hash == 0 && !getHashIsZero()) {
        uint32_t len = getLength();
        uint8_t *val = (uint8_t *)getValue()->getData();
        if(getCoder() == LATIN1) for(uint32_t i = 0; i < len; i++)
            hash = 31 * hash + val[i];
        else for(uint32_t i = 0; i < len; i++) {
            uint32_t index = i << 1;
            uint16_t v = (val[index + 1] << 8) | val[index];
            hash = 31 * hash + v;
        }
        if(hash == 0)
            setHashIsZero(true);
        else
            setHash(hash);
    }
    return hash;
}

uint16_t JString::getCharAt(uint32_t index) const {
    if(getCoder() == 0)
        return ((uint8_t *)getValue()->getData())[index];
    else {
        index <<= 1;
        uint8_t *data = (uint8_t *)getValue()->getData();
        return (data[index + 1] << 8) | data[index];
    }
}

int32_t JString::compareTo(JString *other) const {
    uint32_t len1 = getLength();
    uint32_t len2 = other->getLength();
    uint32_t len = (len1 > len2) ? len1 : len2;
    for(uint32_t i = 0; i < len; i++) {
        uint16_t c1 = getCharAt(i);
        uint16_t c2 = other->getCharAt(i);
        if(c1 != c2)
            return c1 - c2;
    }
    return len1 - len2;
}

int32_t JString::compareTo(const char *utf8, uint16_t length) const {
    uint16_t index1 = 0;
    uint16_t index2 = 0;
    uint32_t len1 = getLength();
    while(index1 < len1 && utf8[index2]) {
        uint16_t c1 = getCharAt(index1);
        uint16_t c2 = (uint16_t)Utf8DecodeOneChar(&utf8[index2]);
        if(c1 != c2)
            return c1 - c2;
        index1++;
        index2 += Utf8DecodeSizeOneChar(utf8[index2]);
    }
    return (index1 < len1) ? 1 : ((utf8[index2] && index2 < length) ? -1 : 0);
}

uint32_t JString::getHash(void) const {
    return getFields()->getField32ByIndex(1)->value;
}

void JString::setHash(uint32_t hash) {
    getFields()->getField32ByIndex(1)->value = hash;
}

bool JString::getHashIsZero(void) const {
    return getFields()->getField32ByIndex(2)->value;
}

void JString::setHashIsZero(bool value) {
    getFields()->getField32ByIndex(2)->value = value ? 1 : 0;
}