
#include <string.h>
#include "flint_rgb565_gfx_helper.h"

#define BLEND(_pixel, _alpha, _fg) do {                                 \
    uint32_t bg = __builtin_bswap16(*(_pixel));                         \
    bg = (bg | (bg << 16)) & 0x07E0F81F;                                \
    bg += (((_fg) - bg) * (_alpha)) >> 5;                               \
    bg &= 0x07E0F81F;                                                   \
    bg = (bg | (bg >> 16));                                             \
    *(_pixel) = __builtin_bswap16(bg);                                  \
} while(0)

void Rgb565GfxHelper::clear(uint16_t color) {
    if(color == 0)
        memset(data, 0, width * height * 2);
    else {
        uint16_t *p = (uint16_t *)data;
        uint16_t *end = &p[width * height];
        for(; p < end; p++) *p = color;
    }
}

void Rgb565GfxHelper::blendPixel(uint8_t alpha, uint16_t color, int32_t x, int32_t y) {
    if(clipX1 <= x && x <= clipX2 && clipY1 <= y && y <= clipY2) {
        uint32_t fg = __builtin_bswap16(color);
        fg = (fg | (fg << 16)) & 0x07E0F81F;
        uint16_t *p = &((uint16_t *)data)[y * width + x];
        BLEND(p, alpha, fg);
    }
}

void Rgb565GfxHelper::blendHLine(uint8_t alpha, uint16_t color, int32_t x1, int32_t x2, int32_t y) {
    if(clipY1 <= y && y <= clipY2) {
        x1 = GFX_MAX(clipX1, x1);
        x2 = GFX_MIN(clipX2, x2);
        if(x1 > x2) return;

        uint16_t *p = &((uint16_t *)data)[y * width + x1];
        uint16_t *end = &((uint16_t *)data)[y * width + x2];
        if(alpha == 0x1F)
            for(; p <= end; p++) *p = color;
        else {
            uint32_t fg = __builtin_bswap16(color);
            fg = (fg | (fg << 16)) & 0x07E0F81F;
            for(; p <= end; p++) BLEND(p, alpha, fg);
        }
    }
}

void Rgb565GfxHelper::blendVLine(uint8_t alpha, uint16_t color, int32_t y1, int32_t y2, int32_t x) {
    if(clipX1 <= x && x <= clipX2) {
        y1 = GFX_MAX(clipY1, y1);
        y2 = GFX_MIN(clipY2, y2);
        if(y1 > y2) return;

        uint16_t *p = &((uint16_t *)data)[y1 * width + x];
        uint16_t *end = &((uint16_t *)data)[y2 * width + x];
        if(alpha == 0x1F)
            for(; p <= end; p += width) *p = color;
        else {
            uint32_t fg = __builtin_bswap16(color);
            fg = (fg | (fg << 16)) & 0x07E0F81F;
            for(; p <= end; p += width) BLEND(p, alpha, fg);
        }
    }
}

void Rgb565GfxHelper::blendRect(uint8_t alpha, uint16_t color, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    x1 = GFX_MAX(clipX1, x1);
    x2 = GFX_MIN(clipX2, x2);
    if(x1 > x2) return;

    y1 = GFX_MAX(clipY1, y1);
    y2 = GFX_MIN(clipY2, y2);
    if(y1 > y2) return;

    if(alpha == 0x1F) {
        if(x1 == x2) {
            uint16_t *p = &((uint16_t *)data)[y1 * width + x1];
            uint16_t *end = &((uint16_t *)data)[y2 * width + x1];
            for(; p <= end; p += width) *p = color;
        }
        else for(int32_t y = y1; y <= y2; y++) {
            uint16_t *p = &((uint16_t *)data)[y * width + x1];
            uint16_t *end = &((uint16_t *)data)[y * width + x2];
            for(; p <= end; p++) *p = color;
        }
    }
    else {
        uint32_t fg = __builtin_bswap16(color);
        fg = (fg | (fg << 16)) & 0x07E0F81F;
        if(x1 == x2) {
            uint16_t *p = &((uint16_t *)data)[y1 * width + x1];
            uint16_t *end = &((uint16_t *)data)[y2 * width + x1];
            for(; p <= end; p += width) BLEND(p, alpha, fg);
        }
        else for(int32_t y = y1; y <= y2; y++) {
            uint16_t *p = &((uint16_t *)data)[y * width + x1];
            uint16_t *end = &((uint16_t *)data)[y * width + x2];
            for(; p <= end; p++) BLEND(p, alpha, fg);
        }
    }
}
