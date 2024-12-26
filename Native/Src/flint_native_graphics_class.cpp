
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

static uint32_t convertColor(uint32_t color, uint8_t colorMode) {
    switch (colorMode) {
        case COLOR_MODE_RGB444:
            return ((color >> 4) & 0x0F) | ((color >> 8) & 0xF0) | ((color >> 12) & 0x0F00);
        case COLOR_MODE_RGB555:
            color = ((color >> 3) & 0x1F) | ((color >> 6) & 0x03E0) | ((color >> 9) & 0x7C00);
            return (color << 8) | (color >> 8);
        case COLOR_MODE_RGB565:
            color = ((color >> 3) & 0x1F) | ((color >> 5) & 0x07E0) | ((color >> 8) & 0xF800);
            return (color << 8) | (color >> 8);
        case COLOR_MODE_BGR565:
            color = ((color << 8) & 0xF800) | ((color >> 5) & 0x07E0) | ((color >> 19) & 0x1F);
            return (color << 8) | (color >> 8);
        default:
            return color;
    }
}

class FlintGraphics {
public:
    uint8_t alpha;
    uint8_t colorMode;
    uint8_t *colorBuffer;
    int16_t x;
    int16_t y;
    uint16_t width;
    int16_t clipX;
    int16_t clipY;
    int16_t clipWidth;
    int16_t clipHeight;
    uint32_t rgb;

    FlintGraphics(FlintObject *g, uint32_t color) {
        FlintInt8Array *buffer = (FlintInt8Array *)g->getFields().getFieldObject(*(const FlintConstUtf8 *)"\x0B\x00\x05\xB6""colorBuffer").object;

        colorMode = (uint8_t)g->getFields().getFieldData32(*(const FlintConstUtf8 *)"\x09\x00\x5F\x5D""colorMode").value;
        colorBuffer = (uint8_t *)buffer->getData();
        x = g->getFields().getFieldData32(*(const FlintConstUtf8 *)"\x01\x00\x40\x9D""x").value;
        y = g->getFields().getFieldData32(*(const FlintConstUtf8 *)"\x01\x00\x81\x5D""y").value;
        width = g->getFields().getFieldData32(*(const FlintConstUtf8 *)"\x05\x00\x55\x59""width").value;
        clipX = g->getFields().getFieldData32(*(const FlintConstUtf8 *)"\x05\x00\xF6\x81""clipX").value - x;
        clipY = g->getFields().getFieldData32(*(const FlintConstUtf8 *)"\x05\x00\x37\x41""clipY").value - y;
        clipWidth = g->getFields().getFieldData32(*(const FlintConstUtf8 *)"\x09\x00\xB2\xD7""clipWidth").value;
        clipHeight = g->getFields().getFieldData32(*(const FlintConstUtf8 *)"\x0A\x00\xF6\xEA""clipHeight").value;

        alpha = color >> 24;
        rgb = convertColor(color, colorMode);
    }

    uint8_t getPixelSize(void) const {
        return (colorMode == COLOR_MODE_RGB888) ? 3 : 2;
    }
};

class FlintColor : public FlintObject {
public:
    uint32_t getValue(void) const {
        return getFields().getFieldData32(*(const FlintConstUtf8 *)"\x05\x00\x2B\x6E""value").value;
    }
};

static void sortSwap(int32_t *a1, int32_t *a2) {
    if(*a1 > *a2) {
        int32_t tmp = *a1;
        *a1 = *a2;
        *a2 = tmp;
    }
}

static void fillRect(const FlintGraphics &g, int32_t x1, int32_t x2, int32_t y1, int32_t y2) {
    if(g.alpha == 0xFF) {
        if(g.getPixelSize() == 2) {
            uint16_t rgb = g.rgb;
            for(; y1 <= y2; y1++) {
                uint16_t *buff = &((uint16_t *)g.colorBuffer)[y1 * g.width];
                for(int32_t x = x1; x <= x2; x++)
                    buff[x] = rgb;
            }
        }
        else {
            uint8_t a = (uint8_t)g.rgb;
            uint8_t b = (uint8_t)(g.rgb >> 8);
            uint8_t c = (uint8_t)(g.rgb >> 16);
            for(; y1 <= y2; y1++) {
                uint8_t *buff = (uint8_t *)&g.colorBuffer[y1 * g.width * 3];
                for(int32_t x = x1; x <= x2; x += 3) {
                    buff[x] = a;
                    buff[x + 1] = b;
                    buff[x + 2] = c;
                }
            }
        }
    }
    else {
        // TODO
    }
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
    uint32_t color = ((FlintColor *)execution.stackPopObject())->getValue();
    const FlintGraphics g(execution.stackPopObject(), color);

    x1 -= g.x;
    x2 -= g.x;
    y1 -= g.y;
    y2 -= g.y;

    if(x1 == x2) {
        if((x1 < g.clipX) || (x1 >= g.clipWidth))
            return;
        sortSwap(&y1, &y2);
        y1 = FLINT_MAX(y1, g.clipY);
        y2 = FLINT_MIN(y2, g.clipHeight - 1);
        if(y1 <= y2)
            fillRect(g, x1, x2, y1, y2);
    }
    else if(y1 == y2) {
        if((y1 < g.clipY) || (y1 >= g.clipHeight))
            return;
        sortSwap(&x1, &x2);
        x1 = FLINT_MAX(x1, g.clipX);
        x2 = FLINT_MIN(x2, g.clipWidth - 1);
        if(x1 <= x2)
            fillRect(g, x1, x2, y1, y2);
    }
    else {
        // TODO
    }
}

static void nativeDrawRect(FlintExecution &execution) {
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    int32_t color = ((FlintColor *)execution.stackPopObject())->getValue();
    const FlintGraphics g(execution.stackPopObject(), color);

    x -= g.x;
    y -= g.y;

    int32_t x1 = FLINT_MAX(x, g.clipX);
    int32_t x2 = FLINT_MIN(x + width, g.clipX + g.clipWidth) - 1;
    int32_t y1 = FLINT_MAX(y, g.clipY);
    int32_t y2 = FLINT_MIN(y + height, g.clipY + g.clipHeight) - 1;

    if(y1 <= y2) {
        if((x1 <= x) && (x <= x2))
            fillRect(g, x, x, y1, y2);
        if((x2 >= g.clipX) && x2 == (x + width - 1))
            fillRect(g, x2, x2, y1, y2);
    }
    if(x1 <= x2) {
        if((y1 <= y) && (y <= y2))
            fillRect(g, x1, x2, y, y);
        if((y2 >= g.clipY) && y2 == (y + height - 1))
            fillRect(g, x1, x2, y2, y2);
    }
}

static void nativeFillRect(FlintExecution &execution) {
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    int32_t color = ((FlintColor *)execution.stackPopObject())->getValue();
    const FlintGraphics g(execution.stackPopObject(), color);

    x -= g.x;
    y -= g.y;

    int32_t x1 = FLINT_MAX(x, g.clipX);
    int32_t x2 = FLINT_MIN(x + width, g.clipX + g.clipWidth) - 1;
    if(x1 > x2)
        return;
    int32_t y1 = FLINT_MAX(y, g.clipY);
    int32_t y2 = FLINT_MIN(y + height, g.clipY + g.clipHeight) - 1;
    if(y1 > y2)
        return;

    fillRect(g, x1, x2, y1, y2);
}

static void nativeDrawRoundRect(FlintExecution &execution) {
    int32_t arcHeight = execution.stackPopInt32();
    int32_t arcWidth = execution.stackPopInt32();
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    int32_t color = ((FlintColor *)execution.stackPopObject())->getValue();
    const FlintGraphics g(execution.stackPopObject(), color);
    // TODO
}

static void nativeFillRoundRect(FlintExecution &execution) {
    int32_t arcHeight = execution.stackPopInt32();
    int32_t arcWidth = execution.stackPopInt32();
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    int32_t color = ((FlintColor *)execution.stackPopObject())->getValue();
    const FlintGraphics g(execution.stackPopObject(), color);
    // TODO
}

static void nativeDrawEllipse(FlintExecution &execution) {
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    int32_t color = ((FlintColor *)execution.stackPopObject())->getValue();
    const FlintGraphics g(execution.stackPopObject(), color);
    // TODO
}

static void nativeFillEllipse(FlintExecution &execution) {
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    int32_t color = ((FlintColor *)execution.stackPopObject())->getValue();
    const FlintGraphics g(execution.stackPopObject(), color);
    // TODO
}

static void nativeDrawArc(FlintExecution &execution) {
    int32_t arcAngle = execution.stackPopInt32();
    int32_t startAngle = execution.stackPopInt32();
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    int32_t color = ((FlintColor *)execution.stackPopObject())->getValue();
    const FlintGraphics g(execution.stackPopObject(), color);
    // TODO
}

static void nativeFillArc(FlintExecution &execution) {
    int32_t arcAngle = execution.stackPopInt32();
    int32_t startAngle = execution.stackPopInt32();
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    int32_t color = ((FlintColor *)execution.stackPopObject())->getValue();
    const FlintGraphics g(execution.stackPopObject(), color);
    // TODO
}

static void nativeDrawPolyline(FlintExecution &execution) {
    int32_t nPoints = execution.stackPopInt32();
    FlintInt32Array *yPoints = (FlintInt32Array *)execution.stackPopObject();
    FlintInt32Array *xPoints = (FlintInt32Array *)execution.stackPopObject();
    int32_t color = ((FlintColor *)execution.stackPopObject())->getValue();
    const FlintGraphics g(execution.stackPopObject(), color);
    // TODO
}

static void nativeDrawPolygon(FlintExecution &execution) {
    int32_t nPoints = execution.stackPopInt32();
    FlintInt32Array *yPoints = (FlintInt32Array *)execution.stackPopObject();
    FlintInt32Array *xPoints = (FlintInt32Array *)execution.stackPopObject();
    int32_t color = ((FlintColor *)execution.stackPopObject())->getValue();
    const FlintGraphics g(execution.stackPopObject(), color);
    // TODO
}

static void nativeFillPolygon(FlintExecution &execution) {
    int32_t nPoints = execution.stackPopInt32();
    FlintInt32Array *yPoints = (FlintInt32Array *)execution.stackPopObject();
    FlintInt32Array *xPoints = (FlintInt32Array *)execution.stackPopObject();
    int32_t color = ((FlintColor *)execution.stackPopObject())->getValue();
    const FlintGraphics g(execution.stackPopObject(), color);
    // TODO
}

static void nativeDrawString(FlintExecution &execution) {
    const FlintGraphics g(execution.stackPopObject(), 0);
    // TODO
}

static void nativeDrawImage1(FlintExecution &execution) {
    const FlintGraphics g(execution.stackPopObject(), 0);
    // TODO
}

static void nativeDrawImage2(FlintExecution &execution) {
    const FlintGraphics g(execution.stackPopObject(), 0);
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
