
#include <string.h>
#include "flint_rgb565_gfx.h"

static constexpr uint16_t fpmax = (1 << GFX_F_BITS);
static constexpr uint16_t fphalf = (1 << (GFX_F_BITS - 1));
static constexpr uint16_t fpmask = fpmax - 1;

#define BLEND(_pixel, _alpha, _fg) do {                                 \
    uint32_t bg = __builtin_bswap16(*(_pixel));                         \
    bg = (bg | (bg << 16)) & 0x07E0F81F;                                \
    bg += (((_fg) - bg) * (_alpha)) >> 5;                               \
    bg &= 0x07E0F81F;                                                   \
    bg = (bg | (bg >> 16));                                             \
    *(_pixel) = __builtin_bswap16(bg);                                  \
} while(0)

typedef struct {
    int32_t x;
    int32_t y;
} Point;

static uint64_t ISqrt(uint64_t x) {
    uint64_t g, ret = 1ull << ((63 - __builtin_clzll(x)) >> 1);
    do {
        g = ret;
        ret = (g + x / g) >> 1;
    } while(GFX_ABS((int64_t)(ret - g)) > 1);
    return ret;
}

class Rgb565GfxHelper : public Rgb565Gfx {
public:
    void blendPixel(int32_t x, int32_t y, uint8_t alpha, uint32_t fg);

    void drawHLine(uint32_t color, int32_t x1, int32_t x2, int32_t y);
    void blendHLine(uint8_t alpha, uint32_t fg, int32_t x1, int32_t x2, int32_t y);
    void drawVLine(uint32_t color, int32_t y1, int32_t y2, int32_t x);
    void blendVLine(uint8_t alpha, uint32_t fg, int32_t y1, int32_t y2, int32_t x);

    void fill(uint32_t color, int32_t x1, uint32_t y1, int32_t x2, int32_t y2);

    void fillCornerArc(uint32_t color, int32_t cx, int32_t cy, uint32_t r, uint8_t arc);

    void drawHLine(uint32_t color, uint32_t thickness, int32_t x1, int32_t x2, int32_t y);
    void drawVLine(uint32_t color, uint32_t thickness, int32_t y1, int32_t y2, int32_t x);
    void drawSkewLine(uint32_t color, uint32_t thickness, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

    void drawChar(const CharInfo *c, uint32_t color, int32_t x, int32_t y);
};

void Rgb565GfxHelper::blendPixel(int32_t x, int32_t y, uint8_t alpha, uint32_t fg) {
    if(clipX1 <= x && x <= clipX2 && clipY1 <= y && y <= clipY2) {
        uint16_t *p = &((uint16_t *)data)[y * w + x];
        BLEND(p, alpha, fg);
    }
}

void Rgb565GfxHelper::drawHLine(uint32_t color, int32_t x1, int32_t x2, int32_t y) {
    if(clipY1 <= y && y <= clipY2) {
        x1 = GFX_MAX(clipX1, x1);
        x2 = GFX_MIN(clipX2, x2);
        if(x1 > x2) return;

        uint16_t *p = &((uint16_t *)data)[y * w + x1];
        uint16_t *end = &((uint16_t *)data)[y * w + x2];
        for(; p <= end; p++) *p = color;
    }
}

void Rgb565GfxHelper::blendHLine(uint8_t alpha, uint32_t fg, int32_t x1, int32_t x2, int32_t y) {
    if(clipY1 <= y && y <= clipY2) {
        x1 = GFX_MAX(clipX1, x1);
        x2 = GFX_MIN(clipX2, x2);
        if(x1 > x2) return;

        uint16_t *p = &((uint16_t *)data)[y * w + x1];
        uint16_t *end = &((uint16_t *)data)[y * w + x2];
        for(; p <= end; p++) BLEND(p, alpha, fg);
    }
}

void Rgb565GfxHelper::drawVLine(uint32_t color, int32_t y1, int32_t y2, int32_t x) {
    if(clipX1 <= x && x <= clipX2) {
        y1 = GFX_MAX(clipY1, y1);
        y2 = GFX_MIN(clipY2, y2);
        if(y1 > y2) return;

        uint16_t *p = &((uint16_t *)data)[y1 * w + x];
        uint16_t *end = &((uint16_t *)data)[y2 * w + x];
        for(; p <= end; p += w) *p = color;
    }
}

void Rgb565GfxHelper::blendVLine(uint8_t alpha, uint32_t fg, int32_t y1, int32_t y2, int32_t x) {
    if(clipX1 <= x && x <= clipX2) {
        y1 = GFX_MAX(clipY1, y1);
        y2 = GFX_MIN(clipY2, y2);
        if(y1 > y2) return;

        uint16_t *p = &((uint16_t *)data)[y1 * w + x];
        uint16_t *end = &((uint16_t *)data)[y2 * w + x];
        for(; p <= end; p += w) BLEND(p, alpha, fg);
    }
}

void Rgb565GfxHelper::fill(uint32_t color, int32_t x1, uint32_t y1, int32_t x2, int32_t y2) {
    x1 = GFX_MAX(clipX1, x1);
    x2 = GFX_MIN(clipX2, x2);
    if(x1 > x2) return;

    y1 = GFX_MAX(clipY1, y1);
    y2 = GFX_MIN(clipY2, y2);
    if(y1 > y2) return;

    uint8_t alpha = color >> 27;

    if(alpha == 0x1F) {
        if(x1 == x2) {
            uint16_t *p = &((uint16_t *)data)[y1 * w + x1];
            uint16_t *end = &((uint16_t *)data)[y2 * w + x1];
            for(; p <= end; p += w) *p = color;
        }
        else for(int32_t y = y1; y <= y2; y++) {
            uint16_t *p = &((uint16_t *)data)[y * w + x1];
            uint16_t *end = &((uint16_t *)data)[y * w + x2];
            for(; p <= end; p++) *p = color;
        }
    }
    else {
        uint32_t fg = __builtin_bswap16(color);
        fg = (fg | (fg << 16)) & 0x07E0F81F;
        if(x1 == x2) {
            uint16_t *p = &((uint16_t *)data)[y1 * w + x1];
            uint16_t *end = &((uint16_t *)data)[y2 * w + x1];
            for(; p <= end; p += w) BLEND(p, alpha, fg);
        }
        else for(int32_t y = y1; y <= y2; y++) {
            uint16_t *p = &((uint16_t *)data)[y * w + x1];
            uint16_t *end = &((uint16_t *)data)[y * w + x2];
            for(; p <= end; p++) BLEND(p, alpha, fg);
        }
    }
}

void Rgb565GfxHelper::fillCornerArc(uint32_t color, int32_t cx, int32_t cy, uint32_t r, uint8_t arc) {
    int32_t x = r;
    int32_t y = (arc < 2) ? 0 : 1;
    int64_t rr = (int64_t)r * r;
    uint8_t x0 = (arc == 0 || arc == 3) ? 0 : 1;

    uint8_t alpha = color >> 27;
    uint32_t fg = __builtin_bswap16(color);
    fg = (fg | (fg << 16)) & 0x07E0F81F;

    if(alpha == 0x1F) {
        for(; y < x; y++) {
            int64_t fx = ISqrt((rr - (int64_t)y * y) << (GFX_F_BITS << 1));
            uint8_t a = ((uint32_t)fx & fpmask) * alpha / fpmax;
            x = (int32_t)(fx >> GFX_F_BITS);
            int32_t py = (arc < 2) ? (cy - y) : (cy + y);
            if(arc == 0 || arc == 3) {
                ((Rgb565GfxHelper *)this)->drawHLine(color, cx - x, cx, py);
                if(a > 0) ((Rgb565GfxHelper *)this)->blendPixel(cx - x - 1, py, a, fg);
            }
            else {
                ((Rgb565GfxHelper *)this)->drawHLine(color, cx + x0, cx + x, py);
                if(a > 0) ((Rgb565GfxHelper *)this)->blendPixel(cx + x + 1, py, a, fg);
            }
        }
        int32_t y0 = y;
        for(; x >= x0; x--) {
            int64_t fy = ISqrt((rr - (int64_t)x * x) << (GFX_F_BITS << 1));
            uint8_t a = ((uint32_t)fy & fpmask) * alpha / fpmax;
            y = (int32_t)(fy >> GFX_F_BITS);
            int32_t px = (arc == 0 || arc == 3) ? (cx - x) : (cx + x);
            if(arc < 2) {
                ((Rgb565GfxHelper *)this)->drawVLine(color, cy - y, cy - y0, px);
                if(a > 0) ((Rgb565GfxHelper *)this)->blendPixel(px, cy - y - 1, a, fg);
            }
            else {
                ((Rgb565GfxHelper *)this)->drawVLine(color, cy + y0, cy + y, px);
                if(a > 0) ((Rgb565GfxHelper *)this)->blendPixel(px, cy + y + 1, a, fg);
            }
        }
    }
    else {
        for(; y < x; y++) {
            int64_t fx = ISqrt((rr - (int64_t)y * y) << (GFX_F_BITS << 1));
            uint8_t a = ((uint32_t)fx & fpmask) * alpha / fpmax;
            x = (int32_t)(fx >> GFX_F_BITS);
            int32_t py = (arc < 2) ? (cy - y) : (cy + y);
            if(arc == 0 || arc == 3) {
                ((Rgb565GfxHelper *)this)->blendHLine(alpha, fg, cx - x, cx, py);
                if(a > 0) ((Rgb565GfxHelper *)this)->blendPixel(cx - x - 1, py, a, fg);
            }
            else {
                ((Rgb565GfxHelper *)this)->blendHLine(alpha, fg, cx + x0, cx + x, py);
                if(a > 0) ((Rgb565GfxHelper *)this)->blendPixel(cx + x + 1, py, a, fg);
            }
        }
        int32_t y0 = y;
        for(; x >= x0; x--) {
            int64_t fy = ISqrt((rr - (int64_t)x * x) << (GFX_F_BITS << 1));
            uint8_t a = ((uint32_t)fy & fpmask) * alpha / fpmax;
            y = (int32_t)(fy >> GFX_F_BITS);
            int32_t px = (arc == 0 || arc == 3) ? (cx - x) : (cx + x);
            if(arc < 2) {
                ((Rgb565GfxHelper *)this)->blendVLine(alpha, fg, cy - y, cy - y0, px);
                if(a > 0) ((Rgb565GfxHelper *)this)->blendPixel(px, cy - y - 1, a, fg);
            }
            else {
                ((Rgb565GfxHelper *)this)->blendVLine(alpha, fg, cy + y0, cy + y, px);
                if(a > 0) ((Rgb565GfxHelper *)this)->blendPixel(px, cy + y + 1, a, fg);
            }
        }
    }
}

void Rgb565GfxHelper::drawHLine(uint32_t color, uint32_t thickness, int32_t x1, int32_t x2, int32_t y) {
    if(x1 > x2) GFX_SWAP(x1, x2);
    int32_t half = (thickness - 1) >> 1;
    int32_t y1 = y - half;
    int32_t y2 = y + half;

    if((thickness & 0x01) == 0) {
        uint32_t a1 = color >> 28;
        uint32_t a2 = color >> 29;
        uint32_t fg = __builtin_bswap16(color);
        fg = (fg | (fg << 16)) & 0x07E0F81F;

        blendVLine(a1, fg, y1, y2, x1);
        blendVLine(a1, fg, y1, y2, x2);

        blendHLine(a1, fg, x1 + 1, x2 - 1, y1 - 1);
        blendHLine(a1, fg, x1 + 1, x2 - 1, y2 + 1);

        blendPixel(x1, y1 - 1, a2, fg);
        blendPixel(x2, y1 - 1, a2, fg);
        blendPixel(x1, y2 + 1, a2, fg);
        blendPixel(x2, y2 + 1, a2, fg);

        x1++;
        x2--;
    }
    fill(color, x1, y1, x2, y2);
}

void Rgb565GfxHelper::drawVLine(uint32_t color, uint32_t thickness, int32_t y1, int32_t y2, int32_t x) {
    if(y1 > y2) GFX_SWAP(y1, y2);
    int32_t half = (thickness - 1) >> 1;
    int32_t x1 = x - half;
    int32_t x2 = x + half;

    if((thickness & 0x01) == 0) {
        uint32_t a1 = color >> 28;
        uint32_t a2 = color >> 29;
        uint32_t fg = __builtin_bswap16(color);
        fg = (fg | (fg << 16)) & 0x07E0F81F;

        blendHLine(a1, fg, x1, x2, y1);
        blendHLine(a1, fg, x1, x2, y2);

        blendVLine(a1, fg, y1 + 1, y2 - 1, x1 - 1);
        blendVLine(a1, fg, y1 + 1, y2 - 1, x2 + 1);

        blendPixel(x1 - 1, y1, a2, fg);
        blendPixel(x2 + 1, y1, a2, fg);
        blendPixel(x1 - 1, y2, a2, fg);
        blendPixel(x2 + 1, y2, a2, fg);

        y1++;
        y2--;
    }
    fill(color, x1, y1, x2, y2);
}

static void CalcRectPoints(uint32_t width, int32_t x1, int32_t y1, int32_t x2, int32_t y2, Point *points) {
    static const uint8_t wratio[] = {
        255, 251, 247, 244, 240, 237, 234, 231, 228, 225, 222, 220, 217, 215, 212, 210,
        208, 206, 203, 201, 199, 198, 196, 194, 192, 190, 189, 187, 186, 184, 182, 181,
        180, 178, 177, 176, 174, 173, 172, 170, 169, 168, 167, 166, 165, 164, 163, 162,
        161, 160, 159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 150, 149, 148, 147,
        255
    };
    int32_t dx = GFX_ABS(x2 - x1);
    int32_t dy = GFX_ABS(y2 - y1);
    int32_t wx, wy;

    if(dx > dy) {
        wy = (width * wratio[(dy << 6) / dx] + 127) >> 8;
        wx = (wy * dy + dx / 2) / dx;
    }
    else {
        wx = (width * wratio[(dx << 6) / dy] + 127) >> 8;
        wy = (wx * dx + dy / 2) / dy;
    }
    if(x1 > x2) {
        GFX_SWAP(x1, x2);
        GFX_SWAP(y1, y2);
    }

    int32_t wy0 = wy >> 1;
    int32_t wy1 = wy0 + (wy & 0x1);  
    int32_t wx0 = wx >> 1;
    int32_t wx1 = wx0 + (wx & 0x01);

    if(y1 < y2) {
        points[0] = {x1 - wx0, y1 + wy1};
        points[1] = {x1 + wx1, y1 - wy0};
        points[2] = {x2 + wx1, y2 - wy0};
        points[3] = {x2 - wx0, y2 + wy1};
    }
    else {
        points[0] = {x1 - wx0, y1 - wy0};
        points[1] = {x2 - wx0, y2 - wy0};
        points[2] = {x2 + wx1, y2 + wy1};
        points[3] = {x1 + wx1, y1 + wy1};
    }
}

void Rgb565GfxHelper::drawSkewLine(uint32_t color, uint32_t thickness, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    uint8_t alpha = color >> 27;
    uint32_t fg = __builtin_bswap16(color);
    fg = (fg | (fg << 16)) & 0x07E0F81F;

    if(thickness == 1) {
        int32_t err = 0;
        int32_t dx = GFX_ABS(x2 - x1);
        int32_t dy = GFX_ABS(y2 - y1);
        if(dx > dy) {
            int8_t ystep = (y1 < y2) ? 1 : -1;
            int32_t y = y1;
            if(x1 > x2) {
                GFX_SWAP(x1, x2);
                GFX_SWAP(y1, y2);
            }
            for(int32_t x = x1; x <= x2; x++) {
                int32_t a = err * alpha / dx;
                blendPixel(x, y, alpha - a, fg);
                blendPixel(x, y - ystep, a, fg);
                err -= dy;
                if(err < 0) {
                    y += ystep;
                    err += dx;
                }
            }
        }
        else {
            int8_t xstep = (x1 < x2) ? 1 : -1;
            int32_t x = x1;
            if(y1 > y2) {
                GFX_SWAP(x1, x2);
                GFX_SWAP(y1, y2);
            }
            for(int32_t y = y1; y <= y2; y++) {
                int32_t a = err * alpha / dy;
                blendPixel(x, y, alpha - a, fg);
                blendPixel(x - xstep, y, a, fg);
                err -= dx;
                if(err < 0) {
                    x += xstep;
                    err += dy;
                }
            }
        }
    }
    else {
        Point p[4];
        CalcRectPoints(thickness, x1, y1, x2, y2, p);

        int32_t ymax = GFX_MIN(this->clipY2, GFX_MIN(p[0].y, p[2].y));
        int32_t y = GFX_MAX(this->clipY1, p[1].y);

        int32_t dx1 = p[1].x - p[0].x;
        int32_t dy1 = p[0].y - p[1].y;
        int32_t dx2 = p[2].x - p[1].x;
        int32_t dy2 = p[2].y - p[1].y;
        uint8_t astep1 = (alpha * dy1 + (dx1 >> 1)) / dx1;
        uint8_t astep2 = (alpha * dy2 + (dx2 >> 1)) / dx2;

        for(; y <= ymax; y++) {
            int32_t x1 = (p[0].y - y) * dx1;
            int32_t fx1 = fpmax - (((x1 << GFX_F_BITS) / dy1) & fpmask);
            x1 = p[0].x + x1 / dy1;

            int32_t x2 = (y - p[1].y) * dx2 + dy2 - 1;
            int32_t fx2 = ((x2 << GFX_F_BITS) / dy2) & fpmask;
            x2 = p[1].x + x2 / dy2;

            int16_t a = (dx1 < dy1) ? (fx1 * alpha / fpmax) : (alpha - astep1 + fx1 * dy1 / dx1 * alpha / fpmax);
            for(uint32_t i = x1++; a > 0 && i >= p[0].x; i--) {
                blendPixel(i, y, a, fg);
                a -= astep1;
            }

            a = (dx2 < dy2) ? (fx2 * alpha / fpmax) : (alpha - astep2 + fx2 * dy2 / dx2 * alpha / fpmax);
            for(uint32_t i = x2--; a > 0 && i <= p[2].x; i++) {
                blendPixel(i, y, a, fg);
                a -= astep2;
            }
            if(alpha == 0x1F) drawHLine(color, x1, x2, y);
            else blendHLine(alpha, fg, x1, x2, y);
        }
        ymax = GFX_MIN(this->clipY2, GFX_MAX(p[0].y, p[2].y));
        if(p[0].y < p[2].y) for(; y < ymax; y++) {
            int32_t x1 = (y - p[0].y) * dx2;
            int32_t fx1 = fpmax - (((x1 << GFX_F_BITS) / dy2) & fpmask);
            x1 = p[0].x + x1 / dy2;

            int32_t x2 = (y - p[1].y) * dx2 + dy2 - 1;
            int32_t fx2 = ((x2 << GFX_F_BITS) / dy2) & fpmask;
            x2 = p[1].x + x2 / dy2;

            int16_t a = (dx2 < dy2) ? (fx1 * alpha / fpmax) : (alpha - astep2 + fx1 * dy2 / dx2 * alpha / fpmax);
            for(uint32_t i = x1++; a > 0 && i >= p[0].x; i--) {
                blendPixel(i, y, a, fg);
                a -= astep2;
            }

            a = (dx2 < dy2) ? (fx2 * alpha / fpmax) : (alpha - astep2 + fx2 * dy2 / dx2 * alpha / fpmax);
            for(uint32_t i = x2--; a > 0 && i <= p[2].x; i++) {
                blendPixel(i, y, a, fg);
                a -= astep2;
            }

            if(alpha == 0x1F) drawHLine(color, x1, x2, y);
            else blendHLine(alpha, fg, x1, x2, y);
        }
        else for(; y < ymax; y++) {
            int32_t x1 = (p[0].y - y) * dx1;
            int32_t fx1 = fpmax - (((x1 << GFX_F_BITS) / dy1) & fpmask);
            x1 = p[0].x + x1 / dy1;

            int32_t x2 = (p[3].y - y) * dx1 + dy1 - 1;
            int32_t fx2 = ((x2 << GFX_F_BITS) / dy1) & fpmask;
            x2 = p[3].x + x2 / dy1;

            int16_t a = (dx1 < dy1) ? (fx1 * alpha / fpmax) : (alpha - astep1 + fx1 * dy1 / dx1 * alpha / fpmax);
            for(uint32_t i = x1++; a > 0 && i >= p[0].x; i--) {
                blendPixel(i, y, a, fg);
                a -= astep1;
            }

            a = (dx1 < dy1) ? (fx2 * alpha / fpmax) : (alpha - astep1 + fx2 * dy1 / dx1 * alpha / fpmax);
            for(uint32_t i = x2--; a > 0 && i <= p[2].x; i++) {
                blendPixel(i, y, a, fg);
                a -= astep1;
            }

            if(alpha == 0x1F) drawHLine(color, x1, x2, y);
            else blendHLine(alpha, fg, x1, x2, y);
        }
        ymax = GFX_MIN(this->clipY2, p[3].y);
        for(; y <= ymax; y++) {
            int32_t x1 = (y - p[0].y) * dx2;
            int32_t fx1 = fpmax - (((x1 << GFX_F_BITS) / dy2) & fpmask);
            x1 = p[0].x + x1 / dy2;

            int32_t x2 = (p[3].y - y) * dx1 + dy1 - 1;
            int32_t fx2 = ((x2 << GFX_F_BITS) / dy1) & fpmask;
            x2 = p[3].x + x2 / dy1;

            int16_t a = (dx2 < dy2) ? (fx1 * alpha / fpmax) : (alpha - astep2 + fx1 * dy2 / dx2 * alpha / fpmax);
            for(uint32_t i = x1++; a > 0 && i >= p[0].x; i--) {
                blendPixel(i, y, a, fg);
                a -= astep2;
            }

            a = (dx1 < dy1) ? (fx2 * alpha / fpmax) : (alpha - astep1 + fx2 * dy1 / dx1 * alpha / fpmax);
            for(uint32_t i = x2--; a > 0 && i <= p[2].x; i++) {
                blendPixel(i, y, a, fg);
                a -= astep1;
            }
            if(alpha == 0x1F) drawHLine(color, x1, x2, y);
            else blendHLine(alpha, fg, x1, x2, y);
        }
    }
}

void Rgb565GfxHelper::drawChar(const CharInfo *c, uint32_t color, int32_t x, int32_t y) {
    uint8_t width = c->getWidth();

    int32_t cx0 = GFX_MAX(clipX1 - x, 0);
    int32_t cxw = GFX_MIN(clipX2 - x + 1, width);
    if(cx0 >= cxw) return;

    uint8_t height = c->getHeight();
    int8_t yOff = c->getYOffset();
    uint8_t alpha = color >> 27;
    uint32_t fg = __builtin_bswap16(color);
    fg = (fg | (fg << 16)) & 0x07E0F81F;

    if(alpha == 0x1F) {
        for(int32_t cy = 0; cy < height; cy++) {
            int32_t py = y + cy + yOff;
            if(!(clipY1 <= y && y <= clipY2)) continue;
            uint16_t *p = &((uint16_t *)data)[py * w + x];
            for(int32_t cx = cx0; cx < cxw; cx++)
                if(c->getPixel(cx, cy)) p[cx] = color;
        }
    }
    else {
        for(int32_t cy = 0; cy < height; cy++) {
            int32_t py = y + cy + yOff;
            if(!(clipY1 <= y && y <= clipY2)) continue;
            for(int32_t cx = cx0; cx < cxw; cx++)
                if(c->getPixel(cx, cy)) blendPixel(x + cx, py, alpha, fg);
        }
    }
}

Rgb565Gfx::Rgb565Gfx(int32_t w, int32_t h, int32_t clipX1, int32_t clipY1, int32_t clipX2, int32_t clipY2, uint8_t *data) :
w(w), h(h), clipX1(clipX1), clipY1(clipY1), clipX2(clipX2), clipY2(clipY2), data(data) {

}

void Rgb565Gfx::clear(uint32_t color) {
    if(color == 0)
        memset(data, 0, w * h * 2);
    else {
        uint16_t *p = (uint16_t *)data;
        uint16_t *end = &p[w * h];
        for(; p < end; p++) *p = color;
    }
}

void Rgb565Gfx::drawLine(uint32_t color, uint32_t thickness, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    if(x1 == x2 && y1 == y2)
        return;
    else if(y1 == y2)
        ((Rgb565GfxHelper *)this)->drawHLine(color, thickness, x1, x2, y1);
    else if(x1 == x2)
        ((Rgb565GfxHelper *)this)->drawVLine(color, thickness, y1, y2, x1);
    else
        ((Rgb565GfxHelper *)this)->drawSkewLine(color, thickness, x1, y1, x2, y2);
}

void Rgb565Gfx::drawRect(uint32_t color, uint32_t thickness, int32_t x, int32_t y, int32_t w, int32_t h) {
    int32_t half = (thickness - 1) >> 1;

    int32_t x1 = x;
    int32_t x2 = x + w;
    int32_t y1 = y;
    int32_t y2 = y + h;

    if((thickness & 0x01) == 0) {
        uint8_t a = color >> 28;
        uint32_t fg = __builtin_bswap16(color);
        fg = (fg | (fg << 16)) & 0x07E0F81F;

        ((Rgb565GfxHelper *)this)->blendHLine(a, fg, x1 - half - 1, x2 + half + 1, y1 - half - 1);
        ((Rgb565GfxHelper *)this)->blendHLine(a, fg, x1 - half - 1, x2 + half + 1, y2 + half + 1);

        ((Rgb565GfxHelper *)this)->blendHLine(a, fg, x1 + half + 1, x2 - half - 1, y1 + half + 1);
        ((Rgb565GfxHelper *)this)->blendHLine(a, fg, x1 + half + 1, x2 - half - 1, y2 - half - 1);

        ((Rgb565GfxHelper *)this)->blendVLine(a, fg, y1 - half, y2 + half, x1 - half - 1);
        ((Rgb565GfxHelper *)this)->blendVLine(a, fg, y1 - half, y2 + half, x2 + half + 1);

        ((Rgb565GfxHelper *)this)->blendVLine(a, fg, y1 + half + 1, y2 - half - 1, x1 + half + 1);
        ((Rgb565GfxHelper *)this)->blendVLine(a, fg, y1 + half + 1, y2 - half - 1, x2 - half - 1);

        thickness--;
    }

    ((Rgb565GfxHelper *)this)->fill(color, x1 - half, y1 - half, x2 + half, y1 + half);
    ((Rgb565GfxHelper *)this)->fill(color, x1 - half, y2 - half, x2 + half, y2 + half);

    ((Rgb565GfxHelper *)this)->fill(color, x1 - half, y1 + half + 1, x1 - half, y2 - half - 1);
    ((Rgb565GfxHelper *)this)->fill(color, x2 + half, y1 + half + 1, x2 + half, y2 - half - 1);
}

void Rgb565Gfx::fillRect(uint32_t color, int32_t x, int32_t y, int32_t w, int32_t h) {
    ((Rgb565GfxHelper *)this)->fill(color, x, y, x + w, y + h);
}

void Rgb565Gfx::fillRoundRect(uint32_t color, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4) {
    uint8_t alpha = color >> 27;

    uint8_t scale = 128;
    if(r1 + r2 > w) scale = (w << 7) / (r1 + r2);
    if(r3 + r4 > w) scale = GFX_MIN(scale, (w << 7) / (r3 + r4));
    if(r1 + r4 > h) scale = GFX_MIN(scale, (h << 7) / (r1 + r4));
    if(r2 + r3 > h) scale = GFX_MIN(scale, (h << 7) / (r2 + r3));
    if(scale < 128) {
        r1 = r1 * scale >> 7;
        r2 = r2 * scale >> 7;
        r3 = r3 * scale >> 7;
        r4 = r4 * scale >> 7;
    }

    int32_t x1, x2;
    int32_t y1 = GFX_MAX(clipY1 - y, 0);
    int32_t y2 = GFX_MIN(clipY2 - y, h - 1);

    if(alpha == 0x1F) for(int32_t i = y1; i <= y2; i++) {
        x1 = x + (i <= r1 ? (r1 + 1) : (i > (h - r4 - 1) ? (r4 + 1) : 0));
        x2 = x + w - 1 - (i <= r2 ? r2 : (i > (h - r3 - 1) ? r3 : 0));
        ((Rgb565GfxHelper *)this)->drawHLine(color, x1, x2, i + y);
    }
    else {
        uint32_t fg = __builtin_bswap16(color);
        fg = (fg | (fg << 16)) & 0x07E0F81F;
        for(int32_t i = y1; i <= y2; i++) {
            x1 = x + (i <= r1 ? (r1 + 1) : (i > (h - r4 - 1) ? (r4 + 1) : 0));
            x2 = x + w - 1 - (i <= r2 ? r2 : (i > (h - r3 - 1) ? r3 : 0));
            ((Rgb565GfxHelper *)this)->blendHLine(alpha, fg, x1, x2, i + y);
        }
    }

    x2 = x + w - 1;
    y2 = y + h - 1;
    if(r1 > 0) ((Rgb565GfxHelper *)this)->fillCornerArc(color, x + r1, y + r1, r1, 0);
    if(r2 > 0) ((Rgb565GfxHelper *)this)->fillCornerArc(color, x2 - r2, y + r2, r2, 1);
    if(r3 > 0) ((Rgb565GfxHelper *)this)->fillCornerArc(color, x2 - r3, y2 - r3, r3, 2);
    if(r4 > 0) ((Rgb565GfxHelper *)this)->fillCornerArc(color, x + r4, y2 - r4, r4, 3);
}

void Rgb565Gfx::fillEllipse(uint32_t color, int32_t x, int32_t y, uint32_t w, uint32_t h) {
    uint8_t alpha = color >> 27;
    uint32_t fg = __builtin_bswap16(color);
    fg = (fg | (fg << 16)) & 0x07E0F81F;

    int32_t cx = x + w / 2;
    int32_t cy = y + h / 2;

    uint64_t ww4 = ((uint64_t)w * w) << 2;
    uint64_t hh4 = ((uint64_t)h * h) << 2;
    uint64_t wh2 = (uint64_t)w * w * h * h;

    if(alpha == 0x1F) {
        if(w & 0x01)
            ((Rgb565GfxHelper *)this)->drawHLine(color, x, x + w - 1, cy);
        else {
            ((Rgb565GfxHelper *)this)->drawHLine(color, x + 1, x + w -1, cy);
            ((Rgb565GfxHelper *)this)->blendPixel(x, cy, 16, fg);
            ((Rgb565GfxHelper *)this)->blendPixel(x + w, cy, 16, fg);
        }
        int32_t ix = w, iy = 1;
        uint16_t off = (w & 0x01) ? 0 : fphalf;
        for(; hh4 * ix >= ww4 * iy; iy++) {
            int64_t fx = ISqrt(((wh2 - ww4 * iy * iy) / hh4) << (GFX_F_BITS << 1)) - off;
            ix = (int32_t)(fx >> GFX_F_BITS);
            uint8_t al = ((uint32_t)fx & fpmask) * 31 / fpmax;
            int32_t x1 = cx - ix;
            int32_t x2 = cx + ix;
            ((Rgb565GfxHelper *)this)->drawHLine(color, x1, x2, cy - iy);
            ((Rgb565GfxHelper *)this)->drawHLine(color, x1, x2, cy + iy);
            if(al > 0) {
                ((Rgb565GfxHelper *)this)->blendPixel(x1 - 1, cy - iy, al, fg);
                ((Rgb565GfxHelper *)this)->blendPixel(x2 + 1, cy - iy, al, fg);
                ((Rgb565GfxHelper *)this)->blendPixel(x1 - 1, cy + iy, al, fg);
                ((Rgb565GfxHelper *)this)->blendPixel(x2 + 1, cy + iy, al, fg);
            }
        }
        off = (h & 0x01) ? 0 : fphalf;
        for(; ix > 0; ix--) {
            int64_t fy = ISqrt(((wh2 - hh4 * ix * ix) / ww4) << (GFX_F_BITS << 1)) - off;
            int32_t i = (int32_t)(fy >> GFX_F_BITS);
            uint8_t al = ((uint32_t)fy & fpmask) * 31 / fpmax;
            int32_t y1 = cy - i;
            int32_t y2 = cy + i;
            ((Rgb565GfxHelper *)this)->drawVLine(color, y1, cy - iy, cx - ix);
            ((Rgb565GfxHelper *)this)->drawVLine(color, cy + iy, y2, cx - ix);
            ((Rgb565GfxHelper *)this)->drawVLine(color, y1, cy - iy, cx + ix);
            ((Rgb565GfxHelper *)this)->drawVLine(color, cy + iy, y2, cx + ix);
            if(al > 0) {
                ((Rgb565GfxHelper *)this)->blendPixel(cx - ix, y1 - 1, al, fg);
                ((Rgb565GfxHelper *)this)->blendPixel(cx + ix, y1 - 1, al, fg);
                ((Rgb565GfxHelper *)this)->blendPixel(cx - ix, y2 + 1, al, fg);
                ((Rgb565GfxHelper *)this)->blendPixel(cx + ix, y2 + 1, al, fg);
            }
        }
        if(h & 0x01) {
            ((Rgb565GfxHelper *)this)->drawVLine(color, y, cy - iy, cx);
            ((Rgb565GfxHelper *)this)->drawVLine(color, cy + iy, y + h - 1, cx);
        }
        else {
            ((Rgb565GfxHelper *)this)->drawVLine(color, y + 1, cy - iy, cx);
            ((Rgb565GfxHelper *)this)->drawVLine(color, cy + iy, y + h - 1, cx);
            ((Rgb565GfxHelper *)this)->blendPixel(cx, y, 16, fg);
            ((Rgb565GfxHelper *)this)->blendPixel(cx, y + h, 16, fg);
        }
    }
    else {
        if(w & 0x01)
            ((Rgb565GfxHelper *)this)->blendHLine(alpha, fg, x, x + w - 1, cy);
        else {
            uint8_t al = (alpha + 1) / 2;
            ((Rgb565GfxHelper *)this)->blendHLine(alpha, fg, x + 1, x + w -1, cy);
            ((Rgb565GfxHelper *)this)->blendPixel(x, cy, al, fg);
            ((Rgb565GfxHelper *)this)->blendPixel(x + w, cy, al, fg);
        }
        int32_t ix = w, iy = 1;
        uint16_t off = (w & 0x01) ? 0 : fphalf;
        for(; hh4 * ix >= ww4 * iy; iy++) {
            int64_t fx = ISqrt(((wh2 - ww4 * iy * iy) / hh4) << (GFX_F_BITS << 1)) - off;
            ix = (int32_t)(fx >> GFX_F_BITS);
            uint8_t al = ((uint32_t)fx & fpmask) * alpha / fpmax;
            int32_t x1 = cx - ix;
            int32_t x2 = cx + ix;
            ((Rgb565GfxHelper *)this)->blendHLine(alpha, fg, x1, x2, cy - iy);
            ((Rgb565GfxHelper *)this)->blendHLine(alpha, fg, x1, x2, cy + iy);
            if(al > 0) {
                ((Rgb565GfxHelper *)this)->blendPixel(x1 - 1, cy - iy, al, fg);
                ((Rgb565GfxHelper *)this)->blendPixel(x2 + 1, cy - iy, al, fg);
                ((Rgb565GfxHelper *)this)->blendPixel(x1 - 1, cy + iy, al, fg);
                ((Rgb565GfxHelper *)this)->blendPixel(x2 + 1, cy + iy, al, fg);
            }
        }
        off = (h & 0x01) ? 0 : fphalf;
        for(; ix > 0; ix--) {
            int64_t fy = ISqrt(((wh2 - hh4 * ix * ix) / ww4) << (GFX_F_BITS << 1)) - off;
            int32_t i = (int32_t)(fy >> GFX_F_BITS);
            uint8_t al = ((uint32_t)fy & fpmask) * alpha / fpmax;
            int32_t y1 = cy - i;
            int32_t y2 = cy + i;
            ((Rgb565GfxHelper *)this)->blendVLine(alpha, fg, y1, cy - iy, cx - ix);
            ((Rgb565GfxHelper *)this)->blendVLine(alpha, fg, cy + iy, y2, cx - ix);
            ((Rgb565GfxHelper *)this)->blendVLine(alpha, fg, y1, cy - iy, cx + ix);
            ((Rgb565GfxHelper *)this)->blendVLine(alpha, fg, cy + iy, y2, cx + ix);
            if(al > 0) {
                ((Rgb565GfxHelper *)this)->blendPixel(cx - ix, y1 - 1, al, fg);
                ((Rgb565GfxHelper *)this)->blendPixel(cx + ix, y1 - 1, al, fg);
                ((Rgb565GfxHelper *)this)->blendPixel(cx - ix, y2 + 1, al, fg);
                ((Rgb565GfxHelper *)this)->blendPixel(cx + ix, y2 + 1, al, fg);
            }
        }
        if(h & 0x01) {
            ((Rgb565GfxHelper *)this)->blendVLine(alpha, fg, y, cy - iy, cx);
            ((Rgb565GfxHelper *)this)->blendVLine(alpha, fg, cy + iy, y + h - 1, cx);
        }
        else {
            uint8_t al = (alpha + 1) / 2;
            ((Rgb565GfxHelper *)this)->blendVLine(alpha, fg, y + 1, cy - iy, cx);
            ((Rgb565GfxHelper *)this)->blendVLine(alpha, fg, cy + iy, y + h - 1, cx);
            ((Rgb565GfxHelper *)this)->blendPixel(cx, y, al, fg);
            ((Rgb565GfxHelper *)this)->blendPixel(cx, y + h, al, fg);
        }
    }
}

void Rgb565Gfx::drawLatin1(uint8_t *str, uint32_t len, Font *font, uint32_t color, int32_t x, int32_t y) {
    uint8_t stdWidth = font->getStdWidth();
    uint8_t stdHeight = font->getStdHeight();
    uint8_t space = font->getSpaceWidth();
    space = GFX_MAX(space / 4, 1);

    while(len--) {
        if(x > clipX2) return;
        const CharInfo *c = font->getChar(*str++);
        if(c != NULL) {
            ((Rgb565GfxHelper *)this)->drawChar(c, color, x, y);
            x += c->getWidth() + space;
        }
        else {
            fillRect(color, x, y, stdWidth - 1, stdHeight - 1);
            x += stdWidth + space;
        }
    }
}

void Rgb565Gfx::drawUTF16(uint8_t *str, uint32_t len, Font *font, uint32_t color, int32_t x, int32_t y) {
    uint8_t stdWidth = font->getStdWidth();
    uint8_t stdHeight = font->getStdHeight();
    uint8_t space = font->getSpaceWidth();
    space = GFX_MAX(space / 4, 1);

    while(len--) {
        if(x > clipX2) return;
        uint16_t unicode = str[0] | (str[1] << 8);
        const CharInfo *c = font->getChar(unicode);
        if(c != NULL) {
            ((Rgb565GfxHelper *)this)->drawChar(c, color, x, y);
            x += c->getWidth() + space;
        }
        else {
            fillRect(color, x, y, stdWidth - 1, stdHeight - 1);
            x += stdWidth + space;
        }
        str += 2;
    }
}
