
#include <string.h>
#include "flint.h"
#include "flint_java_object.h"
#include "flint_const_name_base.h"
#include "flint_native_graphics_class.h"
#include "flint_throw_support.h"

#define COLOR_MODE_RGB555       0
#define COLOR_MODE_RGB565       1
#define COLOR_MODE_BGR565       2
#define COLOR_MODE_RGB888       3

static const uint32_t colorMask[] = {0x03E07C1F, 0x07E0F81F, 0x07E0F81F};

#define IS_IN_CLIP(_x, _y, _clipX, _clipY, _clipW, _clipH)  \
    ((                                                      \
        ((_x) >= (_clipX)) &&                               \
        ((_y) >= (_clipY)) &&                               \
        ((_x) < ((_clipX) + (_clipW))) &&                   \
        ((_y) < ((_clipY) + (_clipH)))                      \
    ) ? true : false)

#define IS_IN_CLIP_X(_x, _clipX, _clipW)                    \
    ((                                                      \
        ((_x) >= (_clipX)) &&                               \
        ((_x) < ((_clipX) + (_clipW)))                      \
    ) ? true : false)

#define IS_IN_CLIP_Y(_y, _clipY, _clipH)                    \
    ((                                                      \
        ((_y) >= (_clipY)) &&                               \
        ((_y) < ((_clipY) + (_clipH)))                      \
    ) ? true : false)

class JColor : public JObject {
public:
    uint32_t getValue(int32_t valueMode) const {
        uint32_t value = getFields().getFieldData32ByIndex(0)->value;
        uint8_t alpha = value >> 24;
        switch(valueMode) {
            case COLOR_MODE_RGB555:
                value = ((value >> 3) & 0x1F) | ((value >> 6) & 0x03E0) | ((value >> 9) & 0x7C00);
                return value | ((alpha >> 3) << 24);
            case COLOR_MODE_RGB565:
                value = ((value >> 3) & 0x1F) | ((value >> 5) & 0x07E0) | ((value >> 8) & 0xF800);
                return value | ((alpha >> 3) << 24);
            case COLOR_MODE_BGR565:
                value = ((value << 8) & 0xF800) | ((value >> 5) & 0x07E0) | ((value >> 19) & 0x1F);
                return value | ((alpha >> 3) << 24);
            default:
                return value;
        }
    }
};

class JGraphics : public JObject {
private:
    JGraphics(void) = delete;
    JGraphics(const JGraphics &) = delete;
    void operator=(const JGraphics &) = delete;

    int32_t getColorMode(void) {
        return getFields().getFieldData32ByIndex(0)->value;
    }

    uint8_t *getColorBuffer(void) {
        return (uint8_t *)((JInt8Array *)(getFields().getFieldObjectByIndex(0)->object))->getData();
    }

    int32_t getX(void) {
        return getFields().getFieldData32ByIndex(1)->value;
    }

    int32_t getY(void) {
        return getFields().getFieldData32ByIndex(2)->value;
    }

    int32_t getWidth(void) {
        return getFields().getFieldData32ByIndex(3)->value;
    }

    int32_t getClipX(void) {
        return getFields().getFieldData32ByIndex(4)->value;
    }

    int32_t getClipY(void) {
        return getFields().getFieldData32ByIndex(5)->value;
    }

    int32_t getClipWidth(void) {
        return getFields().getFieldData32ByIndex(6)->value;
    }

    int32_t getClipHeight(void) {
        return getFields().getFieldData32ByIndex(7)->value;
    }

    void drawPixel(int32_t color, int32_t x, int32_t y) {
        uint8_t alpha = color >> 24;
        int32_t colorMode = getColorMode();
        if(colorMode == COLOR_MODE_RGB888) {
            uint8_t *buff = &getColorBuffer()[(y * getWidth() + x) * 3];
            uint8_t *rgb = (uint8_t *)&color;
            if(alpha == 0xFF) {
                buff[0] = rgb[0];
                buff[1] = rgb[1];
                buff[2] = rgb[2];
            }
            else {
                buff[0] += (rgb[0] - buff[0]) * alpha >> 8;
                buff[1] += (rgb[1] - buff[1]) * alpha >> 8;
                buff[2] += (rgb[2] - buff[2]) * alpha >> 8;
            }
        }
        else {
            uint8_t *buff = &getColorBuffer()[(y * getWidth() + x) * 2];
            if(alpha == 0x1F) {
                buff[0] = color >> 8;
                buff[1] = color;
            }
            else {
                uint32_t mask = colorMask[colorMode];
                uint32_t fg = (uint16_t)color;
                fg = (fg | (fg << 16)) & mask;
                uint32_t bg = (buff[0] << 8) | buff[1];
                bg = (bg | (bg << 16)) & mask;
                bg += (fg - bg) * alpha >> 5;
                bg &= mask;
                bg = (bg | bg >> 16);
                buff[0] = bg >> 8;
                buff[1] = bg;
            }
        }
    }

    void drawHorizontaLine(int32_t color, int32_t x1, int32_t x2, int32_t y) {
        uint8_t alpha = color >> 24;
        int32_t colorMode = getColorMode();
        if(colorMode == COLOR_MODE_RGB888) {
            uint8_t *buff = &getColorBuffer()[y * getWidth() * 3];
            uint8_t *rgb = (uint8_t *)&color;
            x1 *= 3;
            x2 *= 3;
            if(alpha == 0xFF) {
                for(; x1 <= x2; x1 += 3) {
                    buff[x1 + 0] = rgb[0];
                    buff[x1 + 1] = rgb[1];
                    buff[x1 + 2] = rgb[2];
                }
            }
            else {
                for(; x1 <= x2; x1 += 3) {
                    buff[x1 + 0] += (rgb[0] - buff[x1 + 0]) * alpha >> 8;
                    buff[x1 + 1] += (rgb[1] - buff[x1 + 1]) * alpha >> 8;
                    buff[x1 + 2] += (rgb[2] - buff[x1 + 2]) * alpha >> 8;
                }
            }
        }
        else {
            if(alpha == 0x1F) {
                uint16_t *buff = &((uint16_t *)getColorBuffer())[y * getWidth()];
                uint16_t rgb16 = (uint16_t)((color >> 8) | (color << 8));
                for(; x1 <= x2; x1++)
                    buff[x1] = rgb16;
            }
            else {
                uint8_t *buff = &getColorBuffer()[y * getWidth() * 2];
                uint32_t mask = colorMask[colorMode];
                uint32_t fg = (uint16_t)color;
                fg = (fg | (fg << 16)) & mask;
                x1 *= 2;
                x2 *= 2;
                for(; x1 <= x2; x1 += 2) {
                    uint32_t bg = (buff[x1] << 8) | buff[x1 + 1];
                    bg = (bg | (bg << 16)) & mask;
                    bg += (fg - bg) * alpha >> 5;
                    bg &= mask;
                    bg = (bg | bg >> 16);
                    buff[x1] = bg >> 8;
                    buff[x1 + 1] = bg;
                }
            }
        }
    }

    void drawVerticalLine(int32_t color, int32_t y1, int32_t y2, int32_t x) {
        uint8_t alpha = color >> 24;
        int32_t width = getWidth();
        int32_t colorMode = getColorMode();
        if(colorMode == COLOR_MODE_RGB888) {
            uint8_t *buff = getColorBuffer();
            uint8_t *rgb = (uint8_t *)&color;
            x *= 3;
            if(alpha == 0xFF) {
                for(; y1 <= y2; y1++) {
                    uint32_t index = (y1 * width * 3) + x;
                    buff[index + 0] = rgb[0];
                    buff[index + 1] = rgb[1];
                    buff[index + 2] = rgb[2];
                }
            }
            else {
                for(; y1 <= y2; y1++) {
                    int32_t index = (y1 * width * 3) + x;
                    buff[index + 0] += (rgb[0] - buff[index + 0]) * alpha >> 8;
                    buff[index + 1] += (rgb[1] - buff[index + 1]) * alpha >> 8;
                    buff[index + 2] += (rgb[2] - buff[index + 2]) * alpha >> 8;
                }
            }
        }
        else {
            uint16_t *buff = (uint16_t *)getColorBuffer();
            if(alpha == 0x1F) {
                uint16_t rgb16 = (uint16_t)((color >> 8) | (color << 8));
                for(; y1 <= y2; y1++)
                    (&buff[y1 * width])[x] = rgb16;
            }
            else {
                uint32_t mask = colorMask[colorMode];
                uint32_t fg = (uint16_t)color;
                fg = (fg | (fg << 16)) & mask;
                x *= 2;
                for(; y1 <= y2; y1++) {
                    uint8_t *pixel = (uint8_t *)&buff[y1 * width];
                    uint32_t bg = (pixel[x] << 8) | pixel[x + 1];
                    bg = (bg | (bg << 16)) & mask;
                    bg += (fg - bg) * alpha >> 5;
                    bg &= mask;
                    bg = (bg | bg >> 16);
                    pixel[x] = bg >> 8;
                    pixel[x + 1] = bg;
                }
            }
        }
    }

    void fillFastRect(int32_t color, int32_t x1, int32_t x2, int32_t y1, int32_t y2) {
        uint8_t alpha = color >> 24;
        int32_t width = getWidth();
        int32_t colorMode = getColorMode();
        if(colorMode == COLOR_MODE_RGB888) {
            uint8_t *buff = getColorBuffer();
            uint8_t *rgb = (uint8_t *)&color;
            x1 *= 3;
            x2 *= 3;
            if(alpha == 0xFF) {
                for(; y1 <= y2; y1++) {
                    uint8_t *pixels = &buff[y1 * width * 3];
                    for(int32_t x = x1; x <= x2; x += 3) {
                        pixels[x + 0] = rgb[0];
                        pixels[x + 1] = rgb[1];
                        pixels[x + 2] = rgb[2];
                    }
                }
            }
            else {
                for(; y1 <= y2; y1++) {
                    uint8_t *pixels = &buff[y1 * width * 3];
                    for(int32_t x = x1; x <= x2; x += 3) {
                        pixels[x + 0] += (rgb[0] - pixels[x + 0]) * alpha >> 8;
                        pixels[x + 1] += (rgb[1] - pixels[x + 1]) * alpha >> 8;
                        pixels[x + 2] += (rgb[2] - pixels[x + 2]) * alpha >> 8;
                    }
                }
            }
        }
        else {
            if(alpha == 0x1F) {
                uint16_t *buff = (uint16_t *)getColorBuffer();
                uint16_t rgb16 = (uint16_t)((color >> 8) | (color << 8));
                for(; y1 <= y2; y1++) {
                    uint16_t *pixels = &buff[y1 * width];
                    for(int32_t x = x1; x <= x2; x++)
                        pixels[x] = rgb16;
                }
            }
            else {
                uint8_t *buff = getColorBuffer();
                uint32_t mask = colorMask[colorMode];
                uint32_t fg = (uint16_t)color;
                fg = (fg | (fg << 16)) & mask;
                x1 *= 2;
                x2 *= 2;
                for(; y1 <= y2; y1++) {
                    uint8_t *pixels = &buff[y1 * width * 2];
                    for(int32_t x = x1; x <= x2; x += 2) {
                        uint32_t bg = (pixels[x] << 8) | pixels[x];
                        bg = (bg | (bg << 16)) & mask;
                        bg += (fg - bg) * alpha >> 5;
                        bg &= mask;
                        bg = (bg | bg >> 16);
                        pixels[x] = bg >> 8;
                        pixels[x + 1] = bg;
                    }
                }
            }
        }
    }

    void drawCircleQuadrant(int32_t color, int32_t xc, int32_t yc, int32_t r, int32_t q) {
        int32_t x = 0, y = r;
        int32_t d = 3 - 2 * r;
        int32_t clipX = getClipX() - getX();
        int32_t clipY = getClipY() - getY();
        int32_t clipW = getClipWidth();
        int32_t clipH = getClipHeight();
        while(x <= y) {
            if(q & 0x01) {
                if(IS_IN_CLIP(xc - x, yc - y, clipX, clipY, clipW, clipH)) drawPixel(color, xc - x, yc - y);
                if(IS_IN_CLIP(xc - y, yc - x, clipX, clipY, clipW, clipH)) drawPixel(color, xc - y, yc - x);
            }
            if(q & 0x02) {
                if(IS_IN_CLIP(xc + x, yc - y, clipX, clipY, clipW, clipH)) drawPixel(color, xc + x, yc - y);
                if(IS_IN_CLIP(xc + y, yc - x, clipX, clipY, clipW, clipH)) drawPixel(color, xc + y, yc - x);
            }
            if(q & 0x04) {
                if(IS_IN_CLIP(xc + x, yc + y, clipX, clipY, clipW, clipH)) drawPixel(color, xc + x, yc + y);
                if(IS_IN_CLIP(xc + y, yc + x, clipX, clipY, clipW, clipH)) drawPixel(color, xc + y, yc + x);
            }
            if(q & 0x08) {
                if(IS_IN_CLIP(xc - x, yc + y, clipX, clipY, clipW, clipH)) drawPixel(color, xc - x, yc + y);
                if(IS_IN_CLIP(xc - y, yc + x, clipX, clipY, clipW, clipH)) drawPixel(color, xc - y, yc + x);
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

    void fillCircleQuadrant(int32_t color, int32_t xc, int32_t yc, int32_t r, int32_t q) {
        int32_t x = 0, y = r;
        int32_t d = 3 - 2 * r;
        int32_t clipX = getClipX() - getX();
        int32_t clipY = getClipY() - getY();
        int32_t clipW = getClipWidth();
        int32_t clipH = getClipHeight();
        int32_t xc1 = FLINT_MAX(xc, clipX);
        int32_t xc2 = FLINT_MIN(xc, clipX + clipW - 1);
        while (x <= y) {
            if(IS_IN_CLIP_Y(yc - x, clipY, clipH)) {
                if(q & 0x01) drawHorizontaLine(color, FLINT_MAX(clipX, xc - y), xc2, yc - x);
                if(q & 0x02) drawHorizontaLine(color, xc1, FLINT_MIN(xc + y, clipX + clipW - 1), yc - x);
            }
            if(IS_IN_CLIP_Y(yc + x, clipY, clipH)) {
                if(q & 0x04) drawHorizontaLine(color, xc1, FLINT_MIN(xc + y, clipX + clipW - 1), yc + x);
                if(q & 0x08) drawHorizontaLine(color, FLINT_MAX(clipX, xc - y), xc2, yc + x);
            }
            if(d > 0) {
                if(x != y) {
                    if(IS_IN_CLIP_Y(yc - y, clipY, clipH)) {
                        if(q & 0x01) drawHorizontaLine(color, FLINT_MAX(clipX, xc - x), xc2, yc - y);
                        if(q & 0x02) drawHorizontaLine(color, xc1, FLINT_MIN(xc + x, clipX + clipW - 1), yc - y);
                    }
                    if(IS_IN_CLIP_Y(yc + y, clipY, clipH)) {
                        if(q & 0x04) drawHorizontaLine(color, xc1, FLINT_MIN(xc + x, clipX + clipW - 1), yc + y);
                        if(q & 0x08) drawHorizontaLine(color, FLINT_MAX(clipX, xc - x), xc2, yc + y);
                    }
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
    void drawLine(JColor *color, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
        uint32_t colorValue = color->getValue(getColorMode());
        int32_t originX = getX();
        int32_t originY = getY();
        int32_t clipX = getClipX() - originX;
        int32_t clipY = getClipY() - originY;
        int32_t clipW = getClipWidth();
        int32_t clipH = getClipHeight();
        x1 -= originX;
        x2 -= originX;
        y1 -= originY;
        y2 -= originY;

        if(x1 == x2) {
            if(!IS_IN_CLIP_X(x1, clipX, clipW))
                return;
            if(y1 > y2)
                FLINT_SWAP(y1, y2);
            drawVerticalLine(colorValue, FLINT_MAX(y1, clipY), FLINT_MIN(y2, clipY + clipH - 1), x1);
        }
        else if(y1 == y2) {
            if(!IS_IN_CLIP_Y(y1, clipY, clipH))
                return;
            if(x1 > x2)
                FLINT_SWAP(x1, x2);
            drawHorizontaLine(colorValue, FLINT_MAX(x1, clipX), FLINT_MIN(x2, clipX + clipW - 1), y1);
        }
        else {
            int32_t dx = FLINT_ABS(x2 - x1), sx = x1 < x2 ? 1 : -1;
            int32_t dy = -FLINT_ABS(y2 - y1), sy = y1 < y2 ? 1 : -1;
            int32_t err = dx + dy;
            while(1) {
                if(IS_IN_CLIP(x1, y1, clipX, clipY, clipW, clipH))
                    drawPixel(colorValue, x1, y1);
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

    void drawRect(JColor *color, int32_t x, int32_t y, int32_t width, int32_t height) {
        uint32_t colorValue = color->getValue(getColorMode());
        int32_t originX = getX();
        int32_t originY = getY();
        int32_t clipX = getClipX() - originX;
        int32_t clipY = getClipY() - originY;
        int32_t clipW = getClipWidth();
        int32_t clipH = getClipHeight();
        x -= originX;
        y -= originY;

        int32_t x1 = FLINT_MAX(x, clipX);
        int32_t x2 = FLINT_MIN(x + width, clipX + clipW) - 1;
        int32_t y1 = FLINT_MAX(y, clipY);
        int32_t y2 = FLINT_MIN(y + height, clipY + clipH) - 1;

        if(y1 <= y2) {
            if((x1 <= x) && (x <= x2)) drawVerticalLine(colorValue, y1, y2, x1);
            if((x2 >= clipX) && x2 == (x + width - 1)) drawVerticalLine(colorValue, y1, y2, x2);
        }
        if(x1 <= x2) {
            if((y1 <= y) && (y <= y2)) drawHorizontaLine(colorValue, x1, x2, y1);
            if((y2 >= clipY) && y2 == (y + height - 1)) drawHorizontaLine(colorValue, x1, x2, y2);
        }
    }

    void fillRect(JColor *color, int32_t x, int32_t y, int32_t width, int32_t height) {
        int32_t originX = getX();
        int32_t originY = getY();
        int32_t clipX = getClipX() - originX;
        int32_t clipY = getClipY() - originY;
        int32_t clipW = getClipWidth();
        int32_t clipH = getClipHeight();
        x -= originX;
        y -= originY;

        int32_t x1 = FLINT_MAX(x, clipX);
        int32_t x2 = FLINT_MIN(x + width, clipX + clipW) - 1;
        if(x1 > x2)
            return;
        int32_t y1 = FLINT_MAX(y, clipY);
        int32_t y2 = FLINT_MIN(y + height, clipY + clipH) - 1;
        if(y1 > y2)
            return;

        fillFastRect(color->getValue(getColorMode()), x1, x2, y1, y2);
    }

    void drawRoundRect(JColor *color, int32_t x, int32_t y, int32_t width, int32_t height, int32_t r1, int32_t r2, int32_t r3, int32_t r4) {
        uint32_t colorValue = color->getValue(getColorMode());
        int32_t originX = getX();
        int32_t originY = getY();
        int32_t clipX = getClipX() - originX;
        int32_t clipY = getClipY() - originY;
        int32_t clipW = getClipWidth();
        int32_t clipH = getClipHeight();
        x -= originX;
        y -= originY;

        r1 = (r1 < 0) ? 0 : FLINT_MIN(r1, width / 2);
        r2 = (r2 < 0) ? 0 : FLINT_MIN(r2, width / 2);
        r3 = (r3 < 0) ? 0 : FLINT_MIN(r3, width / 2);
        r4 = (r4 < 0) ? 0 : FLINT_MIN(r4, width / 2);

        int32_t x1 = FLINT_MAX(x, clipX);
        int32_t x2 = FLINT_MIN(x + width, clipX + clipW) - 1;
        int32_t y1 = FLINT_MAX(y, clipY);
        int32_t y2 = FLINT_MIN(y + height, clipY + clipH) - 1;

        if(y1 <= y2) {
            if((x1 <= x) && (x <= x2)) drawVerticalLine(colorValue, FLINT_MAX(y1, y + r1), FLINT_MIN(y2, y + height - r4), x1);
            if((x2 >= clipX) && x2 == (x + width - 1)) drawVerticalLine(colorValue, FLINT_MAX(y1, y + r2), FLINT_MIN(y2, y + height - r3), x2);
        }
        if(x1 <= x2) {
            if((y1 <= y) && (y <= y2)) drawHorizontaLine(colorValue, FLINT_MAX(x1, x + r1), FLINT_MIN(x2, x + width - r2), y1);
            if((y2 >= clipY) && y2 == (y + height - 1)) drawHorizontaLine(colorValue, FLINT_MAX(x1, x + r4), FLINT_MIN(x2, x + width - r3), y2);
        }

        if(r1) drawCircleQuadrant(colorValue, x + r1, y + r1, r1, 0x01);
        if(r2) drawCircleQuadrant(colorValue, x + width - 1 - r2, y + r2, r2, 0x02);
        if(r3) drawCircleQuadrant(colorValue, x + width - 1 - r3, y + height - 1 - r3, r3, 0x04);
        if(r4) drawCircleQuadrant(colorValue, x + r4, y + height - 1 - r4, r4, 0x08);
    }

    void fillRoundRect(JColor *color, int32_t x, int32_t y, int32_t width, int32_t height, int32_t r1, int32_t r2, int32_t r3, int32_t r4) {
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

        fillRect(color, x2, y, w2, height);
        fillRect(color, x, y + r1 + !!r1, r14 + !!r14, height - r1 - r4 - !!r1 - !!r4);
        fillRect(color, x + width - r23 - !!r23, y + r2 + !!r2, r23 + 1, height - r2 - r3 - !!r2 - !!r3);

        if(r1 != r4) {
            if(r1 < r4)
                fillRect(color, x + r1 + !!r1, y, r4 - r1, r1 + 1);
            else
                fillRect(color, x + r4 + !!r4, y + height - r4 - 1, r14 - r4, r4 + 1);
        }
        if(r2 != r3) {
            if(r2 < r3)
                fillRect(color, x + width - r3 - 1, y, r3 - r2, r2 + 1);
            else
                fillRect(color, x + width - r2 - 1, y + height - r3 - 1, r23 - r3, r3 + 1);
        }

        uint32_t colorValue = color->getValue(getColorMode());
        x -= getX();
        y -= getY();

        if(r1) fillCircleQuadrant(colorValue, x + r1, y + r1, r1, 0x01);
        if(r2) fillCircleQuadrant(colorValue, x + width - 1 - r2, y + r2, r2, 0x02);
        if(r3) fillCircleQuadrant(colorValue, x + width - 1 - r3, y + height - 1 - r3, r3, 0x04);
        if(r4) fillCircleQuadrant(colorValue, x + r4, y + height - 1 - r4, r4, 0x08);
    }

    void drawEllipse(JColor *color, int32_t x, int32_t y, int32_t width, int32_t height) {
        uint32_t colorValue = color->getValue(getColorMode());
        int32_t originX = getX();
        int32_t originY = getY();
        int32_t clipX = getClipX() - originX;
        int32_t clipY = getClipY() - originY;
        int32_t clipW = getClipWidth();
        int32_t clipH = getClipHeight();
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
            if(IS_IN_CLIP(x2, y, clipX, clipY, clipW, clipH)) drawPixel(colorValue, x2, y);
            if(IS_IN_CLIP(x, y, clipX, clipY, clipW, clipH)) drawPixel(colorValue, x, y);
            if(IS_IN_CLIP(x, y2, clipX, clipY, clipW, clipH)) drawPixel(colorValue, x, y2);
            if(IS_IN_CLIP(x2, y2, clipX, clipY, clipW, clipH)) drawPixel(colorValue, x2, y2);
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
            if(IS_IN_CLIP(x - 1, y, clipX, clipY, clipW, clipH)) drawPixel(colorValue, x - 1, y);
            if(IS_IN_CLIP(x2 + 1, y, clipX, clipY, clipW, clipH)) drawPixel(colorValue, x2 + 1, y);
            if(IS_IN_CLIP(x - 1, y2, clipX, clipY, clipW, clipH)) drawPixel(colorValue, x - 1, y2);
            if(IS_IN_CLIP(x2 + 1, y2, clipX, clipY, clipW, clipH)) drawPixel(colorValue, x2 + 1, y2);
            y++;
            y2--;
        }
    }

    void fillEllipse(JColor *color, int32_t x, int32_t y, int32_t width, int32_t height) {
        uint32_t colorValue = color->getValue(getColorMode());
        int32_t originX = getX();
        int32_t originY = getY();
        int32_t clipX = getClipX() - originX;
        int32_t clipY = getClipY() - originY;
        int32_t clipW = getClipWidth();
        int32_t clipH = getClipHeight();
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
                int32_t tmp2 = FLINT_MIN(x2, clipX + clipW - 1);
                if(IS_IN_CLIP_Y(y, clipY, clipH)) drawHorizontaLine(colorValue, tmp1, tmp2, y);
                if(IS_IN_CLIP_Y(y2, clipY, clipH)) drawHorizontaLine(colorValue, tmp1, tmp2, y2);
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
            int32_t tmp2 = FLINT_MIN(x2 + 1, clipX + clipW - 1);
            if(IS_IN_CLIP_Y(y, clipY, clipH)) drawHorizontaLine(colorValue, tmp1, tmp2, y++);
            if(IS_IN_CLIP_Y(y2, clipY, clipH)) drawHorizontaLine(colorValue, tmp1, tmp2, y2--);
        }
    }

    void drawPolyline(JColor *color, int32_t *xPoints, int32_t *yPoints, int32_t nPoints) {
        nPoints--;
        for(int32_t i = 0; i < nPoints; i++)
            drawLine(color, xPoints[i], yPoints[i], xPoints[i + 1], yPoints[i + 1]);
    }

    void drawPolygon(JColor *color, int32_t *xPoints, int32_t *yPoints, int32_t nPoints) {
        if(nPoints > 0) {
            drawPolyline(color, xPoints, yPoints, nPoints);
            drawLine(color, xPoints[0], yPoints[0], xPoints[nPoints - 1], yPoints[nPoints - 1]);
        }
    }
};

static FlintError nativeClear(FlintExecution *exec) {
    JObject *g = exec->stackPopObject();
    JInt8Array *colorBuffer = (JInt8Array *)g->getFields().getFieldObjectByIndex(0)->object;
    memset(colorBuffer->getData(), 0, colorBuffer->getLength());
    return ERR_OK;
}

static FlintError nativeDrawLine(FlintExecution *exec) {
    int32_t y2 = exec->stackPopInt32();
    int32_t x2 = exec->stackPopInt32();
    int32_t y1 = exec->stackPopInt32();
    int32_t x1 = exec->stackPopInt32();
    JColor *colorObj = (JColor *)exec->stackPopObject();
    if(!colorObj)
        return throwNullPointerException(exec);
    JGraphics *g = (JGraphics *)exec->stackPopObject();
    g->drawLine(colorObj, x1, y1, x2, y2);
    return ERR_OK;
}

static FlintError nativeDrawRect(FlintExecution *exec) {
    int32_t height = exec->stackPopInt32();
    int32_t width = exec->stackPopInt32();
    int32_t y = exec->stackPopInt32();
    int32_t x = exec->stackPopInt32();
    JColor *colorObj = (JColor *)exec->stackPopObject();
    if(!colorObj)
        return throwNullPointerException(exec);
    JGraphics *g = (JGraphics *)exec->stackPopObject();
    g->drawRect(colorObj, x, y, width, height);
    return ERR_OK;
}

static FlintError nativeFillRect(FlintExecution *exec) {
    int32_t height = exec->stackPopInt32();
    int32_t width = exec->stackPopInt32();
    int32_t y = exec->stackPopInt32();
    int32_t x = exec->stackPopInt32();
    JColor *colorObj = (JColor *)exec->stackPopObject();
    if(!colorObj)
        return throwNullPointerException(exec);
    JGraphics *g = (JGraphics *)exec->stackPopObject();
    g->fillRect(colorObj, x, y, width, height);
    return ERR_OK;
}

static FlintError nativeDrawRoundRect(FlintExecution *exec) {
    int32_t r4 = exec->stackPopInt32();
    int32_t r3 = exec->stackPopInt32();
    int32_t r2 = exec->stackPopInt32();
    int32_t r1 = exec->stackPopInt32();
    int32_t height = exec->stackPopInt32();
    int32_t width = exec->stackPopInt32();
    int32_t y = exec->stackPopInt32();
    int32_t x = exec->stackPopInt32();
    JColor *colorObj = (JColor *)exec->stackPopObject();
    if(!colorObj)
        return throwNullPointerException(exec);
    JGraphics *g = (JGraphics *)exec->stackPopObject();
    g->drawRoundRect(colorObj, x, y, width, height, r1, r2, r3, r4);
    return ERR_OK;
}

static FlintError nativeFillRoundRect(FlintExecution *exec) {
    int32_t r4 = exec->stackPopInt32();
    int32_t r3 = exec->stackPopInt32();
    int32_t r2 = exec->stackPopInt32();
    int32_t r1 = exec->stackPopInt32();
    int32_t height = exec->stackPopInt32();
    int32_t width = exec->stackPopInt32();
    int32_t y = exec->stackPopInt32();
    int32_t x = exec->stackPopInt32();
    JColor *colorObj = (JColor *)exec->stackPopObject();
    if(!colorObj)
        return throwNullPointerException(exec);
    JGraphics *g = (JGraphics *)exec->stackPopObject();
    g->fillRoundRect(colorObj, x, y, width, height, r1, r2, r3, r4);
    return ERR_OK;
}

static FlintError nativeDrawEllipse(FlintExecution *exec) {
    int32_t height = exec->stackPopInt32();
    int32_t width = exec->stackPopInt32();
    int32_t y = exec->stackPopInt32();
    int32_t x = exec->stackPopInt32();
    JColor *colorObj = (JColor *)exec->stackPopObject();
    if(!colorObj)
        return throwNullPointerException(exec);
    JGraphics *g = (JGraphics *)exec->stackPopObject();
    g->drawEllipse(colorObj, x, y, width, height);
    return ERR_OK;
}

static FlintError nativeFillEllipse(FlintExecution *exec) {
    int32_t height = exec->stackPopInt32();
    int32_t width = exec->stackPopInt32();
    int32_t y = exec->stackPopInt32();
    int32_t x = exec->stackPopInt32();
    JColor *colorObj = (JColor *)exec->stackPopObject();
    if(!colorObj)
        return throwNullPointerException(exec);
    JGraphics *g = (JGraphics *)exec->stackPopObject();
    g->fillEllipse(colorObj, x, y, width, height);
    return ERR_OK;
}

static FlintError nativeDrawArc(FlintExecution *exec) {
    int32_t arcAngle = exec->stackPopInt32();
    int32_t startAngle = exec->stackPopInt32();
    int32_t height = exec->stackPopInt32();
    int32_t width = exec->stackPopInt32();
    int32_t y = exec->stackPopInt32();
    int32_t x = exec->stackPopInt32();
    JColor *colorObj = (JColor *)exec->stackPopObject();
    if(!colorObj)
        return throwNullPointerException(exec);
    JGraphics *g = (JGraphics *)exec->stackPopObject();
    // TODO
    return ERR_OK;
}

static FlintError nativeFillArc(FlintExecution *exec) {
    int32_t arcAngle = exec->stackPopInt32();
    int32_t startAngle = exec->stackPopInt32();
    int32_t height = exec->stackPopInt32();
    int32_t width = exec->stackPopInt32();
    int32_t y = exec->stackPopInt32();
    int32_t x = exec->stackPopInt32();
    JColor *colorObj = (JColor *)exec->stackPopObject();
    if(!colorObj)
        return throwNullPointerException(exec);
    JGraphics *g = (JGraphics *)exec->stackPopObject();
    // TODO
    return ERR_OK;
}

static FlintError nativeDrawPolyline(FlintExecution *exec) {
    int32_t nPoints = exec->stackPopInt32();
    JInt32Array *yPoints = (JInt32Array *)exec->stackPopObject();
    JInt32Array *xPoints = (JInt32Array *)exec->stackPopObject();
    JColor *colorObj = (JColor *)exec->stackPopObject();
    if(!yPoints || !xPoints || !colorObj)
        return throwNullPointerException(exec);
    JGraphics *g = (JGraphics *)exec->stackPopObject();
    if(nPoints > xPoints->getLength())
        return throwArrayIndexOutOfBoundsException(exec, nPoints, xPoints->getLength());
    if(nPoints > yPoints->getLength())
        return throwArrayIndexOutOfBoundsException(exec, nPoints, yPoints->getLength());
    g->drawPolyline(colorObj, xPoints->getData(), yPoints->getData(), nPoints);
    return ERR_OK;
}

static FlintError nativeDrawPolygon(FlintExecution *exec) {
    int32_t nPoints = exec->stackPopInt32();
    JInt32Array *yPoints = (JInt32Array *)exec->stackPopObject();
    JInt32Array *xPoints = (JInt32Array *)exec->stackPopObject();
    JColor *colorObj = (JColor *)exec->stackPopObject();
    if(!yPoints || !xPoints || !colorObj)
        return throwNullPointerException(exec);
    JGraphics *g = (JGraphics *)exec->stackPopObject();
    if(nPoints > xPoints->getLength())
        return throwArrayIndexOutOfBoundsException(exec, nPoints, xPoints->getLength());
    if(nPoints > yPoints->getLength())
        return throwArrayIndexOutOfBoundsException(exec, nPoints, yPoints->getLength());
    g->drawPolygon(colorObj, xPoints->getData(), yPoints->getData(), nPoints);
    return ERR_OK;
}

static FlintError nativeFillPolygon(FlintExecution *exec) {
    int32_t nPoints = exec->stackPopInt32();
    JInt32Array *yPoints = (JInt32Array *)exec->stackPopObject();
    JInt32Array *xPoints = (JInt32Array *)exec->stackPopObject();
    JColor *colorObj = (JColor *)exec->stackPopObject();
    if(!yPoints || !xPoints || !colorObj)
        return throwNullPointerException(exec);
    JGraphics *g = (JGraphics *)exec->stackPopObject();
    // TODO
    return ERR_OK;
}

static FlintError nativeDrawString(FlintExecution *exec) {
    JGraphics *g = (JGraphics *)exec->stackPopObject();
    // TODO
    return ERR_OK;
}

static FlintError nativeDrawImage1(FlintExecution *exec) {
    JGraphics *g = (JGraphics *)exec->stackPopObject();
    // TODO
    return ERR_OK;
}

static FlintError nativeDrawImage2(FlintExecution *exec) {
    JGraphics *g = (JGraphics *)exec->stackPopObject();
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
