
#include <string.h>
#include "flint_rgb565_gfx.h"
#include "flint_fixed_point.h"
#include "flint_rgb565_gfx_helper.h"

typedef struct {
    int32_t x;
    int32_t y;
} Point;

static inline bool IsVisible(Rgb565Gfx *g, int32_t x, int32_t y, int32_t w, int32_t h) {
    if(x > g->clipX2 || (x + w) < g->clipX1) return false;
    if(y > g->clipY2 || (y + h) < g->clipY1) return false;
    return true;
}

Rgb565Gfx::Rgb565Gfx(int32_t w, int32_t h, int32_t clipX1, int32_t clipY1, int32_t clipX2, int32_t clipY2, uint8_t *data) :
width(w), height(h), clipX1(clipX1), clipY1(clipY1), clipX2(clipX2), clipY2(clipY2), data(data) {

}

void Rgb565Gfx::clear(uint32_t color) {
    ((Rgb565GfxHelper *)this)->clear(color);
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

void Rgb565Gfx::drawLine(uint32_t color, int32_t thk, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    if(x1 == x2 && y1 == y2)
        return;
    uint8_t alpha = color >> 27;
    if(thk < 1) thk = 1;
    if(y1 == y2) {
        if(x1 > x2) GFX_SWAP(x1, x2);
        int32_t half = (thk - 1) >> 1;
        y1 -= half;
        y2 += half;

        if((thk & 0x01) == 0) {
            uint32_t a1 = color >> 28;
            uint32_t a2 = color >> 29;

            ((Rgb565GfxHelper *)this)->blendVLine(a1, color, y1, y2, x1);
            ((Rgb565GfxHelper *)this)->blendVLine(a1, color, y1, y2, x2);

            ((Rgb565GfxHelper *)this)->blendHLine(a1, color, x1 + 1, x2 - 1, y1 - 1);
            ((Rgb565GfxHelper *)this)->blendHLine(a1, color, x1 + 1, x2 - 1, y2 + 1);

            ((Rgb565GfxHelper *)this)->blendPixel(a2, color, x1, y1 - 1);
            ((Rgb565GfxHelper *)this)->blendPixel(a2, color, x2, y1 - 1);
            ((Rgb565GfxHelper *)this)->blendPixel(a2, color, x1, y2 + 1);
            ((Rgb565GfxHelper *)this)->blendPixel(a2, color, x2, y2 + 1);

            x1++;
            x2--;
        }
        ((Rgb565GfxHelper *)this)->blendRect(alpha, color, x1, y1, x2, y2);
    }
    else if(x1 == x2) {
        if(y1 > y2) GFX_SWAP(y1, y2);
        int32_t half = (thk - 1) >> 1;
        x1 -= half;
        x2 += half;

        if((thk & 0x01) == 0) {
            uint32_t a1 = color >> 28;
            uint32_t a2 = color >> 29;

            ((Rgb565GfxHelper *)this)->blendHLine(a1, color, x1, x2, y1);
            ((Rgb565GfxHelper *)this)->blendHLine(a1, color, x1, x2, y2);

            ((Rgb565GfxHelper *)this)->blendVLine(a1, color, y1 + 1, y2 - 1, x1 - 1);
            ((Rgb565GfxHelper *)this)->blendVLine(a1, color, y1 + 1, y2 - 1, x2 + 1);

            ((Rgb565GfxHelper *)this)->blendPixel(a2, color, x1 - 1, y1);
            ((Rgb565GfxHelper *)this)->blendPixel(a2, color, x2 + 1, y1);
            ((Rgb565GfxHelper *)this)->blendPixel(a2, color, x1 - 1, y2);
            ((Rgb565GfxHelper *)this)->blendPixel(a2, color, x2 + 1, y2);

            y1++;
            y2--;
        }
        ((Rgb565GfxHelper *)this)->blendRect(alpha, color, x1, y1, x2, y2);
    }
    else {
        if(thk == 1) {
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
                    uint8_t a = err * alpha / dx;
                    ((Rgb565GfxHelper *)this)->blendPixel(alpha - a, color, x, y);
                    ((Rgb565GfxHelper *)this)->blendPixel(a, color, x, y - ystep);
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
                    uint8_t a = err * alpha / dy;
                    ((Rgb565GfxHelper *)this)->blendPixel(alpha - a, color, x, y);
                    ((Rgb565GfxHelper *)this)->blendPixel(a, color, x - xstep, y);
                    err -= dx;
                    if(err < 0) {
                        x += xstep;
                        err += dy;
                    }
                }
            }
        }
        else {
            static constexpr uint16_t fpmax = (1 << FP_PRECISION);
            static constexpr uint16_t fpmask = fpmax - 1;
            Point p[4];
            CalcRectPoints(thk, x1, y1, x2, y2, p);

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
                int32_t fx1 = fpmax - (((x1 << FP_PRECISION) / dy1) & fpmask);
                x1 = p[0].x + x1 / dy1;

                int32_t x2 = (y - p[1].y) * dx2 + dy2 - 1;
                int32_t fx2 = ((x2 << FP_PRECISION) / dy2) & fpmask;
                x2 = p[1].x + x2 / dy2;

                int16_t a = (dx1 < dy1) ? (fx1 * alpha / fpmax) : (alpha - astep1 + fx1 * dy1 / dx1 * alpha / fpmax);
                for(int32_t i = x1++; a > 0 && i >= p[0].x; i--) {
                    ((Rgb565GfxHelper *)this)->blendPixel(a, color, i, y);
                    a -= astep1;
                }

                a = (dx2 < dy2) ? (fx2 * alpha / fpmax) : (alpha - astep2 + fx2 * dy2 / dx2 * alpha / fpmax);
                for(int32_t i = x2--; a > 0 && i <= p[2].x; i++) {
                    ((Rgb565GfxHelper *)this)->blendPixel(a, color, i, y);
                    a -= astep2;
                }

                ((Rgb565GfxHelper *)this)->blendHLine(alpha, color, x1, x2, y);
            }
            ymax = GFX_MIN(this->clipY2, GFX_MAX(p[0].y, p[2].y));
            if(p[0].y < p[2].y) for(; y < ymax; y++) {
                int32_t x1 = (y - p[0].y) * dx2;
                int32_t fx1 = fpmax - (((x1 << FP_PRECISION) / dy2) & fpmask);
                x1 = p[0].x + x1 / dy2;

                int32_t x2 = (y - p[1].y) * dx2 + dy2 - 1;
                int32_t fx2 = ((x2 << FP_PRECISION) / dy2) & fpmask;
                x2 = p[1].x + x2 / dy2;

                int16_t a = (dx2 < dy2) ? (fx1 * alpha / fpmax) : (alpha - astep2 + fx1 * dy2 / dx2 * alpha / fpmax);
                for(int32_t i = x1++; a > 0 && i >= p[0].x; i--) {
                    ((Rgb565GfxHelper *)this)->blendPixel(a, color, i, y);
                    a -= astep2;
                }

                a = (dx2 < dy2) ? (fx2 * alpha / fpmax) : (alpha - astep2 + fx2 * dy2 / dx2 * alpha / fpmax);
                for(int32_t i = x2--; a > 0 && i <= p[2].x; i++) {
                    ((Rgb565GfxHelper *)this)->blendPixel(a, color, i, y);
                    a -= astep2;
                }

                ((Rgb565GfxHelper *)this)->blendHLine(alpha, color, x1, x2, y);
            }
            else for(; y < ymax; y++) {
                int32_t x1 = (p[0].y - y) * dx1;
                int32_t fx1 = fpmax - (((x1 << FP_PRECISION) / dy1) & fpmask);
                x1 = p[0].x + x1 / dy1;

                int32_t x2 = (p[3].y - y) * dx1 + dy1 - 1;
                int32_t fx2 = ((x2 << FP_PRECISION) / dy1) & fpmask;
                x2 = p[3].x + x2 / dy1;

                int16_t a = (dx1 < dy1) ? (fx1 * alpha / fpmax) : (alpha - astep1 + fx1 * dy1 / dx1 * alpha / fpmax);
                for(int32_t i = x1++; a > 0 && i >= p[0].x; i--) {
                    ((Rgb565GfxHelper *)this)->blendPixel(a, color, i, y);
                    a -= astep1;
                }

                a = (dx1 < dy1) ? (fx2 * alpha / fpmax) : (alpha - astep1 + fx2 * dy1 / dx1 * alpha / fpmax);
                for(int32_t i = x2--; a > 0 && i <= p[2].x; i++) {
                    ((Rgb565GfxHelper *)this)->blendPixel(a, color, i, y);
                    a -= astep1;
                }

                ((Rgb565GfxHelper *)this)->blendHLine(alpha, color, x1, x2, y);
            }
            ymax = GFX_MIN(this->clipY2, p[3].y);
            if(y >= p[0].y && y >= p[2].y) for(; y <= ymax; y++) {
                int32_t x1 = (y - p[0].y) * dx2;
                int32_t fx1 = fpmax - (((x1 << FP_PRECISION) / dy2) & fpmask);
                x1 = p[0].x + x1 / dy2;

                int32_t x2 = (p[3].y - y) * dx1 + dy1 - 1;
                int32_t fx2 = ((x2 << FP_PRECISION) / dy1) & fpmask;
                x2 = p[3].x + x2 / dy1;

                int16_t a = (dx2 < dy2) ? (fx1 * alpha / fpmax) : (alpha - astep2 + fx1 * dy2 / dx2 * alpha / fpmax);
                for(int32_t i = x1++; a > 0 && i >= p[0].x; i--) {
                    ((Rgb565GfxHelper *)this)->blendPixel(a, color, i, y);
                    a -= astep2;
                }

                a = (dx1 < dy1) ? (fx2 * alpha / fpmax) : (alpha - astep1 + fx2 * dy1 / dx1 * alpha / fpmax);
                for(int32_t i = x2--; a > 0 && i <= p[2].x; i++) {
                    ((Rgb565GfxHelper *)this)->blendPixel(a, color, i, y);
                    a -= astep1;
                }

                ((Rgb565GfxHelper *)this)->blendHLine(alpha, color, x1, x2, y);
            }
        }
    }
}

void Rgb565Gfx::drawRect(uint32_t color, int32_t thk, int32_t x, int32_t y, int32_t w, int32_t h) {
    if(thk < 1) thk = 1;
    if(!IsVisible(this, x - thk / 2, y - thk / 2, w + thk - (thk & 1), h + thk - (thk & 1))) return;
    uint8_t alpha = color >> 27;
    int32_t half = (thk - 1) >> 1;

    int32_t xo1 = x - half;
    int32_t yo1 = y - half;
    int32_t xo2 = x + w + half;
    int32_t yo2 = y + h + half;

    int32_t xi1 = x + half;
    int32_t yi1 = y + half;
    int32_t xi2 = x + w - half;
    int32_t yi2 = y + h - half;

    ((Rgb565GfxHelper *)this)->blendRect(alpha, color, xo1, yo1, xo2, yi1);
    ((Rgb565GfxHelper *)this)->blendRect(alpha, color, xo1, yi2, xo2, yo2);

    yi1++;
    yi2--;

    ((Rgb565GfxHelper *)this)->blendRect(alpha, color, xo1, yi1, xi1, yi2);
    ((Rgb565GfxHelper *)this)->blendRect(alpha, color, xi2, yi1, xo2, yi2);

    if((thk & 0x01) == 0) {
        uint8_t a = color >> 28;

        xo1--;
        xo2++;
        xi1++;
        xi2--;

        ((Rgb565GfxHelper *)this)->blendHLine(a, color, xo1, xo2, yo1 - 1);
        ((Rgb565GfxHelper *)this)->blendHLine(a, color, xo1, xo2, yo2 + 1);

        ((Rgb565GfxHelper *)this)->blendHLine(a, color, xi1, xi2, yi1);
        ((Rgb565GfxHelper *)this)->blendHLine(a, color, xi1, xi2, yi2);

        ((Rgb565GfxHelper *)this)->blendVLine(a, color, yo1, yo2, xo1);
        ((Rgb565GfxHelper *)this)->blendVLine(a, color, yo1, yo2, xo2);

        ((Rgb565GfxHelper *)this)->blendVLine(a, color, yi1, yi2, xi1);
        ((Rgb565GfxHelper *)this)->blendVLine(a, color, yi1, yi2, xi2);
    }
}

void Rgb565Gfx::fillRect(uint32_t color, int32_t x, int32_t y, int32_t w, int32_t h) {
    ((Rgb565GfxHelper *)this)->blendRect(color >> 27, color, x, y, x + w, y + h);
}

static void RadiusAdjustment(int32_t w, int32_t h, int32_t &r1, int32_t &r2, int32_t &r3, int32_t &r4) {
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
}

static void DrawEllipseHLine2(Rgb565Gfx *g, uint32_t color, FP cx, FP cy, FP xo, FP xi) {
    FP pxo = cx - xo;
    FP pxi = cx - xi;
    int32_t py = cy;

    uint8_t alpha = color >> 27;
    uint8_t ao = alpha - pxo.fraction(alpha);
    uint8_t ai = pxi.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, (int32_t)pxo + 1, (int32_t)pxi - 1, py);
    if(ao > 0) ((Rgb565GfxHelper *)g)->blendPixel(ao, color, pxo, py);
    if(ai > 0) ((Rgb565GfxHelper *)g)->blendPixel(ai, color, pxi, py);

    pxo = cx + xo;
    pxi = cx + xi;
    ao = pxo.fraction(alpha);
    ai = alpha - pxi.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, (int32_t)pxi + 1, (int32_t)pxo - 1, py);
    if(ao > 0) ((Rgb565GfxHelper *)g)->blendPixel(ao, color, pxo, py);
    if(ai > 0) ((Rgb565GfxHelper *)g)->blendPixel(ai, color, pxi, py);
}

static void DrawEllipseHLine41(Rgb565Gfx *g, uint32_t color, FP cx, FP cy, FP xo, FP xi, FP y) {
    FP pxo = cx - xo;
    FP pxi = cx - xi;
    int32_t py1 = cy - y;
    int32_t py2 = cy + y;

    uint8_t alpha = color >> 27;
    uint8_t ao = alpha - pxo.fraction(alpha);
    uint8_t ai = pxi.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, (int32_t)pxo + 1, (int32_t)pxi - 1, py1);
    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, (int32_t)pxo + 1, (int32_t)pxi - 1, py2);
    if(ao > 0) {
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, pxo, py1);
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, pxo, py2);
    }
    if(ai > 0) {
        ((Rgb565GfxHelper *)g)->blendPixel(ai, color, pxi, py1);
        ((Rgb565GfxHelper *)g)->blendPixel(ai, color, pxi, py2);
    }

    pxo = cx + xo;
    pxi = cx + xi;
    ao = pxo.fraction(alpha);
    ai = alpha - pxi.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, (int32_t)pxi + 1, (int32_t)pxo - 1, py1);
    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, (int32_t)pxi + 1, (int32_t)pxo - 1, py2);
    if(ao > 0) {
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, pxo, py1);
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, pxo, py2);
    }
    if(ai > 0) {
        ((Rgb565GfxHelper *)g)->blendPixel(ai, color, pxi, py1);
        ((Rgb565GfxHelper *)g)->blendPixel(ai, color, pxi, py2);
    }
}

static void DrawEllipseHLine42(Rgb565Gfx *g, uint32_t color, FP cx, FP cy, FP xo, FP xi, FP y) {
    FP pxo = cx - xo;
    int32_t pxi = cx - xi;
    int32_t py1 = cy - y;
    int32_t py2 = cy + y;

    uint8_t alpha = color >> 27;
    uint8_t ao = alpha - pxo.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, (int32_t)pxo + 1, pxi, py1);
    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, (int32_t)pxo + 1, pxi, py2);
    if(ao > 0) {
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, pxo, py1);
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, pxo, py2);
    }

    pxo = cx + xo;
    pxi = cx + xi;
    ao = pxo.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, pxi, (int32_t)pxo - 1, py1);
    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, pxi, (int32_t)pxo - 1, py2);
    if(ao > 0) {
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, pxo, py1);
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, pxo, py2);
    }
}

static void DrawEllipseVLine2(Rgb565Gfx *g, uint32_t color, int32_t cx, FP cy, FP yo, FP yi) {
    FP pyo = cy - yo;
    FP pyi = cy - yi;

    uint8_t alpha = color >> 27;
    uint8_t ao = alpha - pyo.fraction(alpha);
    uint8_t ai = pyi.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, (int32_t)pyo + 1, (int32_t)pyi - 1, cx);
    if(ao > 0) ((Rgb565GfxHelper *)g)->blendPixel(ao, color, cx, pyo);
    if(ai > 0) ((Rgb565GfxHelper *)g)->blendPixel(ai, color, cx, pyi);

    pyo = cy + yo;
    pyi = cy + yi;
    ao = pyo.fraction(alpha);
    ai = alpha - pyi.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, (int32_t)pyi + 1, (int32_t)pyo - 1, cx);
    if(ao > 0) ((Rgb565GfxHelper *)g)->blendPixel(ao, color, cx, pyo);
    if(ai > 0) ((Rgb565GfxHelper *)g)->blendPixel(ai, color, cx, pyi);
}

static void DrawEllipseVLine41(Rgb565Gfx *g, uint32_t color, FP cx, FP cy, FP yo, FP yi, FP x) {
    FP pyo = cy - yo;
    FP pyi = cy - yi;
    int32_t px1 = cx - x;
    int32_t px2 = cx + x;

    uint8_t alpha = color >> 27;
    uint8_t ao = alpha - pyo.fraction(alpha);
    uint8_t ai = pyi.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, (int32_t)pyo + 1, (int32_t)pyi - 1, px1);
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, (int32_t)pyo + 1, (int32_t)pyi - 1, px2);
    if(ao > 0) {
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px1, pyo);
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px2, pyo);
    }
    if(ai > 0) {
        ((Rgb565GfxHelper *)g)->blendPixel(ai, color, px1, pyi);
        ((Rgb565GfxHelper *)g)->blendPixel(ai, color, px2, pyi);
    }

    pyo = cy + yo;
    pyi = cy + yi;
    ao = pyo.fraction(alpha);
    ai = alpha - pyi.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, (int32_t)pyi + 1, (int32_t)pyo - 1, px1);
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, (int32_t)pyi + 1, (int32_t)pyo - 1, px2);
    if(ao > 0) {
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px1, pyo);
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px2, pyo);
    }
    if(ai > 0) {
        ((Rgb565GfxHelper *)g)->blendPixel(ai, color, px1, pyi);
        ((Rgb565GfxHelper *)g)->blendPixel(ai, color, px2, pyi);
    }
}

static void DrawEllipseVLine42(Rgb565Gfx *g, uint32_t color, FP cx, FP cy, FP yo, FP yi, FP x) {
    FP pyo = cy - yo;
    int32_t pyi = cy - yi;
    int32_t px1 = cx - x;
    int32_t px2 = cx + x;

    uint8_t alpha = color >> 27;
    uint8_t ao = alpha - pyo.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, (int32_t)pyo + 1, pyi, px1);
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, (int32_t)pyo + 1, pyi, px2);
    if(ao > 0 && (int32_t)pyo <= (int32_t)pyi) {
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px1, pyo);
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px2, pyo);
    }

    pyo = cy + yo;
    pyi = cy + yi;
    ao = pyo.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, pyi, (int32_t)pyo - 1, px1);
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, pyi, (int32_t)pyo - 1, px2);
    if(ao > 0 && (int32_t)pyo >= (int32_t)pyi) {
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px1, pyo);
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px2, pyo);
    }
}

static void PlotH2(Rgb565Gfx *g, uint32_t color, FP cx, int32_t cy, FP x) {
    uint8_t alpha = color >> 27;
    FP px1 = cx - x;
    FP px2 = cx + x;

    uint8_t ai1 = px1.fraction(alpha);
    uint8_t ao1 = alpha - ai1;

    uint8_t ao2 = px2.fraction(alpha);
    uint8_t ai2 = alpha - ao2;

    ((Rgb565GfxHelper *)g)->blendPixel(ao1, color, px1, cy);
    if(ao2 > 0) ((Rgb565GfxHelper *)g)->blendPixel(ao2, color, px2, cy);
    ++px1;
    --px2;
    if(ai1 > 0) ((Rgb565GfxHelper *)g)->blendPixel(ai1, color, px1, cy);
    ((Rgb565GfxHelper *)g)->blendPixel(ai2, color, px2, cy);
}

static void PlotH4(Rgb565Gfx *g, uint32_t color, FP cx, FP cy, FP x, FP y) {
    uint8_t alpha = color >> 27;
    FP px1 = cx - x;
    FP px2 = cx + x;
    int32_t py1 = cy - y;
    int32_t py2 = cy + y;

    uint8_t ai1 = px1.fraction(alpha);
    uint8_t ao1 = alpha - ai1;

    uint8_t ao2 = px2.fraction(alpha);
    uint8_t ai2 = alpha - ao2;

    ((Rgb565GfxHelper *)g)->blendPixel(ao1, color, px1, py1);
    ((Rgb565GfxHelper *)g)->blendPixel(ao1, color, px1, py2);
    if(ao2 > 0) {
        ((Rgb565GfxHelper *)g)->blendPixel(ao2, color, px2, py1);
        ((Rgb565GfxHelper *)g)->blendPixel(ao2, color, px2, py2);
    }
    ++px1;
    --px2;
    if(ai1 > 0) {
        ((Rgb565GfxHelper *)g)->blendPixel(ai1, color, px1, py1);
        ((Rgb565GfxHelper *)g)->blendPixel(ai1, color, px1, py2);
    }
    ((Rgb565GfxHelper *)g)->blendPixel(ai2, color, px2, py1);
    ((Rgb565GfxHelper *)g)->blendPixel(ai2, color, px2, py2);
}

static void PlotV2(Rgb565Gfx *g, uint32_t color, int32_t cx, FP cy, FP y) {
    uint8_t alpha = color >> 27;
    FP py1 = cy - y;
    FP py2 = cy + y;

    uint8_t ai1 = py1.fraction(alpha);
    uint8_t ao1 = alpha - ai1;

    uint8_t ao2 = py2.fraction(alpha);
    uint8_t ai2 = alpha - ao2;

    ((Rgb565GfxHelper *)g)->blendPixel(ao1, color, cx, py1);
    if(ao2 > 0) ((Rgb565GfxHelper *)g)->blendPixel(ao2, color, cx, py2);
    ++py1;
    --py2;
    if(ai1 > 0) ((Rgb565GfxHelper *)g)->blendPixel(ai1, color, cx, py1);
    ((Rgb565GfxHelper *)g)->blendPixel(ai2, color, cx, py2);
}

static void PlotV4(Rgb565Gfx *g, uint32_t color, FP cx, FP cy, FP x, FP y) {
    uint8_t alpha = color >> 27;
    int32_t px1 = cx - x;
    int32_t px2 = cx + x;
    FP py1 = cy - y;
    FP py2 = cy + y;

    uint8_t ai1 = py1.fraction(alpha);
    uint8_t ao1 = alpha - ai1;

    uint8_t ao2 = py2.fraction(alpha);
    uint8_t ai2 = alpha - ao2;

    ((Rgb565GfxHelper *)g)->blendPixel(ao1, color, px1, py1);
    ((Rgb565GfxHelper *)g)->blendPixel(ao1, color, px2, py1);
    if(ao2 > 0) {
        ((Rgb565GfxHelper *)g)->blendPixel(ao2, color, px1, py2);
        ((Rgb565GfxHelper *)g)->blendPixel(ao2, color, px2, py2);
    }
    ++py1;
    --py2;
    if(ai1 > 0) {
        ((Rgb565GfxHelper *)g)->blendPixel(ai1, color, px1, py1);
        ((Rgb565GfxHelper *)g)->blendPixel(ai1, color, px2, py1);
    }
    ((Rgb565GfxHelper *)g)->blendPixel(ai2, color, px1, py2);
    ((Rgb565GfxHelper *)g)->blendPixel(ai2, color, px2, py2);
}

void Rgb565Gfx::drawCircle(uint32_t color, int32_t thk, int32_t x, int32_t y, uint32_t d) {
    if(!IsVisible(this, x - thk / 2, y - thk / 2, d + thk - (thk & 1), d + thk - (thk & 1))) return;
    d++;
    FP cx = x + (FP)d / (int32_t)2;
    FP cy = y + (FP)d / (int32_t)2;
    if(thk > 1) {
        FP xo, xi, yo, yi;
        FP rro = square((FP)(d + thk) / (int32_t)2);
        FP rri = square((FP)(d - thk) / (int32_t)2);

        FP ymax = FP::absMax(cy - clipY1, clipY2 - cy);
        if(cy - clipY2 > FP::ONE) yo = (d & 1) ? floor(cy - clipY2) : (floor(cy - clipY2) - FP::HALF);
        else if(clipY1 - cy > FP::ONE) yo = (d & 1) ? floor(clipY1 - cy) : (floor(clipY1 - cy) - FP::HALF);
        else yo = (d & 1) ? FP::ONE : FP::HALF;
        if(d & 1) {
            DrawEllipseHLine2(this, color, cx, cy, (FP)(d + thk) / (int32_t)2, (FP)(d - thk) / (int32_t)2);
            DrawEllipseVLine2(this, color, cx, cy, (FP)(d + thk) / (int32_t)2, (FP)(d - thk) / (int32_t)2);
        }
        while(true) {
            if(yo > ymax) return;
            FP yy = square(yo);
            xi = sqrt(rri - yy);
            if(xi >= yo) {
                xo = sqrt(rro - yy);
                DrawEllipseHLine41(this, color, cx, cy, xo, xi, yo);
            }
            else break;
            ++yo;
        }

        FP yo0 = yo;
        FP xil0 = cx - floor(cx - xi) + FP::ONE;
        FP xir0 = floor(cx + xi) - cx + FP::ONE;
        while(true) {
            if(yo > ymax) return;
            xo = sqrt(rro - square(yo));
            if(xo >= yo) DrawEllipseHLine42(this, color, cx, cy, xo, xil0, yo);
            else break;
            ++yo;
        }
        if(xil0 != xir0) {
            ((Rgb565GfxHelper *)this)->blendVLine(color >> 27, color, cy - yo + FP::ONE, cy - yo0, cx + xir0);
            ((Rgb565GfxHelper *)this)->blendVLine(color >> 27, color, cy + yo0, cy + yo - FP::ONE, cx + xir0);
        }

        yo0 = yo;
        FP xo0 = (d & 1) ? floor(xo) : (floor(xo) + FP::HALF);
        FP xmax = GFX_MIN(GFX_MAX(xil0, xir0) - FP::ONE, GFX_MAX(cx - clipX1, clipX2 - cx));
        if(cx - clipX2 > FP::ONE) xo = (d & 1) ? floor(cx - clipX2) : (floor(cx - clipX2) - FP::HALF);
        else if(clipX1 - cx > FP::ONE) xo = (d & 1) ? floor(clipX1 - cx) : (floor(clipX1 - cx) - FP::HALF);
        else xo = (d & 1) ? FP::ONE : FP::HALF;
        for(; xo < xmax; ++xo) {
            FP xx = square(xo);
            yo = sqrt(rro - xx);
            yi = sqrt(rri - xx);
            DrawEllipseVLine41(this, color, cx, cy, yo, yi, xo);
        }

        xmax = GFX_MIN(xo0, GFX_MAX(cx - clipX1, clipX2 - cx));
        for(; xo <= xmax; ++xo) {
            yo = sqrt(rro - square(xo));
            DrawEllipseVLine42(this, color, cx, cy, yo, yo0, xo);
        }
    }
    else {
        FP rr = square((FP)d / (int32_t)2);
        FP yo, xo;
        FP ymax = FP::absMax(cy - clipY1, clipY2 - cy);
        if(cy - clipY2 > FP::ONE) yo = (d & 1) ? floor(cy - clipY2) : (floor(cy - clipY2) - FP::HALF);
        else if(clipY1 - cy > FP::ONE) yo = (d & 1) ? floor(clipY1 - cy) : (floor(clipY1 - cy) - FP::HALF);
        else yo = (d & 1) ? FP::ONE : FP::HALF;
        if(d & 1) {
            PlotH2(this, color, cx, cy, (FP)d / (int32_t)2);
            PlotV2(this, color, cx, cy, (FP)d / (int32_t)2);
        }
        while(true) {
            if(yo > ymax) return;
            xo = sqrt(rr - square(yo));
            if(xo >= yo) PlotH4(this, color, cx, cy, xo, yo);
            else break;
            ++yo;
        }

        FP xmax = GFX_MIN(xo, GFX_MAX(cx - clipX1, clipX2 - cx));
        if(cx - clipX2 > FP::ONE) xo = (d & 1) ? floor(cx - clipX2) : (floor(cx - clipX2) - FP::HALF);
        else if(clipX1 - cx > FP::ONE) xo = (d & 1) ? floor(clipX1 - cx) : (floor(clipX1 - cx) - FP::HALF);
        else xo = (d & 1) ? FP::ONE : FP::HALF;
        for(; xo < xmax; ++xo) {
            yo = sqrt(rr - square(xo));
            PlotV4(this, color, cx, cy, xo, yo);
        }
    }
}

void Rgb565Gfx::drawEllipse(uint32_t color, int32_t thk, int32_t x, int32_t y, uint32_t w, uint32_t h) {
    if(thk < 1) thk = 1;
    if(w == h) return drawCircle(color, thk, x, y, w);
    if(!IsVisible(this, x - thk / 2, y - thk / 2, w + thk - (thk & 1), h + thk - (thk & 1))) return;
    w++;
    h++;
    FP cx = x + (FP)w / (int32_t)2;
    FP cy = y + (FP)h / (int32_t)2;
    if(thk > 1) {
        FP xo, xi, yo, yi;

        FP aao = square((FP)(w + thk) / (int32_t)2);
        FP bbo = square((FP)(h + thk) / (int32_t)2);
        FP abo = aao * bbo;

        FP aai = square((FP)(w - thk) / (int32_t)2);
        FP bbi = square((FP)(h - thk) / (int32_t)2);
        FP abi = aai * bbi;

        FP ymax = FP::absMax(cy - clipY1, clipY2 - cy);
        if(cy - clipY2 > FP::ONE) yo = (h & 1) ? floor(cy - clipY2) : (floor(cy - clipY2) - FP::HALF);
        else if(clipY1 - cy > FP::ONE) yo = (h & 1) ? floor(clipY1 - cy) : (floor(clipY1 - cy) - FP::HALF);
        else yo = (h & 1) ? FP::ONE : FP::HALF;
        if(h & 1) DrawEllipseHLine2(this, color, cx, cy, (FP)(w + thk) / (int32_t)2, (FP)(w - thk) / (int32_t)2);
        if(w & 1) DrawEllipseVLine2(this, color, cx, cy, (FP)(h + thk) / (int32_t)2, (FP)(h - thk) / (int32_t)2);
        while(true) {
            if(yo > ymax) return;
            FP yy = square(yo);
            xi = sqrt((abi - aai * yy) / bbi);
            if(bbi * xi >= aai * yo) {
                xo = sqrt((abo - aao * yy) / bbo);
                DrawEllipseHLine41(this, color, cx, cy, xo, xi, yo);
            }
            else break;
            ++yo;
        }

        FP yo0 = yo;
        FP xil0 = cx - floor(cx - xi) + FP::ONE;
        FP xir0 = floor(cx + xi) - cx + FP::ONE;
        while(true) {
            if(yo > ymax) return;
            xo = sqrt((abo - aao * square(yo)) / bbo);
            if(bbo * xo >= aao * yo) DrawEllipseHLine42(this, color, cx, cy, xo, xil0, yo);
            else break;
            ++yo;
        }
        if(xil0 != xir0) {
            ((Rgb565GfxHelper *)this)->blendVLine(color >> 27, color, cy - yo + FP::ONE, cy - yo0, cx + xir0);
            ((Rgb565GfxHelper *)this)->blendVLine(color >> 27, color, cy + yo0, cy + yo - FP::ONE, cx + xir0);
        }

        yo0 = yo;
        FP xo0 = (w & 1) ? floor(xo) : (floor(xo) + FP::HALF);
        FP xmax = GFX_MIN(GFX_MAX(xil0, xir0) - FP::ONE, GFX_MAX(cx - clipX1, clipX2 - cx));
        if(cx - clipX2 > FP::ONE) xo = (w & 1) ? floor(cx - clipX2) : (floor(cx - clipX2) - FP::HALF);
        else if(clipX1 - cx > FP::ONE) xo = (w & 1) ? floor(clipX1 - cx) : (floor(clipX1 - cx) - FP::HALF);
        else xo = (w & 1) ? FP::ONE : FP::HALF;
        for(; xo < xmax; ++xo) {
            FP xx = square(xo);
            yo = sqrt((abo - bbo * xx) / aao);
            yi = sqrt((abi - bbi * xx) / aai);
            DrawEllipseVLine41(this, color, cx, cy, yo, yi, xo);
        }

        xmax = GFX_MIN(xo0, GFX_MAX(cx - clipX1, clipX2 - cx));
        for(; xo <= xmax; ++xo) {
            yo = sqrt((abo - bbo * square(xo)) / aao);
            DrawEllipseVLine42(this, color, cx, cy, yo, yo0, xo);
        }
    }
    else {
        FP aa = square((FP)w / (int32_t)2);
        FP bb = square((FP)h / (int32_t)2);
        FP ab = aa * bb;

        FP yo, xo;
        FP ymax = FP::absMax(cy - clipY1, clipY2 - cy);
        if(cy - clipY2 > FP::ONE) yo = (h & 1) ? floor(cy - clipY2) : (floor(cy - clipY2) - FP::HALF);
        else if(clipY1 - cy > FP::ONE) yo = (h & 1) ? floor(clipY1 - cy) : (floor(clipY1 - cy) - FP::HALF);
        else yo = (h & 1) ? FP::ONE : FP::HALF;
        if(h & 1) PlotH2(this, color, cx, cy, (FP)w / (int32_t)2);
        if(w & 1) PlotV2(this, color, cx, cy, (FP)h / (int32_t)2);
        while(true) {
            if(yo > ymax) return;
            xo = sqrt((ab - aa * square(yo)) / bb);
            if(bb * xo >= aa * yo) PlotH4(this, color, cx, cy, xo, yo);
            else break;
            ++yo;
        }

        FP xmax = GFX_MIN(xo, GFX_MAX(cx - clipX1, clipX2 - cx));
        if(cx - clipX2 > FP::ONE) xo = (w & 1) ? floor(cx - clipX2) : (floor(cx - clipX2) - FP::HALF);
        else if(clipX1 - cx > FP::ONE) xo = (w & 1) ? floor(clipX1 - cx) : (floor(clipX1 - cx) - FP::HALF);
        else xo = (w & 1) ? FP::ONE : FP::HALF;
        for(; xo < xmax; ++xo) {
            yo = sqrt((ab - bb * square(xo)) / aa);
            PlotV4(this, color, cx, cy, xo, yo);
        }
    }
}

static void DrawHLineAA(Rgb565Gfx *g, uint32_t color, FP x1, FP x2, int32_t y) {
    uint8_t alpha = color >> 27;
    uint8_t a1 = alpha - x1.fraction(alpha);
    uint8_t a2 = x2.fraction(alpha);
    int32_t px1 = x1;
    int32_t px2 = x2;
    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, px1 + 1, px2 - 1, y);
    if(a1 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a1, color, px1, y);
    if(a2 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a2, color, px2, y);
}

static void DrawHLineAA(Rgb565Gfx *g, uint32_t color, FP x1, int32_t x2, int32_t y) {
    int32_t px1 = x1;
    uint8_t alpha = color >> 27;
    uint8_t a1 = alpha - x1.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, px1 + 1, x2, y);
    if(a1 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a1, color, px1, y);
}

static void DrawHLineAA(Rgb565Gfx *g, uint32_t color, int32_t x1, FP x2, int32_t y) {
    int32_t px2 = x2;
    uint8_t alpha = color >> 27;
    uint8_t a2 = x2.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, x1, px2 - 1, y);
    if(a2 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a2, color, px2, y);
}

static void DrawVLineAA(Rgb565Gfx *g, uint32_t color, FP y1, FP y2, int32_t x) {
    uint8_t alpha = color >> 27;
    uint8_t a1 = alpha - y1.fraction(alpha);
    uint8_t a2 = y2.fraction(alpha);
    int32_t py1 = y1;
    int32_t py2 = y2;
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, py1 + 1, py2 - 1, x);
    if(a1 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a1, color, x, py1);
    if(a2 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a2, color, x, py2);
}

static void DrawVLineAA(Rgb565Gfx *g, uint32_t color, FP y1, int32_t y2, int32_t x) {
    uint8_t alpha = color >> 27;
    uint8_t a1 = alpha - y1.fraction(alpha);
    int32_t py = y1;
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, py + 1, y2, x);
    if(a1 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a1, color, x, py);
}

static void DrawVLineAA(Rgb565Gfx *g, uint32_t color, int32_t y1, FP y2, int32_t x) {
    uint8_t alpha = color >> 27;
    uint8_t a2 = y2.fraction(alpha);
    int32_t py = y2;
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, y1, py - 1, x);
    if(a2 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a2, color, x, py);
}

static void FillEllipseHLine2(Rgb565Gfx *g, uint32_t color, FP cx, FP cy, FP x, FP y) {
    FP fx1 = cx - x;
    FP fx2 = cx + x;
    int32_t px1 = fx1;
    int32_t px2 = fx2;
    int32_t py1 = cy - y;
    int32_t py2 = cy + y;

    uint8_t alpha = color >> 27;
    uint8_t a1 = alpha - fx1.fraction(alpha);
    uint8_t a2 = fx2.fraction(alpha);

    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, px1 + 1, px2 - 1, py1);
    if(a1 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a1, color, px1, py1);
    if(a2 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a2, color, px2, py1);

    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, px1 + 1, px2 - 1, py2);
    if(a1 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a1, color, px1, py2);
    if(a2 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a2, color, px2, py2);
}

static void FillEllipseVLine2(Rgb565Gfx *g, uint32_t color, int32_t cx, FP cy, FP yo, FP yi) {
    uint8_t alpha = color >> 27;

    FP fyo = cy - yo;
    int32_t pyo = fyo;
    int32_t pyi = cy - yi;
    uint8_t ao = alpha - fyo.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, pyo + 1, pyi, cx);
    if(ao > 0) ((Rgb565GfxHelper *)g)->blendPixel(ao, color, cx, pyo);

    fyo = cy + yo;
    pyo = fyo;
    pyi = cy + yi;
    ao = fyo.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, pyi, pyo - 1, cx);
    if(ao > 0) ((Rgb565GfxHelper *)g)->blendPixel(ao, color, cx, pyo);
}

static void FillEllipseVLine4(Rgb565Gfx *g, uint32_t color, FP cx, FP cy, FP yo, FP yi, FP x) {
    uint8_t alpha = color >> 27;
    int32_t px1 = cx - x;
    int32_t px2 = cx + x;

    FP fyo = cy - yo;
    int32_t pyo = fyo;
    int32_t pyi = cy - yi;
    uint8_t ao = alpha - fyo.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, pyo + 1, pyi, px1);
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, pyo + 1, pyi, px2);
    if(ao > 0) {
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px1, pyo);
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px2, pyo);
    }

    fyo = cy + yo;
    pyo = fyo;
    pyi = cy + yi;
    ao = fyo.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, pyi, pyo - 1, px1);
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, pyi, pyo - 1, px2);
    if(ao > 0) {
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px1, pyo);
        ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px2, pyo);
    }
}

void Rgb565Gfx::fillCircle(uint32_t color, int32_t x, int32_t y, uint32_t d) {
    if(!IsVisible(this, x, y, d, d)) return;
    d++;
    FP r = (FP)d / (int32_t)2;
    FP cx = x + r;
    FP cy = y + r;
    FP rr = square(r);

    FP xo, yo;
    FP ymax = FP::absMax(cy - clipY1, clipY2 - cy);
    if((cy - clipY2) >= FP::ONE) yo = (d & 1) ? floor(cy - clipY2) : (floor(cy - clipY2) - FP::HALF);
    else if(clipY1 - cy >= FP::ONE) yo = (d & 1) ? floor(clipY1 - cy) : (floor(clipY1 - cy) - FP::HALF);
    else yo = (d & 1) ? FP::ONE : FP::HALF;
    if(d & 1) DrawHLineAA(this, color, cx - r, cx + r, cy);
    while(true) {
        if(yo > ymax) return;
        xo = sqrt(rr - square(yo));
        if(xo >= yo) FillEllipseHLine2(this, color, cx, cy, xo, yo);
        else break;
        ++yo;
    }

    FP yo0 = yo;
    FP xmax = GFX_MIN(xo, GFX_MAX(cx - clipX1, clipX2 - cx));
    if(cx - clipX2 > FP::ONE) xo = (d & 1) ? floor(cx - clipX2) : (floor(cx - clipX2) - FP::HALF);
    else if(clipX1 - cx > FP::ONE) xo = (d & 1) ? floor(clipX1 - cx) : (floor(clipX1 - cx) - FP::HALF);
    else xo = (d & 1) ? FP::ONE : FP::HALF;
    for(; xo < xmax; ++xo) {
        yo = sqrt(rr - square(xo));
        FillEllipseVLine4(this, color, cx, cy, yo, yo0, xo);
    }
    if(d & 1) FillEllipseVLine2(this, color, cx, cy, r, yo0);
}

void Rgb565Gfx::fillEllipse(uint32_t color, int32_t x, int32_t y, uint32_t w, uint32_t h) {
    if(w == h) return fillCircle(color, x, y, w);
    if(!IsVisible(this, x, y, w, h)) return;

    w++;
    h++;
    FP a = (FP)w / (int32_t)2;
    FP b = (FP)h / (int32_t)2;

    FP cx = x + a;
    FP cy = y + b;

    FP aa = square(a);
    FP bb = square(b);
    FP ab = aa * bb;

    FP yo, xo;
    FP ymax = FP::absMax(cy - clipY1, clipY2 - cy);
    if(cy - clipY2 > FP::ONE) yo = (h & 1) ? floor(cy - clipY2) : (floor(cy - clipY2) - FP::HALF);
    else if(clipY1 - cy > FP::ONE) yo = (h & 1) ? floor(clipY1 - cy) : (floor(clipY1 - cy) - FP::HALF);
    else yo = (h & 1) ? FP::ONE : FP::HALF;
    if(h & 1) DrawHLineAA(this, color, cx - a, cx + a, cy);
    while(true) {
        if(yo > ymax) return;
        xo = sqrt((ab - aa * square(yo)) / bb);
        if(bb * xo >= aa * yo) FillEllipseHLine2(this, color, cx, cy, xo, yo);
        else break;
        ++yo;
    }

    FP yo0 = yo;
    FP xmax = GFX_MIN(xo, GFX_MAX(cx - clipX1, clipX2 - cx));
    if(cx - clipX2 > FP::ONE) xo = (w & 1) ? floor(cx - clipX2) : (floor(cx - clipX2) - FP::HALF);
    else if(clipX1 - cx > FP::ONE) xo = (w & 1) ? floor(clipX1 - cx) : (floor(clipX1 - cx) - FP::HALF);
    else xo = (w & 1) ? FP::ONE : FP::HALF;
    for(; xo < xmax; ++xo) {
        yo = sqrt((ab - bb * square(xo)) / aa);
        FillEllipseVLine4(this, color, cx, cy, yo, yo0, xo);
    }
    if(w & 1) FillEllipseVLine2(this, color, cx, cy, b, yo0);
}

static void DrawQuarterCircle1(Rgb565Gfx *g, uint32_t color, int32_t thk, int32_t x, int32_t y, int32_t r) {
    if(!IsVisible(g, x, y, r, r)) return;

    if(thk > 1) {
        FP cx = x + r + FP::HALF;
        FP cy = y + r + FP::HALF;
        FP xo, xi, yi;
        FP rro = square(r + (FP)thk / (int32_t)2);
        FP rri = (thk >= (r << 1)) ? (FP)0 : square(r - (FP)thk / (int32_t)2);

        FP ymax = cy - g->clipY1;
        FP yo = (cy - g->clipY2 > FP::ONE) ? floor(cy - g->clipY2) : FP::ONE;
        while(true) {
            if(yo > ymax) return;
            FP yy = square(yo);
            xi = sqrt(rri - yy);
            if(xi >= yo) {
                xo = sqrt(rro - yy);
                DrawHLineAA(g, color, cx - xo, cx - xi, cy - yo);
            }
            else break;
            ++yo;
        }

        FP xi0 = cx - floor(cx - xi) + FP::ONE;
        while(true) {
            if(yo > ymax) return;
            xo = sqrt(rro - square(yo));
            if(xo >= yo) DrawHLineAA(g, color, cx - xo, (int32_t)(cx - xi0), cy - yo);
            else break;
            ++yo;
        }

        int32_t yo0 = cy - yo;
        FP xo0 = floor(xo);
        FP xmax = GFX_MIN(xi0 - FP::ONE, cx - g->clipX1);
        xo = (cx - g->clipX2 > FP::ONE) ? floor(cx - g->clipX2) : FP::ONE;
        for(; xo < xmax; ++xo) {
            FP xx = square(xo);
            yo = sqrt(rro - xx);
            yi = sqrt(rri - xx);
            DrawVLineAA(g, color, cy - yo, cy - yi, cx - xo);
        }

        xmax = GFX_MIN(xo0, cx - g->clipX1);
        for(; xo <= xmax; ++xo) {
            yo = sqrt(rro - square(xo));
            DrawVLineAA(g, color, cy - yo, yo0, cx - xo);
        }
    }
    else {
        uint8_t alpha = color >> 27;
        FP cx = x + r + FP::HALF;
        FP cy = y + r + FP::HALF;
        FP rr = square(r + FP::HALF);

        FP ymax = cy - g->clipY1;
        FP xo, yo = (cy - g->clipY2 > FP::ONE) ? floor(cy - g->clipY2) : FP::ONE;
        while(true) {
            if(yo > ymax) return;
            FP yy = square(yo);
            xo = sqrt(rr - yy);
            if(xo > yo) {
                uint8_t ai = (cx - xo).fraction(alpha);
                uint8_t ao = alpha - ai;
                int32_t px = cx - xo;
                int32_t py = cy - yo;
                ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px, py);
                if(ai > 0) ((Rgb565GfxHelper *)g)->blendPixel(ai, color, px + 1, py);
            }
            else break;
            ++yo;
        }

        FP xmax = GFX_MIN(floor(xo), cx - g->clipX1);
        xo = (cx - g->clipX2 > FP::ONE) ? floor(cx - g->clipX2) : FP::ONE;
        for(; xo <= xmax; ++xo) {
            yo = sqrt(rr - square(xo));
            uint8_t ai = (cy - yo).fraction(alpha);
            uint8_t ao = alpha - ai;
            int32_t px = cx - xo;
            int32_t py = cy - yo;
            ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px, py);
            if(ai > 0) ((Rgb565GfxHelper *)g)->blendPixel(ai, color, px, py + 1);
        }
    }
}

static void DrawQuarterCircle2(Rgb565Gfx *g, uint32_t color, int32_t thk, int32_t x, int32_t y, int32_t r) {
    if(!IsVisible(g, x, y, r, r)) return;

    if(thk > 1) {
        FP cx = x - FP::HALF;
        FP cy = y + r + FP::HALF;
        FP xo, xi, yi;
        FP rro = square(r + (FP)thk / (int32_t)2);
        FP rri = (thk >= (r << 1)) ? (FP)0 : square(r - (FP)thk / (int32_t)2);

        FP ymax = cy - g->clipY1;
        FP yo = (cy - g->clipY2 > FP::ONE) ? floor(cy - g->clipY2) : FP::ONE;
        while(true) {
            if(yo > ymax) return;
            FP yy = square(yo);
            xi = sqrt(rri - yy);
            if(xi >= yo) {
                xo = sqrt(rro - yy);
                DrawHLineAA(g, color, cx + xi, cx + xo, cy - yo);
            }
            else break;
            ++yo;
        }

        FP xi0 = floor(cx + xi) - cx + FP::ONE;
        while(true) {
            if(yo > ymax) return;
            xo = sqrt(rro - square(yo));
            if(xo >= yo) DrawHLineAA(g, color, (int32_t)(cx + xi0), cx + xo, cy - yo);
            else break;
            ++yo;
        }

        int32_t yo0 = cy - yo;
        FP xo0 = floor(xo);
        FP xmax = GFX_MIN(xi0, g->clipX2 - cx);
        xo = (cx - g->clipX2 > FP::ONE) ? floor(cx - g->clipX2) : FP::ONE;
        for(; xo < xmax; ++xo) {
            FP xx = square(xo);
            yo = sqrt(rro - xx);
            yi = sqrt(rri - xx);
            DrawVLineAA(g, color, cy - yo, cy - yi, cx + xo);
        }

        xmax = GFX_MIN(xo0, g->clipX2 - cx);
        for(; xo <= xmax; ++xo) {
            yo = sqrt(rro - square(xo));
            DrawVLineAA(g, color, cy - yo, yo0, cx + xo);
        }
    }
    else {
        uint8_t alpha = color >> 27;
        FP cx = x - FP::HALF;
        FP cy = y + r + FP::HALF;
        FP rr = square(r + FP::HALF);

        FP ymax = cy - g->clipY1;
        FP xo, yo = (cy - g->clipY2 > FP::ONE) ? floor(cy - g->clipY2) : FP::ONE;
        while(true) {
            if(yo > ymax) return;
            FP yy = square(yo);
            xo = sqrt(rr - yy);
            if(xo > yo) {
                uint8_t ao = (cx + xo).fraction(alpha);
                uint8_t ai = alpha - ao;
                int32_t px = cx + xo;
                int32_t py = cy - yo;
                ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px, py);
                if(ai > 0) ((Rgb565GfxHelper *)g)->blendPixel(ai, color, px - 1, py);
            }
            else break;
            ++yo;
        }

        FP xmax = GFX_MIN(floor(xo), g->clipX2 - cx);
        xo = (g->clipX1 - cx  > FP::ONE) ? floor(g->clipX1 - cx) : FP::ONE;
        for(; xo <= xmax; ++xo) {
            yo = sqrt(rr - square(xo));
            uint8_t ai = (cy - yo).fraction(alpha);
            uint8_t ao = alpha - ai;
            int32_t px = cx + xo;
            int32_t py = cy - yo;
            ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px, py);
            if(ai > 0) ((Rgb565GfxHelper *)g)->blendPixel(ai, color, px, py + 1);
        }
    }
}

static void DrawQuarterCircle3(Rgb565Gfx *g, uint32_t color, int32_t thk, int32_t x, int32_t y, int32_t r) {
    if(!IsVisible(g, x, y, r, r)) return;

    if(thk > 1) {
        FP cx = x - FP::HALF;
        FP cy = y - FP::HALF;
        FP xo, xi, yi;
        FP rro = square(r + (FP)thk / (int32_t)2);
        FP rri = (thk >= (r << 1)) ? (FP)0 : square(r - (FP)thk / (int32_t)2);

        FP ymax = g->clipY2 - cy;
        FP yo = (g->clipY1 - cy > FP::ONE) ? floor(g->clipY1 - cy) : FP::ONE;
        while(true) {
            if(yo > ymax) return;
            FP yy = square(yo);
            xi = sqrt(rri - yy);
            if(xi >= yo) {
                xo = sqrt(rro - yy);
                DrawHLineAA(g, color, cx + xi, cx + xo, cy + yo);
            }
            else break;
            ++yo;
        }

        FP xi0 = floor(cx + xi) - cx + FP::ONE;
        while(true) {
            if(yo > ymax) return;
            xo = sqrt(rro - square(yo));
            if(xo >= yo) DrawHLineAA(g, color, (int32_t)(cx + xi0), cx + xo, cy + yo);
            else break;
            ++yo;
        }

        int32_t yo0 = cy + yo;
        FP xo0 = floor(xo);
        FP xmax = GFX_MIN(xi0, g->clipX2 - cx);
        xo = (g->clipX1 - cx > FP::ONE) ? floor(g->clipX1 - cx) : FP::ONE;
        for(; xo < xmax; ++xo) {
            FP xx = square(xo);
            yo = sqrt(rro - xx);
            yi = sqrt(rri - xx);
            DrawVLineAA(g, color, cy + yi, cy + yo, cx + xo);
        }

        xmax = GFX_MIN(xo0, g->clipX2 - cx);
        for(; xo <= xmax; ++xo) {
            yo = sqrt(rro - square(xo));
            DrawVLineAA(g, color, yo0, cy + yo, cx + xo);
        }
    }
    else {
        uint8_t alpha = color >> 27;
        FP cx = x - FP::HALF;
        FP cy = y - FP::HALF;
        FP rr = square(r + FP::HALF);

        FP ymax = g->clipY2 - cy;
        FP xo, yo = (g->clipY1 - cy > FP::ONE) ? floor(g->clipY1 - cy) : FP::ONE;
        while(true) {
            if(yo > ymax) return;
            FP yy = square(yo);
            xo = sqrt(rr - yy);
            if(xo > yo) {
                uint8_t ao = (cx + xo).fraction(alpha);
                uint8_t ai = alpha - ao;
                int32_t px = cx + xo;
                int32_t py = cy + yo;
                ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px, py);
                if(ai > 0) ((Rgb565GfxHelper *)g)->blendPixel(ai, color, px - 1, py);
            }
            else break;
            ++yo;
        }

        FP xmax = GFX_MIN(floor(xo), g->clipX2 - cx);
        xo = (g->clipX1 - cx > FP::ONE) ? floor(g->clipX1 - cx) : FP::ONE;
        for(; xo <= xmax; ++xo) {
            yo = sqrt(rr - square(xo));
            uint8_t ao = (cy + yo).fraction(alpha);
            uint8_t ai = alpha - ao;
            int32_t px = cx + xo;
            int32_t py = cy + yo;
            ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px, py);
            if(ai > 0) ((Rgb565GfxHelper *)g)->blendPixel(ai, color, px, py - 1);
        }
    }
}

static void DrawQuarterCircle4(Rgb565Gfx *g, uint32_t color, int32_t thk, int32_t x, int32_t y, int32_t r) {
    if(!IsVisible(g, x, y, r, r)) return;

    if(thk > 1) {
        FP cx = x + r + FP::HALF;
        FP cy = y - FP::HALF;
        FP xo, xi, yi;
        FP rro = square(r + (FP)thk / (int32_t)2);
        FP rri = (thk >= (r << 1)) ? (FP)0 : square(r - (FP)thk / (int32_t)2);

        FP ymax = g->clipY2 - cy;
        FP yo = (g->clipY1 - cy > FP::ONE) ? floor(g->clipY1 - cy) : FP::ONE;
        while(true) {
            if(yo > ymax) return;
            FP yy = square(yo);
            xi = sqrt(rri - yy);
            if(xi >= yo) {
                xo = sqrt(rro - yy);
                DrawHLineAA(g, color, cx - xo, cx - xi, cy + yo);
            }
            else break;
            ++yo;
        }

        FP xi0 = cx - floor(cx - xi) + FP::ONE;
        while(true) {
            if(yo > ymax) return;
            xo = sqrt(rro - square(yo));
            if(xo >= yo) DrawHLineAA(g, color, cx - xo, (int32_t)(cx - xi0), cy + yo);
            else break;
            ++yo;
        }

        int32_t yo0 = cy + yo;
        FP xo0 = floor(xo);
        FP xmax = GFX_MIN(xi0 - FP::ONE, cx - g->clipX1);
        xo = (cx - g->clipX2 > FP::ONE) ? floor(cx - g->clipX2) : FP::ONE;
        for(; xo < xmax; ++xo) {
            FP xx = square(xo);
            yo = sqrt(rro - xx);
            yi = sqrt(rri - xx);
            DrawVLineAA(g, color, cy + yi, cy + yo, cx - xo);
        }

        xmax = GFX_MIN(xo0, cx - g->clipX1);
        for(; xo <= xmax; ++xo) {
            yo = sqrt(rro - square(xo));
            DrawVLineAA(g, color, yo0, cy + yo, cx - xo);
        }
    }
    else {
        uint8_t alpha = color >> 27;
        FP cx = x + r + FP::HALF;
        FP cy = y - FP::HALF;
        FP rr = square(r + FP::HALF);

        FP ymax = g->clipY2 - cy;
        FP xo, yo = (g->clipY1 - cy > FP::ONE) ? floor(g->clipY1 - cy) : FP::ONE;
        while(true) {
            if(yo > ymax) return;
            FP yy = square(yo);
            xo = sqrt(rr - yy);
            if(xo > yo) {
                uint8_t ai = (cx - xo).fraction(alpha);
                uint8_t ao = alpha - ai;
                int32_t px = cx - xo;
                int32_t py = cy + yo;
                ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px, py);
                if(ai > 0) ((Rgb565GfxHelper *)g)->blendPixel(ai, color, px + 1, py);
            }
            else break;
            ++yo;
        }

        FP xmax = GFX_MIN(floor(xo), cx - g->clipX1);
        xo = (cx - g->clipX2 > FP::ONE) ? floor(cx - g->clipX2) : FP::ONE;
        for(; xo <= xmax; ++xo) {
            yo = sqrt(rr - square(xo));
            uint8_t ao = (cy + yo).fraction(alpha);
            uint8_t ai = alpha - ao;
            int32_t px = cx - xo;
            int32_t py = cy + yo;
            ((Rgb565GfxHelper *)g)->blendPixel(ao, color, px, py);
            if(ai > 0) ((Rgb565GfxHelper *)g)->blendPixel(ai, color, px, py - 1);
        }
    }
}

void Rgb565Gfx::drawRoundRect(uint32_t color, int32_t thk, int32_t x, int32_t y, int32_t w, int32_t h, int32_t r1, int32_t r2, int32_t r3, int32_t r4) {
    if(thk < 1) thk = 1;
    if(!IsVisible(this, x - thk / 2, y - thk / 2, w + thk - (thk & 1), h + thk - (thk & 1))) return;
    RadiusAdjustment(w, h, r1, r2, r3, r4);

    uint8_t a1 = color >> 27;
    if(thk == 1) {
        ((Rgb565GfxHelper *)this)->blendHLine(a1, color, x + r1, x + w - r2, y);
        ((Rgb565GfxHelper *)this)->blendHLine(a1, color, x + r4, x + w - r3, y + h);
        ((Rgb565GfxHelper *)this)->blendVLine(a1, color, y + r1, y + h - r4, x);
        ((Rgb565GfxHelper *)this)->blendVLine(a1, color, y + r2, y + h - r3, x + w);
    }
    else {
        uint8_t a2 = color >> 28;
        int32_t half = (thk - 1) >> 1;

        /* top */
        int32_t x1 = r1 ? (x + r1) : (x - half);
        int32_t x2 = r2 ? (x + w - r2) : (x + w + half);
        int32_t y1 = y - half;
        int32_t y2 = y + half;
        ((Rgb565GfxHelper *)this)->blendRect(a1, color, x1, y1, x2, y2);
        if((thk & 1) == 0) {
            ((Rgb565GfxHelper *)this)->blendRect(a2, color, x1, y1 - 1, x2, y1 - 1);
            x1 = GFX_MAX(x1, x + half);
            x2 = GFX_MIN(x2, x + w - half);
            if(thk > (r1 << 1)) x1++;
            if(thk > (r2 << 1)) x2--;
            ((Rgb565GfxHelper *)this)->blendRect(a2, color, x1, y2 + 1, x2, y2 + 1);
        }

        /* Bottom */
        x1 = r4 ? (x + r4) : (x - half);
        x2 = r3 ? (x + w - r3) : (x + w + half);
        y1 = y + h - half;
        y2 = y + h + half;
        ((Rgb565GfxHelper *)this)->blendRect(a1, color, x1, y1, x2, y2);
        if((thk & 1) == 0) {
            ((Rgb565GfxHelper *)this)->blendRect(a2, color, x1, y2 + 1, x2, y2 + 1);
            x1 = GFX_MAX(x1, x + half);
            x2 = GFX_MIN(x2, x + w - half);
            if(thk > (r4 << 1)) x1++;
            if(thk > (r3 << 1)) x2--;
            ((Rgb565GfxHelper *)this)->blendRect(a2, color, x1, y1 - 1, x2, y1 - 1);
        }

        /* Left */
        x1 = x - half;
        x2 = GFX_MIN(x + half, GFX_MIN(x + r1, x + r4));
        y1 = y + r1;
        y2 = y + h - r4;
        if((thk & 1) == 0) ((Rgb565GfxHelper *)this)->blendVLine(a2, color, y1, y2, x1 - 1);
        for(; x1 < x2; x1++) ((Rgb565GfxHelper *)this)->blendVLine(a1, color, y1, y2, x1);

        x2 = GFX_MIN(x + half, GFX_MAX(x + r1, x + r4));
        if(r1 < r4) y1 = y + half + 1;
        else y2 = y + h - half - 1;
        for(; x1 < x2; x1++) ((Rgb565GfxHelper *)this)->blendVLine(a1, color, y1, y2, x1);

        x2 = x + half;
        y1 = GFX_MAX(y + r1, y + half + 1);
        y2 = GFX_MIN(y + h - r4, y + h - half - 1);
        for(; x1 <= x2; x1++) ((Rgb565GfxHelper *)this)->blendVLine(a1, color, y1, y2, x1);
        if((thk & 1) == 0) ((Rgb565GfxHelper *)this)->blendVLine(a2, color, y1, y2, x2 + 1);

        /* Right */
        x1 = x + w - half;
        x2 = GFX_MIN(x + w - r2, x + w - r3);
        y1 = GFX_MAX(y + r2, y + half + 1);
        y2 = GFX_MIN(y + h - r3, y + h - half - 1);
        if((thk & 1) == 0) ((Rgb565GfxHelper *)this)->blendVLine(a2, color, y1, y2, x1 - 1);
        for(; x1 <= x2; x1++) ((Rgb565GfxHelper *)this)->blendVLine(a1, color, y1, y2, x1);

        x2 = GFX_MAX(x + w - r2, x + w - r3);
        if(r2 > r3) y1 = y + r2;
        else y2 = y + h - r3;
        for(; x1 <= x2; x1++) ((Rgb565GfxHelper *)this)->blendVLine(a1, color, y1, y2, x1);

        x2 = x + w + half;
        y1 = y + r2;
        y2 = y + h - r3;
        for(; x1 <= x2; x1++) ((Rgb565GfxHelper *)this)->blendVLine(a1, color, y1, y2, x1);
        if((thk & 1) == 0) ((Rgb565GfxHelper *)this)->blendVLine(a2, color, y1, y2, x2 + 1);
    }
    if(r1 > 0) DrawQuarterCircle1(this, color, thk, x, y, r1);
    if(r2 > 0) DrawQuarterCircle2(this, color, thk, x + w - r2 + 1, y, r2);
    if(r3 > 0) DrawQuarterCircle3(this, color, thk, x + w - r3 + 1, y + h - r3 + 1, r3);
    if(r4 > 0) DrawQuarterCircle4(this, color, thk, x, y + h - r4 + 1, r4);
}

static void FillQuarterCircle1(Rgb565Gfx *g, uint32_t color, int32_t x, int32_t y, int32_t r) {
    if(!IsVisible(g, x, y, r, r)) return;

    int32_t cx = x + r;
    int32_t cy = y + r;
    FP aa = r * r;

    FP xo;
    FP ymax = cy - g->clipY1;
    FP yo = (cy - g->clipY2 > 1) ? (cy - g->clipY2 - FP::HALF) : FP::HALF;
    while(true) {
        if(yo > ymax) return;
        xo = sqrt(aa - square(yo));
        if(xo >= yo) DrawHLineAA(g, color, cx - xo, cx - 1, cy - yo);
        else break;
        ++yo;
    }

    int32_t yo0 = cy - yo;
    FP xmax = GFX_MIN(xo, (FP)(cx - g->clipX1));
    xo = (cx - g->clipX2 > 1) ? (cx - g->clipX2 - FP::HALF) : FP::HALF;
    for(; xo <= xmax; ++xo) {
        yo = sqrt(aa - square(xo));
        DrawVLineAA(g, color, cy - yo, yo0, cx - xo);
    }
}

static void FillQuarterCircle2(Rgb565Gfx *g, uint32_t color, int32_t x, int32_t y, int32_t r) {
    if(!IsVisible(g, x, y, r, r)) return;

    int32_t cy = y + r;
    FP aa = r * r;

    FP xo;
    FP ymax = cy - g->clipY1;
    FP yo = (cy - g->clipY2 > 1) ? (cy - g->clipY2 - FP::HALF) : FP::HALF;
    while(true) {
        if(yo > ymax) return;
        xo = sqrt(aa - square(yo));
        if(xo >= yo) DrawHLineAA(g, color, x, x + xo, cy - yo);
        else break;
        ++yo;
    }

    int32_t yo0 = cy - yo;
    FP xmax = GFX_MIN(xo, (FP)(g->clipX2 - x));
    xo = (g->clipX1 - x > 1) ? (g->clipX1 - x - FP::HALF) : FP::HALF;
    for(; xo <= xmax; ++xo) {
        yo = sqrt(aa - square(xo));
        DrawVLineAA(g, color, cy - yo, yo0, x + xo);
    }
}

static void FillQuarterCircle3(Rgb565Gfx *g, uint32_t color, int32_t x, int32_t y, int32_t r) {
    if(!IsVisible(g, x, y, r, r)) return;

    FP aa = r * r;
    FP xo;
    FP ymax = g->clipY2 - y;
    FP yo = (g->clipY1 - y > 1) ? (g->clipY1 - y - FP::HALF) : FP::HALF;
    while(true) {
        if(yo > ymax) return;
        xo = sqrt(aa - square(yo));
        if(xo >= yo) DrawHLineAA(g, color, x, x + xo, y + yo);
        else break;
        ++yo;
    }

    int32_t yo0 = y + yo;
    FP xmax = GFX_MIN(xo, (FP)(g->clipX2 - x));
    xo = (g->clipX1 - x > 1) ? (g->clipX1 - x - FP::HALF) : FP::HALF;
    for(; xo <= xmax; ++xo) {
        yo = sqrt(aa - square(xo));
        DrawVLineAA(g, color, yo0, y + yo, x + xo);
    }
}

static void FillQuarterCircle4(Rgb565Gfx *g, uint32_t color, int32_t x, int32_t y, int32_t r) {
    if(!IsVisible(g, x, y, r, r)) return;

    int32_t cx = x + r;
    FP aa = r * r;

    FP xo;
    FP ymax = g->clipY2 - y;
    FP yo = (g->clipY1 - y > 1) ? (g->clipY1 - y - FP::HALF) : FP::HALF;
    while(true) {
        if(yo > ymax) return;
        xo = sqrt(aa - square(yo));
        if(xo >= yo) DrawHLineAA(g, color, cx - xo, cx - 1, y + yo);
        else break;
        ++yo;
    }

    int32_t yo0 = y + yo;
    FP xmax = GFX_MIN(xo, (FP)(cx - g->clipX1));
    xo = (cx - g->clipX2 > 1) ? (cx - g->clipX2 - FP::HALF) : FP::HALF;
    for(; xo <= xmax; ++xo) {
        yo = sqrt(aa - square(xo));
        DrawVLineAA(g, color, yo0, y + yo, cx - xo);
    }
}

void Rgb565Gfx::fillRoundRect(uint32_t color, int32_t x, int32_t y, int32_t w, int32_t h, int32_t r1, int32_t r2, int32_t r3, int32_t r4) {
    if(!IsVisible(this, x, y, w, h)) return;
    RadiusAdjustment(w, h, r1, r2, r3, r4);

    uint8_t alpha = color >> 27;
    int32_t x1, x2;
    int32_t y1 = GFX_MAX(clipY1 - y, 0);
    int32_t y2 = GFX_MIN(clipY2 - y, h);

    for(int32_t i = y1; i <= y2; i++) {
        x1 = x + (i < r1 ? r1 : (i > (h - r4) ? r4 : 0));
        x2 = x + w - (i < r2 ? r2 : (i > (h - r3) ? r3 : 0));
        ((Rgb565GfxHelper *)this)->blendHLine(alpha, color, x1, x2, i + y);
    }

    if(r1 > 0) FillQuarterCircle1(this, color, x, y, r1);
    if(r2 > 0) FillQuarterCircle2(this, color, x + w - r2 + 1, y, r2);
    if(r3 > 0) FillQuarterCircle3(this, color, x + w - r3 + 1, y + h - r3 + 1, r3);
    if(r4 > 0) FillQuarterCircle4(this, color, x, y + h - r4 + 1, r4);
}

static void DrawImageRGB565(Rgb565Gfx *g, Image *img, int32_t x, int32_t y) {
    uint16_t imgx = GFX_MAX(g->clipX1, x) - x;
    uint16_t imgy = GFX_MAX(g->clipY1, y) - y;
    uint16_t imgw = GFX_MIN(g->clipX2 + 1, x + img->width) - x;
    uint16_t imgh = GFX_MIN(g->clipY2 + 1, y + img->height) - y;
    if(imgx >= imgw) return;
    for(; imgy < imgh; imgy++) {
        uint16_t *src = &((uint16_t *)img->data)[imgy * img->width];
        uint16_t *des = &((uint16_t *)g->data)[(imgy + y) * g->width];
        memcpy(des, src, (imgw - imgx) * 2);
    }
}

static void DrawImageARGB565(Rgb565Gfx *g, Image *img, int32_t x, int32_t y) {
    uint8_t *alpha = &((uint8_t *)img->data)[img->width * img->height * 2];
    uint16_t imgx = GFX_MAX(g->clipX1, x) - x;
    uint16_t imgy = GFX_MAX(g->clipY1, y) - y;
    uint16_t imgw = GFX_MIN(g->clipX2 + 1, x + img->width) - x;
    uint16_t imgh = GFX_MIN(g->clipY2 + 1, y + img->height) - y;
    if(imgx >= imgw) return;
    for(; imgy < imgh; imgy++) {
        uint16_t *src = &((uint16_t *)img->data)[imgy * img->width];
        uint16_t *des = &((uint16_t *)g->data)[(imgy + y) * g->width];
        uint32_t aIdx = imgy * img->width + imgx;
        for(uint16_t i = imgx; i < imgw; i++) {
            uint8_t a = (aIdx & 1) ? (alpha[aIdx >> 1] >> 4) : (alpha[aIdx >> 1] & 0x0F);
            uint32_t bg = __builtin_bswap16(des[i]);
            uint32_t fg = __builtin_bswap16(src[i]);
            bg = (bg | (bg << 16)) & 0x07E0F81F;
            fg = (fg | (fg << 16)) & 0x07E0F81F;
            bg += (((fg) - bg) * a) >> 4;
            bg &= 0x07E0F81F;
            bg = (bg | (bg >> 16));
            des[i] = __builtin_bswap16(bg);
            aIdx++;
        }
    }
}

static void DrawImageARGB888(Rgb565Gfx *g, Image *img, int32_t x, int32_t y) {
    // TODO
}

void Rgb565Gfx::drawImage(Image *img, int32_t x, int32_t y) {
    switch(img->format) {
        case IMG_RGB565: return DrawImageRGB565(this, img, x, y);
        case IMG_ARGB565: return DrawImageARGB565(this, img, x, y);
        default: return DrawImageARGB888(this, img, x, y);
    }
}

static void DrawImageRGB565(Rgb565Gfx *g, Image *img, int32_t x, int32_t y, uint16_t w, uint16_t h) {
    if(w == img->width && h == img->height) return DrawImageRGB565(g, img, x, y);
    uint16_t imgx = GFX_MAX(g->clipX1, x) - x;
    uint16_t imgy = GFX_MAX(g->clipY1, y) - y;
    uint16_t imgw = GFX_MIN(g->clipX2 + 1, x + w) - x;
    uint16_t imgh = GFX_MIN(g->clipY2 + 1, y + h) - y;
    if(imgx >= imgw) return;
    for(; imgy < imgh; imgy++) {
        uint16_t *src = &((uint16_t *)img->data)[(imgy * img->height / h) * img->width];
        uint16_t *des = &((uint16_t *)g->data)[(imgy + y) * g->width];
        for(uint16_t i = imgx; i < imgw; i++) des[i] = src[i * img->width / w];
    }
}

static void DrawImageARGB565(Rgb565Gfx *g, Image *img, int32_t x, int32_t y, uint16_t w, uint16_t h) {
    if(w == img->width && h == img->height) return DrawImageARGB565(g, img, x, y);
    uint8_t *alpha = &((uint8_t *)img->data)[img->width * img->height * 2];
    uint16_t imgx = GFX_MAX(g->clipX1, x) - x;
    uint16_t imgy = GFX_MAX(g->clipY1, y) - y;
    uint16_t imgw = GFX_MIN(g->clipX2 + 1, x + w) - x;
    uint16_t imgh = GFX_MIN(g->clipY2 + 1, y + h) - y;
    if(imgx >= imgw) return;
    for(; imgy < imgh; imgy++) {
        uint32_t tmp = (imgy * img->height / h) * img->width;
        uint16_t *src = &((uint16_t *)img->data)[tmp];
        uint16_t *des = &((uint16_t *)g->data)[(imgy + y) * g->width];
        for(uint16_t i = imgx; i < imgw; i++) {
            uint32_t srcIdx = i * img->width / w;
            uint32_t aIdx = tmp + srcIdx;
            uint8_t a = (aIdx & 1) ? (alpha[aIdx >> 1] >> 4) : (alpha[aIdx >> 1] & 0x0F);
            uint32_t bg = __builtin_bswap16(des[i]);
            uint32_t fg = __builtin_bswap16(src[srcIdx]);
            bg = (bg | (bg << 16)) & 0x07E0F81F;
            fg = (fg | (fg << 16)) & 0x07E0F81F;
            bg += (((fg) - bg) * a) >> 4;
            bg &= 0x07E0F81F;
            bg = (bg | (bg >> 16));
            des[i] = __builtin_bswap16(bg);
        }
    }
}

static void DrawImageARGB888(Rgb565Gfx *g, Image *img, int32_t x, int32_t y, uint16_t w, uint16_t h) {
    if(w == img->width && h == img->height) return DrawImageARGB888(g, img, x, y);
    // TODO
}

void Rgb565Gfx::drawImage(Image *img, int32_t x, int32_t y, uint16_t w, uint16_t h) {
    switch(img->format) {
        case IMG_RGB565: return DrawImageRGB565(this, img, x, y, w, h);
        case IMG_ARGB565: return DrawImageARGB565(this, img, x, y, w, h);
        default: return DrawImageARGB888(this, img, x, y, w, h);
    }
}

static void DrawChar(Rgb565GfxHelper *g, const CharInfo *c, uint32_t color, int32_t x, int32_t y) {
    int32_t cx0 = GFX_MAX(g->clipX1 - x, 0);
    int32_t cxw = GFX_MIN(g->clipX2 - x + 1, (int32_t)c->getWidth());
    if(cx0 >= cxw) return;

    uint8_t height = c->getHeight();
    int8_t yOff = c->getYOffset();
    uint8_t alpha = color >> 27;

    if(alpha == 0x1F) {
        for(int32_t cy = 0; cy < height; cy++) {
            int32_t py = y + cy + yOff;
            if(!(g->clipY1 <= y && y <= g->clipY2)) continue;
            uint16_t *p = &((uint16_t *)g->data)[py * g->width + x];
            for(int32_t cx = cx0; cx < cxw; cx++)
                if(c->getPixel(cx, cy)) p[cx] = color;
        }
    }
    else {
        for(int32_t cy = 0; cy < height; cy++) {
            int32_t py = y + cy + yOff;
            if(!(g->clipY1 <= y && y <= g->clipY2)) continue;
            for(int32_t cx = cx0; cx < cxw; cx++)
                if(c->getPixel(cx, cy)) g->blendPixel(alpha, color, x + cx, py);
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
            DrawChar((Rgb565GfxHelper *)this, c, color, x, y);
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
            DrawChar((Rgb565GfxHelper *)this, c, color, x, y);
            x += c->getWidth() + space;
        }
        else {
            fillRect(color, x, y, stdWidth - 1, stdHeight - 1);
            x += stdWidth + space;
        }
        str += 2;
    }
}
