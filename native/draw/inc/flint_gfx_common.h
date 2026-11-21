
#ifndef __FLINT_GFX_H
#define __FLINT_GFX_H

#include <stddef.h>
#include <stdint.h>

#define GFX_MAX(_a, _b)         ((_a) > (_b) ? (_a) : (_b))
#define GFX_MIN(_a, _b)         ((_a) < (_b) ? (_a) : (_b))
#define GFX_ABS(_a)             ((_a) < 0 ? -(_a) : (_a))
#define GFX_SWAP(_a, _b)        do { decltype(_a) tmp = (_a); (_a) = (_b); (_b) = tmp; } while (0)

class CharInfo {
private:
    const uint32_t unicode;
    const uint8_t width;
    const uint8_t height;
    const int8_t yOff;
    const uint8_t data[];
public:
    inline __attribute__((always_inline)) uint8_t getWidth(void) const {
        return width;
    }

    inline __attribute__((always_inline)) uint8_t getHeight(void) const {
        return height;
    }

    inline __attribute__((always_inline)) int8_t getYOffset(void) const {
        return yOff;
    }

    inline __attribute__((always_inline)) bool getPixel(uint8_t x, uint8_t y) const {
        uint32_t bitIndex = x + y * this->width;
        return (this->data[bitIndex / 8] >> (bitIndex % 8)) & 0x01;
    }
private:
    CharInfo(void) = delete;
    CharInfo(const CharInfo &) = delete;

    friend class Font;
};

class Font {
private:
    const uint32_t count : 16;
    const uint32_t spaceWidth : 8;
    const uint32_t stdHeight : 8;
    const uint32_t vectorTable[];
public:
    uint32_t getCount(void) const;
    uint8_t getSpaceWidth(void) const;
    uint8_t getStdWidth() const;
    uint8_t getStdHeight() const;

    const CharInfo *getChar(uint32_t unicode) const;
private:
    Font(void) = delete;
    Font(const Font &) = delete;
};

class Image {
public:
    const bool hasAlpha;
    const uint16_t width;
    const uint16_t height;
    const void *data;

    constexpr inline __attribute__((always_inline))
    Image(bool hasAlpha, uint16_t width, uint16_t height, const void *data) : hasAlpha(hasAlpha), width(width), height(height), data(data) {

    };
};

#endif /* __FLINT_GFX_H */
