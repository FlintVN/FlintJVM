
#include <string.h>
#include "flint.h"
#include "flint_object.h"
#include "flint_const_name.h"
#include "flint_native_graphics_class.h"

#define COLOR_MODE_RGB444       0
#define COLOR_MODE_RGB555       1
#define COLOR_MODE_RGB565       2
#define COLOR_MODE_BGR565       3
#define COLOR_MODE_RGB888       4

static const uint32_t colorMask[] = {0x00F00F0F, 0x03E07C1F, 0x07E0F81F, 0x07E0F81F};

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
        if((x < clipX) || (y < clipY) || (x >= (clipX + clipWidth)) || (y >= (clipY + clipHeight)))
            return;
        ((uint16_t *)colorBuffer)[y * gwidth + x] = rgb16;
    }

    void drawPixel2(int32_t x, int32_t y) {
        if((x < clipX) || (y < clipY) || (x >= (clipX + clipWidth)) || (y >= (clipY + clipHeight)))
            return;
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

    void drawPixel3(int32_t x, int32_t y) {
        if((x < clipX) || (y < clipY) || (x >= (clipX + clipWidth)) || (y >= (clipY + clipHeight)))
            return;
        uint8_t *buff = (uint8_t *)&colorBuffer[(y * gwidth + x) * 3];
        buff[0] = rgb[0];
        buff[1] = rgb[1];
        buff[2] = rgb[2];
    }

    void drawPixel4(int32_t x, int32_t y) {
        if((x < clipX) || (y < clipY) || (x >= (clipX + clipWidth)) || (y >= (clipY + clipHeight)))
            return;
        uint8_t alpha = rgb[3];
        uint8_t *buff = (uint8_t *)&colorBuffer[(y * gwidth + x) * 3];
        buff[0] += (rgb[0] - buff[0]) * alpha >> 8;
        buff[1] += (rgb[1] - buff[1]) * alpha >> 8;
        buff[2] += (rgb[2] - buff[2]) * alpha >> 8;
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
public:
    FlintGraphics(FlintObject *g, uint32_t color) {
        uint8_t alpha = color >> 24;
        FlintFieldsData &fields = g->getFields();
        FlintInt8Array *buffer = (FlintInt8Array *)fields.getFieldObjectByIndex(0).object;
        colorMode = (uint8_t)fields.getFieldData32ByIndex(0).value;
        colorBuffer = (uint8_t *)buffer->getData();
        originX = fields.getFieldData32ByIndex(1).value;
        originY = fields.getFieldData32ByIndex(2).value;
        gwidth = fields.getFieldData32ByIndex(3).value;
        clipX = fields.getFieldData32ByIndex(4).value - originX;
        clipY = fields.getFieldData32ByIndex(5).value - originY;
        clipWidth = fields.getFieldData32ByIndex(6).value;
        clipHeight = fields.getFieldData32ByIndex(7).value;

        switch (colorMode) {
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
            if((x1 < clipX) || (x1 >= (clipX + clipWidth)))
                return;
            if(y1 > y2)
                FLINT_SWAP(y1, y2);
            y1 = FLINT_MAX(y1, clipY);
            y2 = FLINT_MIN(y2, clipHeight - 1);
            if(y1 <= y2)
                (this->*drawFastVLine)(y1, y2, x1);
        }
        else if(y1 == y2) {
            if((y1 < clipY) || (y1 >= (clipY + clipHeight)))
                return;
            if(x1 > x2)
                FLINT_SWAP(x1, x2);
            x1 = FLINT_MAX(x1, clipX);
            x2 = FLINT_MIN(x2, clipWidth - 1);
            if(x1 <= x2)
                (this->*drawFastHLine)(x1, x2, y1);
        }
        else {
            int16_t dx = FLINT_ABS(x2 - x1);
            int16_t dy = FLINT_ABS(y2 - y1);
            int8_t x_unit = (x1 < x2) ? 1 : -1;
            int8_t y_unit = (y1 < y2) ? 1 : -1;
            if(dx >= dy) {
                int16_t p = 2 * dy - dx;
                (this->*drawPixel)(x1, y1);
                while(x1 != x2) {
                    if(p < 0)
                        p += 2 * dy;
                    else {
                        p += 2 * (dy - dx);
                        y1 += y_unit;
                    }
                    x1 += x_unit;
                    (this->*drawPixel)(x1, y1);
                }
            }
            else {
                int16_t p = 2 * dx - dy;
                (this->*drawPixel)(x1, y1);
                while(y1 != y2) {
                    if(p < 0)
                        p += 2 * dx;
                    else {
                        p += 2 * (dx - dy);
                        x1 += x_unit;
                    }
                    y1 += y_unit;
                    (this->*drawPixel)(x1, y1);
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

class FlintColor : public FlintObject {
public:
    uint32_t getValue(void) const {
        return getFields().getFieldData32(*(const FlintConstUtf8 *)"\x05\x00\x2B\x6E""value").value;
    }
};

static FlintObject *checkNullObject(FlintExecution &execution, FlintObject *obj) {
    if(!obj)
        throw &execution.flint.newNullPointerException();
    return obj;
}

static void nativeClear(FlintExecution &execution) {
    FlintObject *g = execution.stackPopObject();
    FlintInt8Array *colorBuffer = (FlintInt8Array *)g->getFields().getFieldObject(*(const FlintConstUtf8 *)"\x0B\x00\x05\xB6""colorBuffer").object;
    memset(colorBuffer->getData(), 0, colorBuffer->getLength());
}

static void nativeDrawLine(FlintExecution &execution) {
    int32_t y2 = execution.stackPopInt32();
    int32_t x2 = execution.stackPopInt32();
    int32_t y1 = execution.stackPopInt32();
    int32_t x1 = execution.stackPopInt32();
    uint32_t color = ((FlintColor *)checkNullObject(execution, execution.stackPopObject()))->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    g.drawLine(x1, y1, x2, y2);
}

static void nativeDrawRect(FlintExecution &execution) {
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    uint32_t color = ((FlintColor *)checkNullObject(execution, execution.stackPopObject()))->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    g.drawRect(x, y, width, height);
}

static void nativeFillRect(FlintExecution &execution) {
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    uint32_t color = ((FlintColor *)checkNullObject(execution, execution.stackPopObject()))->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    g.fillRect(x, y, width, height);
}

static void nativeDrawRoundRect(FlintExecution &execution) {
    int32_t arcHeight = execution.stackPopInt32();
    int32_t arcWidth = execution.stackPopInt32();
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    uint32_t color = ((FlintColor *)checkNullObject(execution, execution.stackPopObject()))->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    // TODO
}

static void nativeFillRoundRect(FlintExecution &execution) {
    int32_t arcHeight = execution.stackPopInt32();
    int32_t arcWidth = execution.stackPopInt32();
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    uint32_t color = ((FlintColor *)checkNullObject(execution, execution.stackPopObject()))->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    // TODO
}

static void nativeDrawEllipse(FlintExecution &execution) {
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    uint32_t color = ((FlintColor *)checkNullObject(execution, execution.stackPopObject()))->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    // TODO
}

static void nativeFillEllipse(FlintExecution &execution) {
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    uint32_t color = ((FlintColor *)checkNullObject(execution, execution.stackPopObject()))->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    // TODO
}

static void nativeDrawArc(FlintExecution &execution) {
    int32_t arcAngle = execution.stackPopInt32();
    int32_t startAngle = execution.stackPopInt32();
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    uint32_t color = ((FlintColor *)checkNullObject(execution, execution.stackPopObject()))->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    // TODO
}

static void nativeFillArc(FlintExecution &execution) {
    int32_t arcAngle = execution.stackPopInt32();
    int32_t startAngle = execution.stackPopInt32();
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    uint32_t color = ((FlintColor *)checkNullObject(execution, execution.stackPopObject()))->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    // TODO
}

static void nativeDrawPolyline(FlintExecution &execution) {
    int32_t nPoints = execution.stackPopInt32();
    FlintInt32Array *yPoints = (FlintInt32Array *)checkNullObject(execution, execution.stackPopObject());
    FlintInt32Array *xPoints = (FlintInt32Array *)checkNullObject(execution, execution.stackPopObject());
    uint32_t color = ((FlintColor *)checkNullObject(execution, execution.stackPopObject()))->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    if((nPoints > xPoints->getLength()) || (nPoints > yPoints->getLength()))
        throw &execution.flint.newArrayIndexOutOfBoundsException();
    g.drawPolyline(xPoints->getData(), yPoints->getData(), nPoints);
}

static void nativeDrawPolygon(FlintExecution &execution) {
    int32_t nPoints = execution.stackPopInt32();
    FlintInt32Array *yPoints = (FlintInt32Array *)checkNullObject(execution, execution.stackPopObject());
    FlintInt32Array *xPoints = (FlintInt32Array *)checkNullObject(execution, execution.stackPopObject());
    uint32_t color = ((FlintColor *)checkNullObject(execution, execution.stackPopObject()))->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    if((nPoints > xPoints->getLength()) || (nPoints > yPoints->getLength()))
        throw &execution.flint.newArrayIndexOutOfBoundsException();
    g.drawPolygon(xPoints->getData(), yPoints->getData(), nPoints);
}

static void nativeFillPolygon(FlintExecution &execution) {
    int32_t nPoints = execution.stackPopInt32();
    FlintInt32Array *yPoints = (FlintInt32Array *)checkNullObject(execution, execution.stackPopObject());
    FlintInt32Array *xPoints = (FlintInt32Array *)checkNullObject(execution, execution.stackPopObject());
    uint32_t color = ((FlintColor *)checkNullObject(execution, execution.stackPopObject()))->getValue();
    FlintGraphics g(execution.stackPopObject(), color);
    // TODO
}

static void nativeDrawString(FlintExecution &execution) {
    FlintGraphics g(execution.stackPopObject(), 0);
    // TODO
}

static void nativeDrawImage1(FlintExecution &execution) {
    FlintGraphics g(execution.stackPopObject(), 0);
    // TODO
}

static void nativeDrawImage2(FlintExecution &execution) {
    FlintGraphics g(execution.stackPopObject(), 0);
    // TODO
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x05\x00\xBB\x0D""clear",         "\x03\x00\x91\x99""()V",                                                              nativeClear),
    NATIVE_METHOD("\x08\x00\x3C\x95""drawLine",      "\x1C\x00\xFB\xB2""(Lflint/drawing/Color;IIII)V",                                     nativeDrawLine),
    NATIVE_METHOD("\x08\x00\x3E\x22""drawRect",      "\x1C\x00\xFB\xB2""(Lflint/drawing/Color;IIII)V",                                     nativeDrawRect),
    NATIVE_METHOD("\x08\x00\x71\xE5""fillRect",      "\x1C\x00\xFB\xB2""(Lflint/drawing/Color;IIII)V",                                     nativeFillRect),
    NATIVE_METHOD("\x0D\x00\x54\x7E""drawRoundRect", "\x1E\x00\x8D\x62""(Lflint/drawing/Color;IIIIII)V",                                   nativeDrawRoundRect),
    NATIVE_METHOD("\x0D\x00\x3C\xC4""fillRoundRect", "\x1E\x00\x8D\x62""(Lflint/drawing/Color;IIIIII)V",                                   nativeFillRoundRect),
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
