
#include <string.h>
#include "flint.h"
#include "flint_java_object.h"
#include "flint_const_name_base.h"
#include "flint_native_graphics_class.h"
#include "flint_throw_support.h"

#define COLOR_MODE_RGB444       0
#define COLOR_MODE_RGB555       1
#define COLOR_MODE_RGB565       2
#define COLOR_MODE_BGR565       3
#define COLOR_MODE_RGB888       4

static const uint32_t colorMask[] = {0x00F00F0F, 0x03E07C1F, 0x07E0F81F, 0x07E0F81F};

#define IS_IN_CLIP(_x, _y)                  \
    ((                                      \
        ((_x) >= clipX) &&                  \
        ((_y) >= clipY) &&                  \
        ((_x) < (clipX + clipWidth)) &&     \
        ((_y) < (clipY + clipHeight))       \
    ) ? true : false)

#define IS_IN_CLIP_X(_x)                    \
    ((                                      \
        ((_x) >= clipX) &&                  \
        ((_x) < (clipX + clipWidth))        \
    ) ? true : false)

#define IS_IN_CLIP_Y(_y)                    \
    ((                                      \
        ((_y) >= clipY) &&                  \
        ((_y) < (clipY + clipHeight))       \
    ) ? true : false)

class FlintGraphics {
private:
    uint8_t colorMode;
    int16_t originX;
    int16_t originY;
    uint16_t gwidth;
    int16_t clipX;
    int16_t clipY;
    int16_t clipWidth;
    int16_t clipHeight;
    union {
        uint16_t rgb16;
        uint32_t rgb32;
        uint8_t rgb[4];
    };
    uint8_t *colorBuffer;
    void (FlintGraphics::*drawPixel)(int32_t x, int32_t y);
    void (FlintGraphics::*drawFastHLine)(int32_t y1, int32_t y2, int32_t x);
    void (FlintGraphics::*drawFastVLine)(int32_t y1, int32_t y2, int32_t x);
    void (FlintGraphics::*fillFastRect)(int32_t x1, int32_t x2, int32_t y1, int32_t y2);

    void drawPixel1(int32_t x, int32_t y) {
        if(IS_IN_CLIP(x, y))
            ((uint16_t *)colorBuffer)[y * gwidth + x] = rgb16;
    }

    void drawPixel2(int32_t x, int32_t y) {
        if(IS_IN_CLIP(x, y)) {
            uint8_t bitCount = (colorMode == COLOR_MODE_RGB444) ? 4 : 5;
            uint32_t alpha = rgb[3];
            uint32_t mask = colorMask[colorMode];
            uint32_t fg = (rgb[0] << 8) | rgb[1];
            fg = (fg | (fg << 16)) & mask;
            uint8_t *buff = (uint8_t *)&colorBuffer[(y * gwidth + x) * 2];
            uint32_t bg = ((buff[0] << 8) | buff[1]);
            bg = (bg | (bg << 16)) & mask;
            bg += (fg - bg) * alpha >> bitCount;
            bg &= mask;
            bg = (bg | bg >> 16);
            buff[0] = bg >> 8;
            buff[1] = bg;
        }
    }

    void drawPixel3(int32_t x, int32_t y) {
        if(IS_IN_CLIP(x, y)) {
            uint8_t *buff = (uint8_t *)&colorBuffer[(y * gwidth + x) * 3];
            buff[0] = rgb[0];
            buff[1] = rgb[1];
            buff[2] = rgb[2];
        }
    }

    void drawPixel4(int32_t x, int32_t y) {
        if(IS_IN_CLIP(x, y)) {
            uint8_t alpha = rgb[3];
            uint8_t *buff = (uint8_t *)&colorBuffer[(y * gwidth + x) * 3];
            buff[0] += (rgb[0] - buff[0]) * alpha >> 8;
            buff[1] += (rgb[1] - buff[1]) * alpha >> 8;
            buff[2] += (rgb[2] - buff[2]) * alpha >> 8;
        }
    }

    void drawFastHLine1(int32_t x1, int32_t x2, int32_t y) {
        uint16_t *buff = &((uint16_t *)colorBuffer)[y * gwidth];
        for(int32_t x = x1; x <= x2; x++)
            buff[x] = rgb16;
    }

    void drawFastHLine2(int32_t x1, int32_t x2, int32_t y) {
        uint8_t bitCount = (colorMode == COLOR_MODE_RGB444) ? 4 : 5;
        uint32_t alpha = rgb[3];
        uint32_t mask = colorMask[colorMode];
        uint32_t fg = (rgb[0] << 8) | rgb[1];
        fg = (fg | (fg << 16)) & mask;
        x1 *= 2;
        x2 *= 2;
        uint8_t *buff = (uint8_t *)&colorBuffer[y * gwidth * 2];
        for(int32_t x = x1; x <= x2; x += 2) {
            uint32_t bg = ((buff[x] << 8) | buff[x + 1]);
            bg = (bg | (bg << 16)) & mask;
            bg += (fg - bg) * alpha >> bitCount;
            bg &= mask;
            bg = (bg | bg >> 16);
            buff[x] = bg >> 8;
            buff[x + 1] = bg;
        }
    }

    void drawFastHLine3(int32_t x1, int32_t x2, int32_t y) {
        x1 *= 3;
        x2 *= 3;
        uint8_t *buff = (uint8_t *)&colorBuffer[y * gwidth * 3];
        for(int32_t x = x1; x <= x2; x += 3) {
            buff[x + 0] = rgb[0];
            buff[x + 1] = rgb[1];
            buff[x + 2] = rgb[2];
        }
    }

    void drawFastHLine4(int32_t x1, int32_t x2, int32_t y) {
        x1 *= 3;
        x2 *= 3;
        uint8_t alpha = rgb[3];
        uint8_t *buff = (uint8_t *)&colorBuffer[y * gwidth * 3];
        for(int32_t x = x1; x <= x2; x += 3) {
            buff[x + 0] += (rgb[0] - buff[x + 0]) * alpha >> 8;
            buff[x + 1] += (rgb[1] - buff[x + 1]) * alpha >> 8;
            buff[x + 2] += (rgb[2] - buff[x + 2]) * alpha >> 8;
        }
    }

    void drawFastVLine1(int32_t y1, int32_t y2, int32_t x) {
        for(; y1 <= y2; y1++)
            (&((uint16_t *)colorBuffer)[y1 * gwidth])[x] = rgb16;
    }

    void drawFastVLine2(int32_t y1, int32_t y2, int32_t x) {
        uint8_t bitCount = (colorMode == COLOR_MODE_RGB444) ? 4 : 5;
        uint32_t alpha = rgb[3];
        uint32_t mask = colorMask[colorMode];
        uint32_t fg = (rgb[0] << 8) | rgb[1];
        fg = (fg | (fg << 16)) & mask;
        x *= 2;
        for(; y1 <= y2; y1++) {
            uint8_t *buff = (uint8_t *)&colorBuffer[y1 * gwidth * 2];
            uint32_t bg = ((buff[x] << 8) | buff[x + 1]);
            bg = (bg | (bg << 16)) & mask;
            bg += (fg - bg) * alpha >> bitCount;
            bg &= mask;
            bg = (bg | bg >> 16);
            buff[x] = bg >> 8;
            buff[x + 1] = bg;
        }
    }

    void drawFastVLine3(int32_t y1, int32_t y2, int32_t x) {
        x *= 3;
        for(; y1 <= y2; y1++) {
            uint8_t *buff = (uint8_t *)&colorBuffer[y1 * gwidth * 3];
            buff[x + 0] = rgb[0];
            buff[x + 1] = rgb[1];
            buff[x + 2] = rgb[2];
        }
    }

    void drawFastVLine4(int32_t y1, int32_t y2, int32_t x) {
        uint32_t alpha = rgb[3];
        for(; y1 <= y2; y1++) {
            uint8_t *buff = (uint8_t *)&colorBuffer[y1 * gwidth * 3];
            buff[x + 0] += (rgb[0] - buff[x + 0]) * alpha >> 8;
            buff[x + 1] += (rgb[1] - buff[x + 1]) * alpha >> 8;
            buff[x + 2] += (rgb[2] - buff[x + 2]) * alpha >> 8;
        }
    }

    void fillFastRect1(int32_t x1, int32_t x2, int32_t y1, int32_t y2) {
        for(; y1 <= y2; y1++) {
            uint16_t *buff = &((uint16_t *)colorBuffer)[y1 * gwidth];
            for(int32_t x = x1; x <= x2; x++)
                buff[x] = rgb16;
        }
    }

    void fillFastRect2(int32_t x1, int32_t x2, int32_t y1, int32_t y2) {
        uint8_t bitCount = (colorMode == COLOR_MODE_RGB444) ? 4 : 5;
        uint32_t alpha = rgb[3];
        uint32_t mask = colorMask[colorMode];
        uint32_t fg = (rgb[0] << 8) | rgb[1];
        fg = (fg | (fg << 16)) & mask;
        x1 *= 2;
        x2 *= 2;
        for(; y1 <= y2; y1++) {
            uint8_t *buff = (uint8_t *)&colorBuffer[y1 * gwidth * 2];
            for(int32_t x = x1; x <= x2; x += 2) {
                uint32_t bg = ((buff[x] << 8) | buff[x + 1]);
                bg = (bg | (bg << 16)) & mask;
                bg += (fg - bg) * alpha >> bitCount;
                bg &= mask;
                bg = (bg | bg >> 16);
                buff[x] = bg >> 8;
                buff[x + 1] = bg;
            }
        }
    }

    void fillFastRect3(int32_t x1, int32_t x2, int32_t y1, int32_t y2) {
        x1 *= 3;
        x2 *= 3;
        for(; y1 <= y2; y1++) {
            uint8_t *buff = (uint8_t *)&colorBuffer[y1 * gwidth * 3];
            for(int32_t x = x1; x <= x2; x += 3) {
                buff[x + 0] = rgb[0];
                buff[x + 1] = rgb[1];
                buff[x + 2] = rgb[2];
            }
        }
    }

    void fillFastRect4(int32_t x1, int32_t x2, int32_t y1, int32_t y2) {
        x1 *= 3;
        x2 *= 3;
        uint32_t alpha = rgb[3];
        for(; y1 <= y2; y1++) {
            uint8_t *buff = (uint8_t *)&colorBuffer[y1 * gwidth * 3];
            for(int32_t x = x1; x <= x2; x += 3) {
                buff[x + 0] += (rgb[0] - buff[x + 0]) * alpha >> 8;
                buff[x + 1] += (rgb[1] - buff[x + 1]) * alpha >> 8;
                buff[x + 2] += (rgb[2] - buff[x + 2]) * alpha >> 8;
            }
        }
    }

    void drawCircleQuadrant(int32_t xc, int32_t yc, int32_t r, int32_t q) {
        int32_t x = 0, y = r;
        int32_t d = 3 - 2 * r;
        while(x <= y) {
            if(q & 0x01) {
                (this->*drawPixel)(xc - x, yc - y);
                (this->*drawPixel)(xc - y, yc - x);
            }
            if(q & 0x02) {
                (this->*drawPixel)(xc + x, yc - y);
                (this->*drawPixel)(xc + y, yc - x);
            }
            if(q & 0x04) {
                (this->*drawPixel)(xc + x, yc + y);
                (this->*drawPixel)(xc + y, yc + x);
            }
            if(q & 0x08) {
                (this->*drawPixel)(xc - x, yc + y);
                (this->*drawPixel)(xc - y, yc + x);
            }
            if(d > 0) {
                y--;
                d += 4 * (x - y) + 10;
            }
            else
                d += 4 * x + 6;
            x++;
        }
    }

    void fillCircleQuadrant(int32_t xc, int32_t yc, int32_t r, int32_t q) {
        int32_t x = 0, y = r;
        int32_t d = 3 - 2 * r;
        int32_t xc1 = FLINT_MAX(xc, clipX);
        int32_t xc2 = FLINT_MIN(xc, clipX + clipWidth - 1);
        while (x <= y) {
            if((q & 0x01) && IS_IN_CLIP_Y(yc - x))
                (this->*drawFastHLine)(FLINT_MAX(clipX, xc - y), xc2, yc - x);
            if((q & 0x02) && IS_IN_CLIP_Y(yc - x))
                (this->*drawFastHLine)(xc1, FLINT_MIN(xc + y, clipX + clipWidth - 1), yc - x);
            if((q & 0x04) && IS_IN_CLIP_Y(yc + x))
                (this->*drawFastHLine)(xc1, FLINT_MIN(xc + y, clipX + clipWidth - 1), yc + x);
            if((q & 0x08) && IS_IN_CLIP_Y(yc + x))
                (this->*drawFastHLine)(FLINT_MAX(clipX, xc - y), xc2, yc + x);
            if(d > 0) {
                if(x != y) {
                    if((q & 0x01) && IS_IN_CLIP_Y(yc - y))
                        (this->*drawFastHLine)(FLINT_MAX(clipX, xc - x), xc2, yc - y);
                    if((q & 0x02) && IS_IN_CLIP_Y(yc - y))
                        (this->*drawFastHLine)(xc1, FLINT_MIN(xc + x, clipX + clipWidth - 1), yc - y);
                    if((q & 0x04) && IS_IN_CLIP_Y(yc + y))
                        (this->*drawFastHLine)(xc1, FLINT_MIN(xc + x, clipX + clipWidth - 1), yc + y);
                    if((q & 0x08) && IS_IN_CLIP_Y(yc + y))
                        (this->*drawFastHLine)(FLINT_MAX(clipX, xc - x), xc2, yc + y);
                }
                y--;
                d = d + 4 * (x - y) + 10;
            }
            else
                d = d + 4 * x + 6;
            x++;
        }
    }
public:
    FlintGraphics(FlintJavaObject *g, uint32_t color) {
        uint8_t alpha = color >> 24;
        FlintFieldsData &fields = g->getFields();
        FlintInt8Array *buffer = (FlintInt8Array *)fields.getFieldObjectByIndex(0)->object;
        colorMode = (uint8_t)fields.getFieldData32ByIndex(0)->value;
        colorBuffer = (uint8_t *)buffer->getData();
        originX = fields.getFieldData32ByIndex(1)->value;
        originY = fields.getFieldData32ByIndex(2)->value;
        gwidth = fields.getFieldData32ByIndex(3)->value;
        clipX = fields.getFieldData32ByIndex(4)->value - originX;
        clipY = fields.getFieldData32ByIndex(5)->value - originY;
        clipWidth = fields.getFieldData32ByIndex(6)->value;
        clipHeight = fields.getFieldData32ByIndex(7)->value;

        switch(colorMode) {
            case COLOR_MODE_RGB444:
                color = ((color >> 4) & 0x0F) | ((color >> 8) & 0xF0) | ((color >> 12) & 0x0F00);
                rgb[0] = (uint8_t)(color >> 8);
                rgb[1] = (uint8_t)color;
                rgb[3] = (alpha + 8) >> 4;
                break;
            case COLOR_MODE_RGB555:
                color = ((color >> 3) & 0x1F) | ((color >> 6) & 0x03E0) | ((color >> 9) & 0x7C00);
                rgb[0] = (uint8_t)(color >> 8);
                rgb[1] = (uint8_t)color;
                rgb[3] = (alpha + 4) >> 3;
                break;
            case COLOR_MODE_RGB565:
                color = ((color >> 3) & 0x1F) | ((color >> 5) & 0x07E0) | ((color >> 8) & 0xF800);
                rgb[0] = (uint8_t)(color >> 8);
                rgb[1] = (uint8_t)color;
                rgb[3] = (alpha + 4) >> 3;
                break;
            case COLOR_MODE_BGR565:
                color = ((color << 8) & 0xF800) | ((color >> 5) & 0x07E0) | ((color >> 19) & 0x1F);
                rgb[0] = (uint8_t)(color >> 8);
                rgb[1] = (uint8_t)color;
                rgb[3] = (alpha + 4) >> 3;
                break;
            default:
                rgb[0] = (uint8_t)(color >> 16);
                rgb[1] = (uint8_t)(color >> 8);
                rgb[2] = (uint8_t)color;
                rgb[3] = alpha;
                break;
        }

        if((color >> 24) == 0xFF) {
            if(colorMode != COLOR_MODE_RGB888) {
                drawPixel = &FlintGraphics::drawPixel1;
                drawFastHLine = &FlintGraphics::drawFastHLine1;
                drawFastVLine = &FlintGraphics::drawFastVLine1;
                fillFastRect = &FlintGraphics::fillFastRect1;
            }
            else {
                drawPixel = &FlintGraphics::drawPixel3;
                drawFastHLine = &FlintGraphics::drawFastHLine3;
                drawFastVLine = &FlintGraphics::drawFastVLine3;
                fillFastRect = &FlintGraphics::fillFastRect3;
            }
        }
        else {
            if(colorMode != COLOR_MODE_RGB888) {
                drawPixel = &FlintGraphics::drawPixel2;
                drawFastHLine = &FlintGraphics::drawFastHLine2;
                drawFastVLine = &FlintGraphics::drawFastVLine2;
                fillFastRect = &FlintGraphics::fillFastRect2;
            }
            else {
                drawPixel = &FlintGraphics::drawPixel4;
                drawFastHLine = &FlintGraphics::drawFastHLine4;
                drawFastVLine = &FlintGraphics::drawFastVLine4;
                fillFastRect = &FlintGraphics::fillFastRect4;
            }
        }
    }

    void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
        x1 -= originX;
        x2 -= originX;
        y1 -= originY;
        y2 -= originY;

        if(x1 == x2) {
            if(!IS_IN_CLIP_X(x1))
                return;
            if(y1 > y2)
                FLINT_SWAP(y1, y2);
            (this->*drawFastVLine)(FLINT_MAX(y1, clipY), FLINT_MIN(y2, clipY + clipHeight - 1), x1);
        }
        else if(y1 == y2) {
            if(!IS_IN_CLIP_Y(y1))
                return;
            if(x1 > x2)
                FLINT_SWAP(x1, x2);
            (this->*drawFastHLine)(FLINT_MAX(x1, clipX), FLINT_MIN(x2, clipX + clipWidth - 1), y1);
        }
        else {
            int32_t dx = FLINT_ABS(x2 - x1), sx = x1 < x2 ? 1 : -1;
            int32_t dy = -FLINT_ABS(y2 - y1), sy = y1 < y2 ? 1 : -1;
            int32_t err = dx + dy;
            while(1) {
                (this->*drawPixel)(x1, y1);
                if(x1 == x2 && y1 == y2)
                    break;
                int32_t e2 = 2 * err;
                if(e2 >= dy) {
                    err += dy;
                    x1 += sx;
                }
                if(e2 <= dx) {
                    err += dx;
                    y1 += sy;
                }
            }
        }
    }

    void drawRect(int32_t x, int32_t y, int32_t width, int32_t height) {
        x -= originX;
        y -= originY;

        int32_t x1 = FLINT_MAX(x, clipX);
        int32_t x2 = FLINT_MIN(x + width, clipX + clipWidth) - 1;
        int32_t y1 = FLINT_MAX(y, clipY);
        int32_t y2 = FLINT_MIN(y + height, clipY + clipHeight) - 1;

        if(y1 <= y2) {
            if((x1 <= x) && (x <= x2))
                (this->*drawFastVLine)(y1, y2, x1);
            if((x2 >= clipX) && x2 == (x + width - 1))
                (this->*drawFastVLine)(y1, y2, x2);
        }
        if(x1 <= x2) {
            if((y1 <= y) && (y <= y2))
                (this->*drawFastHLine)(x1, x2, y1);
            if((y2 >= clipY) && y2 == (y + height - 1))
                (this->*drawFastHLine)(x1, x2, y2);
        }
    }

    void fillRect(int32_t x, int32_t y, int32_t width, int32_t height) {
        x -= originX;
        y -= originY;

        int32_t x1 = FLINT_MAX(x, clipX);
        int32_t x2 = FLINT_MIN(x + width, clipX + clipWidth) - 1;
        if(x1 > x2)
            return;
        int32_t y1 = FLINT_MAX(y, clipY);
        int32_t y2 = FLINT_MIN(y + height, clipY + clipHeight) - 1;
        if(y1 > y2)
            return;

        (this->*fillFastRect)(x1, x2, y1, y2);
    }

    void drawRoundRect(int32_t x, int32_t y, int32_t width, int32_t height, int32_t r1, int32_t r2, int32_t r3, int32_t r4) {
        x -= originX;
        y -= originY;

        r1 = (r1 < 0) ? 0 : FLINT_MIN(r1, width / 2);
        r2 = (r2 < 0) ? 0 : FLINT_MIN(r2, width / 2);
        r3 = (r3 < 0) ? 0 : FLINT_MIN(r3, width / 2);
        r4 = (r4 < 0) ? 0 : FLINT_MIN(r4, width / 2);

        int32_t x1 = FLINT_MAX(x, clipX);
        int32_t x2 = FLINT_MIN(x + width, clipX + clipWidth) - 1;
        int32_t y1 = FLINT_MAX(y, clipY);
        int32_t y2 = FLINT_MIN(y + height, clipY + clipHeight) - 1;

        if(y1 <= y2) {
            if((x1 <= x) && (x <= x2))
                (this->*drawFastVLine)(FLINT_MAX(y1, y + r1), FLINT_MIN(y2, y + height - r4), x1);
            if((x2 >= clipX) && x2 == (x + width - 1))
                (this->*drawFastVLine)(FLINT_MAX(y1, y + r2), FLINT_MIN(y2, y + height - r3), x2);
        }
        if(x1 <= x2) {
            if((y1 <= y) && (y <= y2))
                (this->*drawFastHLine)(FLINT_MAX(x1, x + r1), FLINT_MIN(x2, x + width - r2), y1);
            if((y2 >= clipY) && y2 == (y + height - 1))
                (this->*drawFastHLine)(FLINT_MAX(x1, x + r4), FLINT_MIN(x2, x + width - r3), y2);
        }

        if(r1) drawCircleQuadrant(x + r1, y + r1, r1, 0x01);
        if(r2) drawCircleQuadrant(x + width - 1 - r2, y + r2, r2, 0x02);
        if(r3) drawCircleQuadrant(x + width - 1 - r3, y + height - 1 - r3, r3, 0x04);
        if(r4) drawCircleQuadrant(x + r4, y + height - 1 - r4, r4, 0x08);
    }

    void fillRoundRect(int32_t x, int32_t y, int32_t width, int32_t height, int32_t r1, int32_t r2, int32_t r3, int32_t r4) {
        r1 = FLINT_MIN(r1, (width - 1) / 2);
        r2 = FLINT_MIN(r2, (width - 1) / 2);
        r3 = FLINT_MIN(r3, (width - 1) / 2);
        r4 = FLINT_MIN(r4, (width - 1) / 2);

        if(r1 < 0) r1 = 0;
        if(r2 < 0) r2 = 0;
        if(r3 < 0) r3 = 0;
        if(r4 < 0) r4 = 0;

        int32_t r14 = FLINT_MAX(r1, r4);
        int32_t r23 = FLINT_MAX(r2, r3);
        
        int32_t x2 = x;
        int32_t w2 = width;
        if(r14) {
            x2 += r14 + 1;
            w2 -= r14 + 1;
        }
        if(r23)
            w2 -= r23 + 1;

        fillRect(x2, y, w2, height);
        fillRect(x, y + r1 + !!r1, r14 + !!r14, height - r1 - r4 - !!r1 - !!r4);
        fillRect(x + width - r23 - !!r23, y + r2 + !!r2, r23 + 1, height - r2 - r3 - !!r2 - !!r3);

        if(r1 != r4) {
            if(r1 < r4)
                fillRect(x + r1 + !!r1, y, r4 - r1, r1 + 1);
            else
                fillRect(x + r4 + !!r4, y + height - r4 - 1, r14 - r4, r4 + 1);
        }
        if(r2 != r3) {
            if(r2 < r3)
                fillRect(x + width - r3 - 1, y, r3 - r2, r2 + 1);
            else
                fillRect(x + width - r2 - 1, y + height - r3 - 1, r23 - r3, r3 + 1);
        }

        x -= originX;
        y -= originY;

        if(r1) fillCircleQuadrant(x + r1, y + r1, r1, 0x01);
        if(r2) fillCircleQuadrant(x + width - 1 - r2, y + r2, r2, 0x02);
        if(r3) fillCircleQuadrant(x + width - 1 - r3, y + height - 1 - r3, r3, 0x04);
        if(r4) fillCircleQuadrant(x + r4, y + height - 1 - r4, r4, 0x08);
    }

    void drawEllipse(int32_t x, int32_t y, int32_t width, int32_t height) {
        x -= originX;
        y -= originY;
        int32_t x2 = x + width - 1;
        int32_t y2 = y + height - 1;
        int32_t a = FLINT_ABS(x2 - x), b = FLINT_ABS(y2 - y), b1 = b & 1;
        int64_t dx = (int64_t)4 * (1 - a) * b * b, dy = 4 * (b1 + 1) * a * a;
        int64_t err = (int64_t)dx + dy + b1 * a * a, e2;

        if(x > x2) {
            x = x2;
            x2 += a;
        }
        if(y > y2)
            y = y2;
        y += (b + 1) / 2;
        y2 = y - b1;
        a *= 8 * a;
        b1 = 8 * b * b;

        do {
            (this->*drawPixel)(x2, y);
            (this->*drawPixel)(x, y);
            (this->*drawPixel)(x, y2);
            (this->*drawPixel)(x2, y2);
            e2 = 2 * err;
            if(e2 <= dy) {
                y++;
                y2--;
                err += dy += a;
            }
            if(e2 >= dx || 2 * err > dy) {
                x++;
                x2--;
                err += dx += b1;
            }
        } while(x <= x2);

        while(y - y2 < b) {
            (this->*drawPixel)(x - 1, y);
            (this->*drawPixel)(x2 + 1, y++);
            (this->*drawPixel)(x - 1, y2);
            (this->*drawPixel)(x2 + 1, y2--);
        }
    }

    void fillEllipse(int32_t x, int32_t y, int32_t width, int32_t height) {
        x -= originX;
        y -= originY;
        int32_t x2 = x + width - 1;
        int32_t y2 = y + height - 1;
        int32_t a = FLINT_ABS(x2 - x), b = FLINT_ABS(y2 - y), b1 = b & 1;
        int64_t dx = (int64_t)4 * (1 - a) * b * b, dy = 4 * (b1 + 1) * a * a;
        int64_t err = (int64_t)dx + dy + b1 * a * a, e2;

        if(x > x2) {
            x = x2;
            x2 += a;
        }
        if(y > y2)
            y = y2;
        y += (b + 1) / 2;
        y2 = y - b1;
        a *= 8 * a;
        b1 = 8 * b * b;

        do {
            e2 = 2 * err;
            if(e2 <= dy) {
                int32_t tmp1 = FLINT_MAX(x, clipX);
                int32_t tmp2 = FLINT_MIN(x2, clipX + clipWidth - 1);
                if(IS_IN_CLIP_Y(y))
                    (this->*drawFastHLine)(tmp1, tmp2, y);
                if(IS_IN_CLIP_Y(y2))
                    (this->*drawFastHLine)(tmp1, tmp2, y2);
                y++;
                y2--;
                err += dy += a;
            }
            if(e2 >= dx || 2 * err > dy) {
                x++;
                x2--;
                err += dx += b1;
            }
        } while(x <= x2);

        while(y - y2 < b) {
            int32_t tmp1 = FLINT_MAX(x - 1, clipX);
            int32_t tmp2 = FLINT_MIN(x2 + 1, clipX + clipWidth - 1);
            if(IS_IN_CLIP_Y(y))
                (this->*drawFastHLine)(tmp1, tmp2, y++);
            if(IS_IN_CLIP_Y(y2))
                (this->*drawFastHLine)(tmp1, tmp2, y2--);
        }
    }

    void drawPolyline(int32_t *xPoints, int32_t *yPoints, int32_t nPoints) {
        nPoints--;
        for(int32_t i = 0; i < nPoints; i++)
            drawLine(xPoints[i], yPoints[i], xPoints[i + 1], yPoints[i + 1]);
    }

    void drawPolygon(int32_t *xPoints, int32_t *yPoints, int32_t nPoints) {
        if(nPoints > 0) {
            drawPolyline(xPoints, yPoints, nPoints);
            drawLine(xPoints[0], yPoints[0], xPoints[nPoints - 1], yPoints[nPoints - 1]);
        }
    }
};

class FlintJavaColor : public FlintJavaObject {
public:
    uint32_t getValue(void) const {
        return getFields().getFieldData32ByIndex(0)->value;
    }
};

static FlintError nativeClear(FlintExecution &execution) {
    FlintJavaObject *g = execution.stackPopObject();
    FlintInt8Array *colorBuffer = (FlintInt8Array *)g->getFields().getFieldObjectByIndex(0)->object;
    memset(colorBuffer->getData(), 0, colorBuffer->getLength());
    return ERR_OK;
}

static FlintError nativeDrawLine(FlintExecution &execution) {
    int32_t y2 = execution.stackPopInt32();
    int32_t x2 = execution.stackPopInt32();
    int32_t y1 = execution.stackPopInt32();
    int32_t x1 = execution.stackPopInt32();
    FlintJavaColor *colorObj = (FlintJavaColor *)execution.stackPopObject();
    if(!colorObj)
        return throwNullPointerException(execution);
    uint32_t color = colorObj->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    g.drawLine(x1, y1, x2, y2);
    return ERR_OK;
}

static FlintError nativeDrawRect(FlintExecution &execution) {
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    FlintJavaColor *colorObj = (FlintJavaColor *)execution.stackPopObject();
    if(!colorObj)
        return throwNullPointerException(execution);
    uint32_t color = colorObj->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    g.drawRect(x, y, width, height);
    return ERR_OK;
}

static FlintError nativeFillRect(FlintExecution &execution) {
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    FlintJavaColor *colorObj = (FlintJavaColor *)execution.stackPopObject();
    if(!colorObj)
        return throwNullPointerException(execution);
    uint32_t color = colorObj->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    g.fillRect(x, y, width, height);
    return ERR_OK;
}

static FlintError nativeDrawRoundRect(FlintExecution &execution) {
    int32_t r4 = execution.stackPopInt32();
    int32_t r3 = execution.stackPopInt32();
    int32_t r2 = execution.stackPopInt32();
    int32_t r1 = execution.stackPopInt32();
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    FlintJavaColor *colorObj = (FlintJavaColor *)execution.stackPopObject();
    if(!colorObj)
        return throwNullPointerException(execution);
    uint32_t color = colorObj->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    g.drawRoundRect(x, y, width, height, r1, r2, r3, r4);
    return ERR_OK;
}

static FlintError nativeFillRoundRect(FlintExecution &execution) {
    int32_t r4 = execution.stackPopInt32();
    int32_t r3 = execution.stackPopInt32();
    int32_t r2 = execution.stackPopInt32();
    int32_t r1 = execution.stackPopInt32();
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    FlintJavaColor *colorObj = (FlintJavaColor *)execution.stackPopObject();
    if(!colorObj)
        return throwNullPointerException(execution);
    uint32_t color = colorObj->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    g.fillRoundRect(x, y, width, height, r1, r2, r3, r4);
    return ERR_OK;
}

static FlintError nativeDrawEllipse(FlintExecution &execution) {
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    FlintJavaColor *colorObj = (FlintJavaColor *)execution.stackPopObject();
    if(!colorObj)
        return throwNullPointerException(execution);
    uint32_t color = colorObj->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    g.drawEllipse(x, y, width, height);
    return ERR_OK;
}

static FlintError nativeFillEllipse(FlintExecution &execution) {
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    FlintJavaColor *colorObj = (FlintJavaColor *)execution.stackPopObject();
    if(!colorObj)
        return throwNullPointerException(execution);
    uint32_t color = colorObj->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    g.fillEllipse(x, y, width, height);
    return ERR_OK;
}

static FlintError nativeDrawArc(FlintExecution &execution) {
    int32_t arcAngle = execution.stackPopInt32();
    int32_t startAngle = execution.stackPopInt32();
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    FlintJavaColor *colorObj = (FlintJavaColor *)execution.stackPopObject();
    if(!colorObj)
        return throwNullPointerException(execution);
    uint32_t color = colorObj->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    // TODO
    return ERR_OK;
}

static FlintError nativeFillArc(FlintExecution &execution) {
    int32_t arcAngle = execution.stackPopInt32();
    int32_t startAngle = execution.stackPopInt32();
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    FlintJavaColor *colorObj = (FlintJavaColor *)execution.stackPopObject();
    if(!colorObj)
        return throwNullPointerException(execution);
    uint32_t color = colorObj->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    // TODO
    return ERR_OK;
}

static FlintError nativeDrawPolyline(FlintExecution &execution) {
    int32_t nPoints = execution.stackPopInt32();
    FlintInt32Array *yPoints = (FlintInt32Array *)execution.stackPopObject();
    FlintInt32Array *xPoints = (FlintInt32Array *)execution.stackPopObject();
    FlintJavaColor *colorObj = (FlintJavaColor *)execution.stackPopObject();
    if(!yPoints || !xPoints || !colorObj)
        return throwNullPointerException(execution);
    uint32_t color = colorObj->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    if(nPoints > xPoints->getLength())
        return throwArrayIndexOutOfBoundsException(execution, nPoints, xPoints->getLength());
    if(nPoints > yPoints->getLength())
        return throwArrayIndexOutOfBoundsException(execution, nPoints, yPoints->getLength());
    g.drawPolyline(xPoints->getData(), yPoints->getData(), nPoints);
    return ERR_OK;
}

static FlintError nativeDrawPolygon(FlintExecution &execution) {
    int32_t nPoints = execution.stackPopInt32();
    FlintInt32Array *yPoints = (FlintInt32Array *)execution.stackPopObject();
    FlintInt32Array *xPoints = (FlintInt32Array *)execution.stackPopObject();
    FlintJavaColor *colorObj = (FlintJavaColor *)execution.stackPopObject();
    if(!yPoints || !xPoints || !colorObj)
        return throwNullPointerException(execution);
    uint32_t color = colorObj->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    if(nPoints > xPoints->getLength())
        return throwArrayIndexOutOfBoundsException(execution, nPoints, xPoints->getLength());
    if(nPoints > yPoints->getLength())
        return throwArrayIndexOutOfBoundsException(execution, nPoints, yPoints->getLength());
    g.drawPolygon(xPoints->getData(), yPoints->getData(), nPoints);
    return ERR_OK;
}

static FlintError nativeFillPolygon(FlintExecution &execution) {
    int32_t nPoints = execution.stackPopInt32();
    FlintInt32Array *yPoints = (FlintInt32Array *)execution.stackPopObject();
    FlintInt32Array *xPoints = (FlintInt32Array *)execution.stackPopObject();
    FlintJavaColor *colorObj = (FlintJavaColor *)execution.stackPopObject();
    if(!yPoints || !xPoints || !colorObj)
        return throwNullPointerException(execution);
    uint32_t color = colorObj->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    // TODO
    return ERR_OK;
}

static FlintError nativeDrawString(FlintExecution &execution) {
    FlintGraphics g(execution.stackPopObject(), 0);
    // TODO
    return ERR_OK;
}

static FlintError nativeDrawImage1(FlintExecution &execution) {
    FlintGraphics g(execution.stackPopObject(), 0);
    // TODO
    return ERR_OK;
}

static FlintError nativeDrawImage2(FlintExecution &execution) {
    FlintGraphics g(execution.stackPopObject(), 0);
    // TODO
    return ERR_OK;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x05\x00\xBB\x0D""clear",         "\x03\x00\x91\x99""()V",                                                              nativeClear),
    NATIVE_METHOD("\x08\x00\x3C\x95""drawLine",      "\x1C\x00\xFB\xB2""(Lflint/drawing/Color;IIII)V",                                     nativeDrawLine),
    NATIVE_METHOD("\x08\x00\x3E\x22""drawRect",      "\x1C\x00\xFB\xB2""(Lflint/drawing/Color;IIII)V",                                     nativeDrawRect),
    NATIVE_METHOD("\x08\x00\x71\xE5""fillRect",      "\x1C\x00\xFB\xB2""(Lflint/drawing/Color;IIII)V",                                     nativeFillRect),
    NATIVE_METHOD("\x0D\x00\x54\x7E""drawRoundRect", "\x20\x00\xAA\x9E""(Lflint/drawing/Color;IIIIIIII)V",                                 nativeDrawRoundRect),
    NATIVE_METHOD("\x0D\x00\x3C\xC4""fillRoundRect", "\x20\x00\xAA\x9E""(Lflint/drawing/Color;IIIIIIII)V",                                 nativeFillRoundRect),
    NATIVE_METHOD("\x0B\x00\x26\x66""drawEllipse",   "\x1C\x00\xFB\xB2""(Lflint/drawing/Color;IIII)V",                                     nativeDrawEllipse),
    NATIVE_METHOD("\x0B\x00\x45\x81""fillEllipse",   "\x1C\x00\xFB\xB2""(Lflint/drawing/Color;IIII)V",                                     nativeFillEllipse),
    NATIVE_METHOD("\x07\x00\x59\x0A""drawArc",       "\x1E\x00\x8D\x62""(Lflint/drawing/Color;IIIIII)V",                                   nativeDrawArc),
    NATIVE_METHOD("\x07\x00\x52\x04""fillArc",       "\x1E\x00\x8D\x62""(Lflint/drawing/Color;IIIIII)V",                                   nativeFillArc),
    NATIVE_METHOD("\x0C\x00\x75\xD9""drawPolyline",  "\x1D\x00\x99\x2E""(Lflint/drawing/Color;[I[II)V",                                    nativeDrawPolyline),
    NATIVE_METHOD("\x0B\x00\x9F\x57""drawPolygon",   "\x1D\x00\x99\x2E""(Lflint/drawing/Color;[I[II)V",                                    nativeDrawPolygon),
    NATIVE_METHOD("\x0B\x00\xFC\xB0""fillPolygon",   "\x1D\x00\x99\x2E""(Lflint/drawing/Color;[I[II)V",                                    nativeFillPolygon),
    NATIVE_METHOD("\x0A\x00\x6B\x54""drawString",    "\x40\x00\x21\x24""(Ljava/lang/String;Lflint/drawing/Font;Lflint/drawing/Color;II)V", nativeDrawString),
    NATIVE_METHOD("\x09\x00\xE9\xD6""drawImage",     "\x1A\x00\xC3\x33""(Lflint/drawing/Image;II)V",                                       nativeDrawImage1),
    NATIVE_METHOD("\x09\x00\xE9\xD6""drawImage",     "\x1C\x00\x5E\xC2""(Lflint/drawing/Image;IIII)V",                                     nativeDrawImage2),
};

const FlintNativeClass GRAPHICS_CLASS = NATIVE_CLASS(flintGraphicsClassName, methods);
