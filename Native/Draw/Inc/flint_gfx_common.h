
#ifndef __FLINT_GFX_H
#define __FLINT_GFX_H

#include <stddef.h>
#include <stdint.h>

#define GFX_MAX(_a, _b)         ((_a) > (_b) ? (_a) : (_b))
#define GFX_MIN(_a, _b)         ((_a) < (_b) ? (_a) : (_b))
#define GFX_ABS(_a)             ((_a) < 0 ? -(_a) : (_a))
#define GFX_SWAP(_a, _b)        do { decltype(_a) tmp = (_a); (_a) = (_b); (_b) = tmp; } while(0)

class CharInfo {
private:
    const uint32_t unicode;
    const uint8_t width;
    const uint8_t height;
    const int8_t yOff;
    const uint8_t data[];
public:
    uint8_t getWidth(void) const;
    uint8_t getHeight(void) const;
    int8_t getYOffset(void) const;

    bool getPixel(uint8_t x, uint8_t y) const;
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

typedef enum : uint8_t {
    IMG_RGB332,
    IMG_ARGB222,
    IMG_RGB565,
    IMG_ARGB565,
    IMG_ARGB888,
} ImgFormat;

class Image {
public:
    const ImgFormat format;
    const uint16_t width;
    const uint16_t height;
    const void *data;

    constexpr inline __attribute__((always_inline))
    Image(ImgFormat format, uint16_t width, uint16_t height, const void *data) : format(format), width(width), height(height), data(data) {

    };
};

#endif /* __FLINT_GFX_H */
