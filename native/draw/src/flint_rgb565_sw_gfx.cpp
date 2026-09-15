
#include <string.h>
#include "flint_rgb565_gfx.h"
#include "flint_fixed_point.h"
#include "flint_rgb565_gfx_helper.h"

static inline bool isVisible(Rgb565Gfx *g, int32_t x, int32_t y, int32_t w, int32_t h) {
    if(x > g->clipX2 || (x + w) <= g->clipX1) return false;
    if(y > g->clipY2 || (y + h) <= g->clipY1) return false;
    return true;
}

Rgb565Gfx::Rgb565Gfx(int32_t w, int32_t h, int32_t clipX1, int32_t clipY1, int32_t clipX2, int32_t clipY2, uint8_t *data) :
width(w), height(h), clipX1(clipX1), clipY1(clipY1), clipX2(clipX2), clipY2(clipY2), data(data) {

}

void Rgb565Gfx::clear(uint32_t color) {
    ((Rgb565GfxHelper *)this)->clear(color);
}

void Rgb565Gfx::drawLine(uint32_t color, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    if(x1 == x2 && y1 == y2)
        return;
    uint8_t alpha = color >> 27;
    if(y1 == y2) {
        if(x1 > x2) GFX_SWAP(x1, x2);
        ((Rgb565GfxHelper *)this)->blendHLine(alpha, color, x1, x2, y1);
    }
    else if(x1 == x2) {
        if(y1 > y2) GFX_SWAP(y1, y2);
        ((Rgb565GfxHelper *)this)->blendVLine(alpha, color, y1, y2, x1);
    }
    else {
        int32_t err = 0;
        int32_t dx = GFX_ABS(x2 - x1);
        int32_t dy = GFX_ABS(y2 - y1);
        if(dx > dy) {
            if(x1 > x2) {
                GFX_SWAP(x1, x2);
                GFX_SWAP(y1, y2);
            }
            int8_t ystep = (y1 < y2) ? 1 : -1;
            int32_t y = y1;
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
            if(y1 > y2) {
                GFX_SWAP(x1, x2);
                GFX_SWAP(y1, y2);
            }
            int8_t xstep = (x1 < x2) ? 1 : -1;
            int32_t x = x1;
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
}

void Rgb565Gfx::drawRect(uint32_t color, int32_t x, int32_t y, int32_t w, int32_t h) {
    if(!isVisible(this, x, y, w, h)) return;
    uint8_t alpha = color >> 27;

    int32_t xo1 = x;
    int32_t yo1 = y;
    int32_t xo2 = x + w;
    int32_t yo2 = y + h;

    int32_t xi1 = x;
    int32_t yi1 = y;
    int32_t xi2 = x + w;
    int32_t yi2 = y + h;

    ((Rgb565GfxHelper *)this)->blendRect(alpha, color, xo1, yo1, xo2, yi1);
    ((Rgb565GfxHelper *)this)->blendRect(alpha, color, xo1, yi2, xo2, yo2);

    yi1++;
    yi2--;

    ((Rgb565GfxHelper *)this)->blendRect(alpha, color, xo1, yi1, xi1, yi2);
    ((Rgb565GfxHelper *)this)->blendRect(alpha, color, xi2, yi1, xo2, yi2);
}

void Rgb565Gfx::fillRect(uint32_t color, int32_t x, int32_t y, int32_t w, int32_t h) {
    ((Rgb565GfxHelper *)this)->blendRect(color >> 27, color, x, y, x + w - 1, y + h - 1);
}

static void radiusAdjustment(int32_t w, int32_t h, int32_t &r1, int32_t &r2, int32_t &r3, int32_t &r4) {
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

static void plotH2(Rgb565Gfx *g, uint32_t color, FP cx, int32_t cy, FP x) {
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

static void plotH4(Rgb565Gfx *g, uint32_t color, FP cx, FP cy, FP x, FP y) {
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

static void plotV2(Rgb565Gfx *g, uint32_t color, int32_t cx, FP cy, FP y) {
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

static void plotV4(Rgb565Gfx *g, uint32_t color, FP cx, FP cy, FP x, FP y) {
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

void Rgb565Gfx::drawCircle(uint32_t color, int32_t x, int32_t y, uint32_t d) {
    if(!isVisible(this, x, y, d, d)) return;
    d++;
    FP cx = x + (FP)d / (int32_t)2;
    FP cy = y + (FP)d / (int32_t)2;
    FP rr = square((FP)d / (int32_t)2);
    FP yo, xo;
    FP ymax = FP::absMax(cy - clipY1, clipY2 - cy);
    if(cy - clipY2 > FP::ONE) yo = (d & 1) ? floor(cy - clipY2) : (floor(cy - clipY2) - FP::HALF);
    else if(clipY1 - cy > FP::ONE) yo = (d & 1) ? floor(clipY1 - cy) : (floor(clipY1 - cy) - FP::HALF);
    else yo = (d & 1) ? FP::ONE : FP::HALF;
    if(d & 1) {
        plotH2(this, color, cx, cy, (FP)d / (int32_t)2);
        plotV2(this, color, cx, cy, (FP)d / (int32_t)2);
    }
    while(true) {
        if(yo > ymax) return;
        xo = sqrt(rr - square(yo));
        if(xo >= yo) plotH4(this, color, cx, cy, xo, yo);
        else break;
        ++yo;
    }

    FP xmax = GFX_MIN(xo, GFX_MAX(cx - clipX1, clipX2 - cx));
    if(cx - clipX2 > FP::ONE) xo = (d & 1) ? floor(cx - clipX2) : (floor(cx - clipX2) - FP::HALF);
    else if(clipX1 - cx > FP::ONE) xo = (d & 1) ? floor(clipX1 - cx) : (floor(clipX1 - cx) - FP::HALF);
    else xo = (d & 1) ? FP::ONE : FP::HALF;
    for(; xo < xmax; ++xo) {
        yo = sqrt(rr - square(xo));
        plotV4(this, color, cx, cy, xo, yo);
    }
}

void Rgb565Gfx::drawEllipse(uint32_t color, int32_t x, int32_t y, uint32_t w, uint32_t h) {
    if(w == h) return drawCircle(color, x, y, w);
    if(!isVisible(this, x, y, w, h)) return;
    w++;
    h++;
    FP cx = x + (FP)w / (int32_t)2;
    FP cy = y + (FP)h / (int32_t)2;
    FP aa = square((FP)w / (int32_t)2);
    FP bb = square((FP)h / (int32_t)2);
    FP ab = aa * bb;

    FP yo, xo;
    FP ymax = FP::absMax(cy - clipY1, clipY2 - cy);
    if(cy - clipY2 > FP::ONE) yo = (h & 1) ? floor(cy - clipY2) : (floor(cy - clipY2) - FP::HALF);
    else if(clipY1 - cy > FP::ONE) yo = (h & 1) ? floor(clipY1 - cy) : (floor(clipY1 - cy) - FP::HALF);
    else yo = (h & 1) ? FP::ONE : FP::HALF;
    if(h & 1) plotH2(this, color, cx, cy, (FP)w / (int32_t)2);
    if(w & 1) plotV2(this, color, cx, cy, (FP)h / (int32_t)2);
    while(true) {
        if(yo > ymax) return;
        xo = sqrt((ab - aa * square(yo)) / bb);
        if(bb * xo >= aa * yo) plotH4(this, color, cx, cy, xo, yo);
        else break;
        ++yo;
    }

    FP xmax = GFX_MIN(xo, GFX_MAX(cx - clipX1, clipX2 - cx));
    if(cx - clipX2 > FP::ONE) xo = (w & 1) ? floor(cx - clipX2) : (floor(cx - clipX2) - FP::HALF);
    else if(clipX1 - cx > FP::ONE) xo = (w & 1) ? floor(clipX1 - cx) : (floor(clipX1 - cx) - FP::HALF);
    else xo = (w & 1) ? FP::ONE : FP::HALF;
    for(; xo < xmax; ++xo) {
        yo = sqrt((ab - bb * square(xo)) / aa);
        plotV4(this, color, cx, cy, xo, yo);
    }
}

static void drawHLineAA(Rgb565Gfx *g, uint32_t color, FP x1, FP x2, int32_t y) {
    uint8_t alpha = color >> 27;
    uint8_t a1 = alpha - x1.fraction(alpha);
    uint8_t a2 = x2.fraction(alpha);
    int32_t px1 = x1;
    int32_t px2 = x2;
    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, px1 + 1, px2 - 1, y);
    if(a1 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a1, color, px1, y);
    if(a2 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a2, color, px2, y);
}

static void drawHLineAA(Rgb565Gfx *g, uint32_t color, FP x1, int32_t x2, int32_t y) {
    int32_t px1 = x1;
    uint8_t alpha = color >> 27;
    uint8_t a1 = alpha - x1.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, px1 + 1, x2, y);
    if(a1 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a1, color, px1, y);
}

static void drawHLineAA(Rgb565Gfx *g, uint32_t color, int32_t x1, FP x2, int32_t y) {
    int32_t px2 = x2;
    uint8_t alpha = color >> 27;
    uint8_t a2 = x2.fraction(alpha);
    ((Rgb565GfxHelper *)g)->blendHLine(alpha, color, x1, px2 - 1, y);
    if(a2 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a2, color, px2, y);
}

static void drawVLineAA(Rgb565Gfx *g, uint32_t color, FP y1, int32_t y2, int32_t x) {
    uint8_t alpha = color >> 27;
    uint8_t a1 = alpha - y1.fraction(alpha);
    int32_t py = y1;
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, py + 1, y2, x);
    if(a1 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a1, color, x, py);
}

static void drawVLineAA(Rgb565Gfx *g, uint32_t color, int32_t y1, FP y2, int32_t x) {
    uint8_t alpha = color >> 27;
    uint8_t a2 = y2.fraction(alpha);
    int32_t py = y2;
    ((Rgb565GfxHelper *)g)->blendVLine(alpha, color, y1, py - 1, x);
    if(a2 > 0) ((Rgb565GfxHelper *)g)->blendPixel(a2, color, x, py);
}

static void fillEllipseHLine2(Rgb565Gfx *g, uint32_t color, FP cx, FP cy, FP x, FP y) {
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

static void fillEllipseVLine2(Rgb565Gfx *g, uint32_t color, int32_t cx, FP cy, FP yo, FP yi) {
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

static void fillEllipseVLine4(Rgb565Gfx *g, uint32_t color, FP cx, FP cy, FP yo, FP yi, FP x) {
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
    if(!isVisible(this, x, y, d, d)) return;
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
    if(d & 1) drawHLineAA(this, color, cx - r, cx + r, cy);
    while(true) {
        if(yo > ymax) return;
        xo = sqrt(rr - square(yo));
        if(xo >= yo) fillEllipseHLine2(this, color, cx, cy, xo, yo);
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
        fillEllipseVLine4(this, color, cx, cy, yo, yo0, xo);
    }
    if(d & 1) fillEllipseVLine2(this, color, cx, cy, r, yo0);
}

void Rgb565Gfx::fillEllipse(uint32_t color, int32_t x, int32_t y, uint32_t w, uint32_t h) {
    if(w == h) return fillCircle(color, x, y, w);
    if(!isVisible(this, x, y, w, h)) return;

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
    if(h & 1) drawHLineAA(this, color, cx - a, cx + a, cy);
    while(true) {
        if(yo > ymax) return;
        xo = sqrt((ab - aa * square(yo)) / bb);
        if(bb * xo >= aa * yo) fillEllipseHLine2(this, color, cx, cy, xo, yo);
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
        fillEllipseVLine4(this, color, cx, cy, yo, yo0, xo);
    }
    if(w & 1) fillEllipseVLine2(this, color, cx, cy, b, yo0);
}

static void drawQuarterCircle1(Rgb565Gfx *g, uint32_t color, int32_t x, int32_t y, int32_t r) {
    if(!isVisible(g, x, y, r, r)) return;

    uint8_t alpha = color >> 27;
    FP cx = x + r + FP::HALF;
    FP cy = y + r + FP::HALF;
    FP rr = square(r + FP::HALF);

    FP xo, yo = (cy - g->clipY2 > FP::ONE) ? floor(cy - g->clipY2) : FP::ONE;
    while(true) {
        int32_t py = cy - yo;
        if(py < g->clipY1) return;
        FP yy = square(yo);
        xo = sqrt(rr - yy);
        if(xo > yo) {
            uint8_t ai = (cx - xo).fraction(alpha);
            uint8_t ao = alpha - ai;
            int32_t px = cx - xo;
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

static void drawQuarterCircle2(Rgb565Gfx *g, uint32_t color, int32_t x, int32_t y, int32_t r) {
    if(!isVisible(g, x, y, r, r)) return;

    uint8_t alpha = color >> 27;
    FP cx = x - FP::HALF;
    FP cy = y + r + FP::HALF;
    FP rr = square(r + FP::HALF);

    FP xo, yo = (cy - g->clipY2 > FP::ONE) ? floor(cy - g->clipY2) : FP::ONE;
    while(true) {
        int32_t py = cy - yo;
        if(py < g->clipY1) return;
        FP yy = square(yo);
        xo = sqrt(rr - yy);
        if(xo > yo) {
            uint8_t ao = (cx + xo).fraction(alpha);
            uint8_t ai = alpha - ao;
            int32_t px = cx + xo;
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

static void drawQuarterCircle3(Rgb565Gfx *g, uint32_t color, int32_t x, int32_t y, int32_t r) {
    if(!isVisible(g, x, y, r, r)) return;

    uint8_t alpha = color >> 27;
    FP cx = x - FP::HALF;
    FP cy = y - FP::HALF;
    FP rr = square(r + FP::HALF);

    FP xo, yo = (g->clipY1 - cy > FP::ONE) ? floor(g->clipY1 - cy) : FP::ONE;
    while(true) {
        int32_t py = cy + yo;
        if(py > g->clipY2) return;
        FP yy = square(yo);
        xo = sqrt(rr - yy);
        if(xo > yo) {
            uint8_t ao = (cx + xo).fraction(alpha);
            uint8_t ai = alpha - ao;
            int32_t px = cx + xo;
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

static void drawQuarterCircle4(Rgb565Gfx *g, uint32_t color, int32_t x, int32_t y, int32_t r) {
    if(!isVisible(g, x, y, r, r)) return;

    uint8_t alpha = color >> 27;
    FP cx = x + r + FP::HALF;
    FP cy = y - FP::HALF;
    FP rr = square(r + FP::HALF);

    FP xo, yo = (g->clipY1 - cy > FP::ONE) ? floor(g->clipY1 - cy) : FP::ONE;
    while(true) {
        int32_t py = cy + yo;
        if(py > g->clipY2) return;
        FP yy = square(yo);
        xo = sqrt(rr - yy);
        if(xo > yo) {
            uint8_t ai = (cx - xo).fraction(alpha);
            uint8_t ao = alpha - ai;
            int32_t px = cx - xo;
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

void Rgb565Gfx::drawRoundRect(uint32_t color, int32_t x, int32_t y, int32_t w, int32_t h, int32_t r1, int32_t r2, int32_t r3, int32_t r4) {
    if(!isVisible(this, x, y, w, h)) return;
    radiusAdjustment(w, h, r1, r2, r3, r4);

    uint8_t a1 = color >> 27;
    ((Rgb565GfxHelper *)this)->blendHLine(a1, color, x + r1, x + w - r2, y);
    ((Rgb565GfxHelper *)this)->blendHLine(a1, color, x + r4, x + w - r3, y + h);
    ((Rgb565GfxHelper *)this)->blendVLine(a1, color, y + r1, y + h - r4, x);
    ((Rgb565GfxHelper *)this)->blendVLine(a1, color, y + r2, y + h - r3, x + w);
    if(r1 > 0) drawQuarterCircle1(this, color, x, y, r1);
    if(r2 > 0) drawQuarterCircle2(this, color, x + w - r2 + 1, y, r2);
    if(r3 > 0) drawQuarterCircle3(this, color, x + w - r3 + 1, y + h - r3 + 1, r3);
    if(r4 > 0) drawQuarterCircle4(this, color, x, y + h - r4 + 1, r4);
}

static void fillQuarterCircle1(Rgb565Gfx *g, uint32_t color, int32_t x, int32_t y, int32_t r) {
    if(!isVisible(g, x, y, r, r)) return;

    int32_t cx = x + r;
    int32_t cy = y + r;
    FP aa = r * r;

    FP xo;
    FP yo = (cy - g->clipY2 > 1) ? (cy - g->clipY2 - FP::HALF) : FP::HALF;
    while(true) {
        int32_t py = cy - yo;
        if(py < g->clipY1) return;
        xo = sqrt(aa - square(yo));
        if(xo >= yo) drawHLineAA(g, color, cx - xo, cx - 1, py);
        else break;
        ++yo;
    }

    int32_t yo0 = cy - yo;
    FP xmax = GFX_MIN(xo, (FP)(cx - g->clipX1));
    xo = (cx - g->clipX2 > 1) ? (cx - g->clipX2 - FP::HALF) : FP::HALF;
    for(; xo <= xmax; ++xo) {
        yo = sqrt(aa - square(xo));
        drawVLineAA(g, color, cy - yo, yo0, cx - xo);
    }
}

static void fillQuarterCircle2(Rgb565Gfx *g, uint32_t color, int32_t x, int32_t y, int32_t r) {
    if(!isVisible(g, x, y, r, r)) return;

    int32_t cy = y + r;
    FP aa = r * r;

    FP xo;
    FP yo = (cy - g->clipY2 > 1) ? (cy - g->clipY2 - FP::HALF) : FP::HALF;
    while(true) {
        int32_t py = cy - yo;
        if(py < g->clipY1) return;
        xo = sqrt(aa - square(yo));
        if(xo >= yo) drawHLineAA(g, color, x, x + xo, py);
        else break;
        ++yo;
    }

    int32_t yo0 = cy - yo;
    FP xmax = GFX_MIN(xo, (FP)(g->clipX2 - x));
    xo = (g->clipX1 - x > 1) ? (g->clipX1 - x - FP::HALF) : FP::HALF;
    for(; xo <= xmax; ++xo) {
        yo = sqrt(aa - square(xo));
        drawVLineAA(g, color, cy - yo, yo0, x + xo);
    }
}

static void fillQuarterCircle3(Rgb565Gfx *g, uint32_t color, int32_t x, int32_t y, int32_t r) {
    if(!isVisible(g, x, y, r, r)) return;

    FP aa = r * r;
    FP xo;
    FP yo = (g->clipY1 - y > 1) ? (g->clipY1 - y - FP::HALF) : FP::HALF;
    while(true) {
        int32_t py = y + yo;
        if(py >  g->clipY2) return;
        xo = sqrt(aa - square(yo));
        if(xo >= yo) drawHLineAA(g, color, x, x + xo, py);
        else break;
        ++yo;
    }

    int32_t yo0 = y + yo;
    FP xmax = GFX_MIN(xo, (FP)(g->clipX2 - x));
    xo = (g->clipX1 - x > 1) ? (g->clipX1 - x - FP::HALF) : FP::HALF;
    for(; xo <= xmax; ++xo) {
        yo = sqrt(aa - square(xo));
        drawVLineAA(g, color, yo0, y + yo, x + xo);
    }
}

static void fillQuarterCircle4(Rgb565Gfx *g, uint32_t color, int32_t x, int32_t y, int32_t r) {
    if(!isVisible(g, x, y, r, r)) return;

    int32_t cx = x + r;
    FP aa = r * r;

    FP xo;
    FP yo = (g->clipY1 - y > 1) ? (g->clipY1 - y - FP::HALF) : FP::HALF;
    while(true) {
        int32_t py = y + yo;
        if(py >  g->clipY2) return;
        xo = sqrt(aa - square(yo));
        if(xo >= yo) drawHLineAA(g, color, cx - xo, cx - 1, py);
        else break;
        ++yo;
    }

    int32_t yo0 = y + yo;
    FP xmax = GFX_MIN(xo, (FP)(cx - g->clipX1));
    xo = (cx - g->clipX2 > 1) ? (cx - g->clipX2 - FP::HALF) : FP::HALF;
    for(; xo <= xmax; ++xo) {
        yo = sqrt(aa - square(xo));
        drawVLineAA(g, color, yo0, y + yo, cx - xo);
    }
}

void Rgb565Gfx::fillRoundRect(uint32_t color, int32_t x, int32_t y, int32_t w, int32_t h, int32_t r1, int32_t r2, int32_t r3, int32_t r4) {
    if(!isVisible(this, x, y, w, h)) return;
    radiusAdjustment(w, h, r1, r2, r3, r4);

    uint8_t alpha = color >> 27;
    int32_t x1, x2;
    int32_t y1 = GFX_MAX(clipY1 - y, 0);
    int32_t y2 = GFX_MIN(clipY2 - y, h - 1);

    for(int32_t i = y1; i <= y2; i++) {
        x1 = x + (i < r1 ? r1 : (i > (h - r4) ? r4 : 0));
        x2 = x + w - 1 - (i < r2 ? r2 : (i > (h - r3) ? r3 : 0));
        ((Rgb565GfxHelper *)this)->blendHLine(alpha, color, x1, x2, i + y);
    }

    if(r1 > 0) fillQuarterCircle1(this, color, x, y, r1);
    if(r2 > 0) fillQuarterCircle2(this, color, x + w - r2, y, r2);
    if(r3 > 0) fillQuarterCircle3(this, color, x + w - r3, y + h - r3, r3);
    if(r4 > 0) fillQuarterCircle4(this, color, x, y + h - r4, r4);
}

static void drawImageRGB565(Rgb565Gfx *g, Image *img, int32_t x, int32_t y) {
    int32_t cx1 = GFX_MAX(GFX_MAX(x, (int32_t)g->clipX1), 0);
    int32_t cy1 = GFX_MAX(GFX_MAX(y, (int32_t)g->clipY1), 0);
    int32_t cx2 = GFX_MIN(GFX_MIN(x + (int32_t)img->width,  (int32_t)g->clipX2 + 1), (int32_t)g->width);
    int32_t cy2 = GFX_MIN(GFX_MIN(y + (int32_t)img->height, (int32_t)g->clipY2 + 1), (int32_t)g->height);
    if(cx1 >= cx2 || cy1 >= cy2) return;
    int32_t w = cx2 - cx1;
    for(int32_t dy = cy1; dy < cy2; dy++) {
        uint16_t *src = &((uint16_t *)img->data)[(dy - y) * (int32_t)img->width + (cx1 - x)];
        uint16_t *des = &((uint16_t *)g->data)[dy * (int32_t)g->width + cx1];
        memcpy(des, src, (size_t)w * 2);
    }
}

static void drawImageARGB565(Rgb565Gfx *g, Image *img, int32_t x, int32_t y) {
    uint8_t *alpha = &((uint8_t *)img->data)[(uint32_t)img->width * img->height * 2];
    int32_t cx1 = GFX_MAX(GFX_MAX(x, (int32_t)g->clipX1), 0);
    int32_t cy1 = GFX_MAX(GFX_MAX(y, (int32_t)g->clipY1), 0);
    int32_t cx2 = GFX_MIN(GFX_MIN(x + (int32_t)img->width,  (int32_t)g->clipX2 + 1), (int32_t)g->width);
    int32_t cy2 = GFX_MIN(GFX_MIN(y + (int32_t)img->height, (int32_t)g->clipY2 + 1), (int32_t)g->height);
    if(cx1 >= cx2 || cy1 >= cy2) return;
    for(int32_t dy = cy1; dy < cy2; dy++) {
        int32_t sy = dy - y;
        uint16_t *src = &((uint16_t *)img->data)[sy * (int32_t)img->width];
        uint16_t *des = &((uint16_t *)g->data)[dy * (int32_t)g->width];
        for(int32_t dx = cx1; dx < cx2; dx++) {
            int32_t sx = dx - x;
            uint32_t aIdx = (uint32_t)sy * img->width + sx;
            uint8_t a = (aIdx & 1) ? (alpha[aIdx >> 1] >> 4) : (alpha[aIdx >> 1] & 0x0F);
            uint32_t bg = __builtin_bswap16(des[dx]);
            uint32_t fg = __builtin_bswap16(src[sx]);
            bg = (bg | (bg << 16)) & 0x07E0F81F;
            fg = (fg | (fg << 16)) & 0x07E0F81F;
            bg += (((fg) - bg) * a) >> 4;
            bg &= 0x07E0F81F;
            bg = (bg | (bg >> 16));
            des[dx] = __builtin_bswap16(bg);
        }
    }
}

static void drawImageARGB888(Rgb565Gfx *g, Image *img, int32_t x, int32_t y) {
    // TODO
}

void Rgb565Gfx::drawImage(Image *img, int32_t x, int32_t y) {
    switch(img->format) {
        case IMG_RGB565: return drawImageRGB565(this, img, x, y);
        case IMG_ARGB565: return drawImageARGB565(this, img, x, y);
        default: return drawImageARGB888(this, img, x, y);
    }
}

static void drawImageRGB565(Rgb565Gfx *g, Image *img, int32_t x, int32_t y, uint16_t w, uint16_t h) {
    if(w == img->width && h == img->height) return drawImageRGB565(g, img, x, y);
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

static void drawImageARGB565(Rgb565Gfx *g, Image *img, int32_t x, int32_t y, uint16_t w, uint16_t h) {
    if(w == img->width && h == img->height) return drawImageARGB565(g, img, x, y);
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

static void drawImageARGB888(Rgb565Gfx *g, Image *img, int32_t x, int32_t y, uint16_t w, uint16_t h) {
    if(w == img->width && h == img->height) return drawImageARGB888(g, img, x, y);
    // TODO
}

void Rgb565Gfx::drawImage(Image *img, int32_t x, int32_t y, uint16_t w, uint16_t h) {
    switch(img->format) {
        case IMG_RGB565: return drawImageRGB565(this, img, x, y, w, h);
        case IMG_ARGB565: return drawImageARGB565(this, img, x, y, w, h);
        default: return drawImageARGB888(this, img, x, y, w, h);
    }
}

static void drawChar(Rgb565GfxHelper *g, const CharInfo *c, uint32_t color, int32_t x, int32_t y) {
    int32_t cx0 = GFX_MAX(g->clipX1 - x, 0);
    int32_t cxw = GFX_MIN(g->clipX2 - x + 1, (int32_t)c->getWidth());
    if(cx0 >= cxw) return;

    uint8_t height = c->getHeight();
    int8_t yOff = c->getYOffset();
    uint8_t alpha = color >> 27;

    if(alpha == 0x1F) {
        for(int32_t cy = 0; cy < height; cy++) {
            int32_t py = y + cy + yOff;
            if(!(g->clipY1 <= py && py <= g->clipY2)) continue;
            uint16_t *p = &((uint16_t *)g->data)[py * g->width + x];
            for(int32_t cx = cx0; cx < cxw; cx++) {
                int32_t px = x + cx;
                if(g->clipX1 <= px && px <= g->clipX2)
                    if(c->getPixel(cx, cy)) p[cx] = color;
            }
        }
    }
    else {
        for(int32_t cy = 0; cy < height; cy++) {
            int32_t py = y + cy + yOff;
            if(!(g->clipY1 <= py && py <= g->clipY2)) continue;
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
            drawChar((Rgb565GfxHelper *)this, c, color, x, y);
            x += c->getWidth() + space;
        }
        else {
            fillRect(color, x, y, stdWidth, stdHeight);
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
            drawChar((Rgb565GfxHelper *)this, c, color, x, y);
            x += c->getWidth() + space;
        }
        else {
            fillRect(color, x, y, stdWidth, stdHeight);
            x += stdWidth + space;
        }
        str += 2;
    }
}
