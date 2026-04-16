
#include <string.h>
#include "flint_default_conf.h"
#include "flint_rgb565_drawing.h"

#if FLINT_API_DRAW_ENABLED

static void Rgb565_DrawHLine(FGfx *g, uint32_t color, uint32_t width, int32_t x1, int32_t x2, int32_t y) {
    width -= 1;
    int32_t half0 = width >> 1;
    int32_t half1 = half0 + (width & 0x1);
    int32_t xmin = F_MAX(g->clipX1, F_MIN(x1, x2 - 1));
    int32_t xmax = F_MIN(g->clipX2, F_MAX(x1, x2 - 1));
    int32_t ymin = F_MAX(g->clipY1, y - half1);
    int32_t ymax = F_MIN(g->clipY2, y + half0);
    uint16_t *data = (uint16_t *)g->data;
    uint8_t alpha = color >> 24;

    if(alpha == 0xFF) {
        uint16_t fg = (uint16_t)color;
        for(uint32_t i = ymin; i <= ymax; i++) {
            uint16_t *buff = &data[i * g->w + xmin];
            uint16_t *end = &data[i * g->w + xmax];
            for(; buff <= end; buff++)
                *buff = fg;
        }
    }
    else {
        alpha >>= 3;
        constexpr uint32_t mask = 0x07E0F81F;
        uint32_t fg = __builtin_bswap16(color);
        fg = (fg | (fg << 16)) & mask;
        for(uint32_t i = ymin; i <= ymax; i++) {
            uint16_t *buff = &data[i * g->w + xmin];
            uint16_t *end = &data[i * g->w + xmax];
            for(; buff <= end; buff++) {
                uint32_t bg = __builtin_bswap16(*buff);
                bg = (bg | (bg << 16)) & mask;
                bg += ((fg - bg) * alpha) >> 5;
                bg &= mask;
                bg = (bg | (bg >> 16));
                *buff = __builtin_bswap16(bg);
            }
        }
    }
}

static void Rgb565_DrawVLine(FGfx *g, uint32_t color, uint32_t width, int32_t y1, int32_t y2, int32_t x) {
    width -= 1;
    int32_t half0 = width >> 1;
    int32_t half1 = half0 + (width & 0x1);
    int32_t ymin = F_MAX(g->clipY1, F_MIN(y1, y2 - 1));
    int32_t ymax = F_MIN(g->clipY2, F_MAX(y1, y2 - 1));
    int32_t xmin = F_MAX(g->clipX1, x - half1);
    int32_t xmax = F_MIN(g->clipX2, x + half0);
    uint16_t *data = (uint16_t *)g->data;
    uint8_t alpha = color >> 24;

    if(alpha == 0xFF) {
        uint16_t fg = (uint16_t)color;
        if(width == 0) for(uint32_t y = ymin; y <= ymax; y++) {
            data[y * g->w + xmin] = fg;
        }
        else for(uint32_t y = ymin; y <= ymax; y++) {
            uint16_t *buff = &data[y * g->w + xmin];
            uint16_t *end = &data[y * g->w + xmax];
            for(; buff <= end; buff++)
                *buff = fg;
        }
    }
    else {
        alpha >>= 3;
        constexpr uint32_t mask = 0x07E0F81F;
        uint32_t fg = __builtin_bswap16(color);
        fg = (fg | (fg << 16)) & mask;
        if(width == 0) for(uint32_t y = ymin; y <= ymax; y++) {
            uint32_t bg = __builtin_bswap16(data[y * g->w + xmin]);
            bg = (bg | (bg << 16)) & mask;
            bg += ((fg - bg) * alpha) >> 5;
            bg &= mask;
            bg = (bg | (bg >> 16));
            data[y * g->w + xmin] = __builtin_bswap16(bg);
        }
        else for(uint32_t y = ymin; y <= ymax; y++) {
            uint16_t *buff = &data[y * g->w + xmin];
            uint16_t *end = &data[y * g->w + xmax];
            for(; buff <= end; buff++) {
                uint32_t bg = __builtin_bswap16(*buff);
                bg = (bg | (bg << 16)) & mask;
                bg += ((fg - bg) * alpha) >> 5;
                bg &= mask;
                bg = (bg | (bg >> 16));
                *buff = __builtin_bswap16(bg);
            }
        }
    }
}

static void Rgb565_DrawSkewLine(FGfx *g, uint32_t color, uint32_t width, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    
}

void Rgb565_DrawLine(FGfx *g, uint32_t color, uint32_t width, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    if(x1 == x2 && y1 == y2) return;

    if(y1 == y2)
        Rgb565_DrawHLine(g, color, width, x1, x2, y1);
    else if(x1 == x2)
        Rgb565_DrawVLine(g, color, width, y1, y2, x1);
    else
        Rgb565_DrawSkewLine(g, color, width, x1, y1, x2, y2);
}

#endif /* FLINT_API_DRAW_ENABLED */
