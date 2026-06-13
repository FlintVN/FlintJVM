
#ifndef __FLINT_RGB565_GFX_H
#define __FLINT_RGB565_GFX_H

#include "flint_gfx_common.h"

class Rgb565Gfx {
public:
    const int32_t width;
    const int32_t height;
    const int32_t clipX1;
    const int32_t clipY1;
    const int32_t clipX2;
    const int32_t clipY2;
    uint8_t * const data;

    Rgb565Gfx(int32_t w, int32_t h, int32_t clipX1, int32_t clipY1, int32_t clipX2, int32_t clipY2, uint8_t *data);
private:
    void drawCircle(uint32_t color, int32_t thk, int32_t x, int32_t y, uint32_t d);
    void fillCircle(uint32_t color, int32_t x, int32_t y, uint32_t d);
public:
    void clear(uint32_t color);
    void drawLine(uint32_t color, int32_t thk, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
    void drawRect(uint32_t color, int32_t thk, int32_t x, int32_t y, int32_t w, int32_t h);
    void fillRect(uint32_t color, int32_t x, int32_t y, int32_t w, int32_t h);
    void drawEllipse(uint32_t color, int32_t thk, int32_t x, int32_t y, uint32_t w, uint32_t h);
    void fillEllipse(uint32_t color, int32_t x, int32_t y, uint32_t w, uint32_t h);
    void drawRoundRect(uint32_t color, int32_t thk, int32_t x, int32_t y, int32_t w, int32_t h, int32_t r1, int32_t r2, int32_t r3, int32_t r4);
    void fillRoundRect(uint32_t color, int32_t x, int32_t y, int32_t w, int32_t h, int32_t r1, int32_t r2, int32_t r3, int32_t r4);
    void drawImage(Image *img, int32_t x, int32_t y);
    void drawImage(Image *img, int32_t x, int32_t y, uint16_t w, uint16_t h);
    void drawLatin1(uint8_t *str, uint32_t len, Font *font, uint32_t color, int32_t x, int32_t y);
    void drawUTF16(uint8_t *str, uint32_t len, Font *font, uint32_t color, int32_t x, int32_t y);
private:
    Rgb565Gfx(const Rgb565Gfx &) = delete;
};

#endif /* __FLINT_RGB565_GFX_H */
