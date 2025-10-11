
#include "stdio.h"
#include "flint.h"
#include "flint_utf8.h"
#include "flint_java_string.h"
#include "flint_fields_data.h"

#define LATIN1          0
#define UTF16           1

static bool IsLatin1(const char *utf8) {
    while(*utf8) {
        if((int8_t)*utf8 < 0) {
            if(Utf8DecodeOneChar(utf8) > 255) return false;
            utf8 += Utf8DecodeSizeOneChar(*utf8);
        }
        else utf8++;
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
    int32_t strLen = vsnprintf(NULL, 0, format, args);
    JByteArray *value = (JByteArray *)Flint::newArray(ctx, Flint::findClass(ctx, "[B"), strLen);
    if(value == NULL) return false;
    char *data = (char *)value->getData();
    /* print starts from data - 1 to workaround losing the last character when buffer size equals the length of the string to print */
    vsnprintf(data - 1, strLen + 1, format, args);
    for(int32_t i = strLen - 1; i >= 0; i--) data[i] = data[i - 1];
    data[-1] = 0;
    setValue(value);
    setCoder(0);
    setHash(0);
    setHashIsZero(false);
    return true;
}

JByteArray *JString::getValue(void) const {
    return (JByteArray *)getFieldByIndex(0)->getObj();
}

const char *JString::getAscii(void) const {
    return (char *)getValue()->getData();
}

void JString::setValue(JByteArray *value) {
    getFieldByIndex(0)->setObj(value);
}

uint32_t JString::getLength(void) const {
    JInt8Array *byteArray = getValue();
    if(getCoder() == 0)
        return byteArray->getLength();
    else
        return byteArray->getLength() / 2;
}

uint8_t JString::getCoder(void) const {
    return getFieldByIndex(1)->getInt32();
}

void JString::setCoder(uint8_t coder) {
    getFieldByIndex(1)->setInt32(coder);
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
    return getFieldByIndex(2)->getInt32();
}

void JString::setHash(uint32_t hash) {
    getFieldByIndex(2)->setInt32(hash);
}

bool JString::getHashIsZero(void) const {
    return getFieldByIndex(3)->getInt32();
}

void JString::setHashIsZero(bool value) {
    getFieldByIndex(3)->setInt32(value ? 1 : 0);
}
