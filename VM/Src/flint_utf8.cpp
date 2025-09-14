
#include "flint_utf8.h"

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
