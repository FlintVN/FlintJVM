
#ifndef __FLINT_RGB565_GFX_HELPER_H
#define __FLINT_RGB565_GFX_HELPER_H

#include "flint_fixed_point.h"
#include "flint_rgb565_gfx.h"

class Rgb565GfxHelper : public Rgb565Gfx {
public:
    void clear(uint16_t color);

    void blendPixel(uint8_t alpha, uint16_t color, int32_t x, int32_t y);

    void blendHLine(uint8_t alpha, uint16_t color, int32_t x1, int32_t x2, int32_t y);
    void blendVLine(uint8_t alpha, uint16_t color, int32_t y1, int32_t y2, int32_t x);

    void blendRect(uint8_t alpha, uint16_t color, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
};

#endif /* __FLINT_RGB565_GFX_HELPER_H */
