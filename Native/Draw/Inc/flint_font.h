
#ifndef __FLINT_FONT_H
#define __FLINT_FONT_H

#include <stddef.h>
#include <stdint.h>

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

#endif /* __FLINT_FONT_H */
