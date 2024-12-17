
#include "flint.h"
#include "flint_object.h"
#include "flint_const_name.h"
#include "flint_native_graphics_class.h"

class FlintGraphics : public FlintObject {
public:
    int32_t getColorMode(void) const {
        return getFields().getFieldData32(*(const FlintConstUtf8 *)"\x09\x00\x5F\x5D""colorMode").value;
    }

    FlintInt8Array *getColorBuffer(void) const {
        return (FlintInt8Array *)getFields().getFieldObject(*(const FlintConstUtf8 *)"\x0B\x00\x05\xB6""colorBuffer").object;
    }

    int32_t getX() const {
        return getFields().getFieldData32(*(const FlintConstUtf8 *)"\x01\x00\x40\x9D""x").value;
    }

    int32_t getY() const {
        return getFields().getFieldData32(*(const FlintConstUtf8 *)"\x01\x00\x81\x5D""y").value;
    }

    int32_t getWidth() const {
        return getFields().getFieldData32(*(const FlintConstUtf8 *)"\x05\x00\x55\x59""width").value;
    }

    int32_t getClipX() const {
        return getFields().getFieldData32(*(const FlintConstUtf8 *)"\x05\x00\xF6\x81""clipX").value;
    }

    int32_t getClipY() const {
        return getFields().getFieldData32(*(const FlintConstUtf8 *)"\x05\x00\x37\x41""clipY").value;
    }

    int32_t getClipWidth() const {
        return getFields().getFieldData32(*(const FlintConstUtf8 *)"\x09\x00\xB2\xD7""clipWidth").value;
    }

    int32_t getClipHeight() const {
        return getFields().getFieldData32(*(const FlintConstUtf8 *)"\x0A\x00\xF6\xEA""clipHeight").value;
    }
};

class FlintColor : public FlintObject {
public:
    int32_t getValue(void) const {
        return getFields().getFieldData32(*(const FlintConstUtf8 *)"\x05\x00\x2B\x6E""value").value;
    }
};

class FlintPen : public FlintObject {
public:
    const FlintColor *getColor() const {
        return (const FlintColor *)getFields().getFieldObject(*(const FlintConstUtf8 *)"\x05\x00\x6F\x2B""color").object;
    }

    int32_t getWidth() const {
        return getFields().getFieldData32(*(const FlintConstUtf8 *)"\x05\x00\x55\x59""width").value;
    }
};

static void nativeDrawLine(FlintExecution &execution) {
    int32_t y2 = execution.stackPopInt32();
    int32_t x2 = execution.stackPopInt32();
    int32_t y1 = execution.stackPopInt32();
    int32_t x1 = execution.stackPopInt32();
    FlintPen *pen = (FlintPen *)execution.stackPopObject();
    FlintGraphics *g = (FlintGraphics *)execution.stackPopObject();
    // TODO
}

static void nativeDrawRect(FlintExecution &execution) {
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    FlintPen *pen = (FlintPen *)execution.stackPopObject();
    FlintGraphics *g = (FlintGraphics *)execution.stackPopObject();
    // TODO
}

static void nativeFillRect(FlintExecution &execution) {
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    FlintColor *color = (FlintColor *)execution.stackPopObject();
    FlintGraphics *g = (FlintGraphics *)execution.stackPopObject();
    // TODO
}

static void nativeDrawRoundRect(FlintExecution &execution) {
    int32_t arcHeight = execution.stackPopInt32();
    int32_t arcWidth = execution.stackPopInt32();
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    FlintPen *pen = (FlintPen *)execution.stackPopObject();
    FlintGraphics *g = (FlintGraphics *)execution.stackPopObject();
    // TODO
}

static void nativeFillRoundRect(FlintExecution &execution) {
    int32_t arcHeight = execution.stackPopInt32();
    int32_t arcWidth = execution.stackPopInt32();
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    FlintColor *color = (FlintColor *)execution.stackPopObject();
    FlintGraphics *g = (FlintGraphics *)execution.stackPopObject();
    // TODO
}

static void nativeDrawEllipse(FlintExecution &execution) {
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    FlintPen *pen = (FlintPen *)execution.stackPopObject();
    FlintGraphics *g = (FlintGraphics *)execution.stackPopObject();
    // TODO
}

static void nativeFillEllipse(FlintExecution &execution) {
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    FlintColor *color = (FlintColor *)execution.stackPopObject();
    FlintGraphics *g = (FlintGraphics *)execution.stackPopObject();
    // TODO
}

static void nativeDrawArc(FlintExecution &execution) {
    int32_t arcAngle = execution.stackPopInt32();
    int32_t startAngle = execution.stackPopInt32();
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    FlintPen *pen = (FlintPen *)execution.stackPopObject();
    FlintGraphics *g = (FlintGraphics *)execution.stackPopObject();
    // TODO
}

static void nativeFillArc(FlintExecution &execution) {
    int32_t arcAngle = execution.stackPopInt32();
    int32_t startAngle = execution.stackPopInt32();
    int32_t height = execution.stackPopInt32();
    int32_t width = execution.stackPopInt32();
    int32_t y = execution.stackPopInt32();
    int32_t x = execution.stackPopInt32();
    FlintColor *color = (FlintColor *)execution.stackPopObject();
    FlintGraphics *g = (FlintGraphics *)execution.stackPopObject();
    // TODO
}

static void nativeDrawPolyline(FlintExecution &execution) {
    int32_t nPoints = execution.stackPopInt32();
    FlintInt32Array *yPoints = (FlintInt32Array *)execution.stackPopObject();
    FlintInt32Array *xPoints = (FlintInt32Array *)execution.stackPopObject();
    FlintPen *pen = (FlintPen *)execution.stackPopObject();
    FlintGraphics *g = (FlintGraphics *)execution.stackPopObject();
    // TODO
}

static void nativeDrawPolygon(FlintExecution &execution) {
    int32_t nPoints = execution.stackPopInt32();
    FlintInt32Array *yPoints = (FlintInt32Array *)execution.stackPopObject();
    FlintInt32Array *xPoints = (FlintInt32Array *)execution.stackPopObject();
    FlintPen *pen = (FlintPen *)execution.stackPopObject();
    FlintGraphics *g = (FlintGraphics *)execution.stackPopObject();
    // TODO
}

static void nativeFillPolygon(FlintExecution &execution) {
    int32_t nPoints = execution.stackPopInt32();
    FlintInt32Array *yPoints = (FlintInt32Array *)execution.stackPopObject();
    FlintInt32Array *xPoints = (FlintInt32Array *)execution.stackPopObject();
    FlintColor *color = (FlintColor *)execution.stackPopObject();
    FlintGraphics *g = (FlintGraphics *)execution.stackPopObject();
    // TODO
}

static void nativeDrawString(FlintExecution &execution) {
    FlintGraphics *g = (FlintGraphics *)execution.stackPopObject();
    // TODO
}

static void nativeDrawImage1(FlintExecution &execution) {
    FlintGraphics *g = (FlintGraphics *)execution.stackPopObject();
    // TODO
}

static void nativeDrawImage2(FlintExecution &execution) {
    FlintGraphics *g = (FlintGraphics *)execution.stackPopObject();
    // TODO
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x08\x00\x3C\x95""drawLine",      "\x1A\x00\x96\xAB""(Lflint/drawing/Pen;IIII)V",                                       nativeDrawLine),
    NATIVE_METHOD("\x08\x00\x3E\x22""drawRect",      "\x1A\x00\x96\xAB""(Lflint/drawing/Pen;IIII)V",                                       nativeDrawRect),
    NATIVE_METHOD("\x08\x00\x71\xE5""fillRect",      "\x1C\x00\xFB\xB2""(Lflint/drawing/Color;IIII)V",                                     nativeFillRect),
    NATIVE_METHOD("\x0D\x00\x54\x7E""drawRoundRect", "\x1C\x00\x60\x38""(Lflint/drawing/Pen;IIIIII)V",                                     nativeDrawRoundRect),
    NATIVE_METHOD("\x0D\x00\x3C\xC4""fillRoundRect", "\x1E\x00\x8D\x62""(Lflint/drawing/Color;IIIIII)V",                                   nativeFillRoundRect),
    NATIVE_METHOD("\x0B\x00\x26\x66""drawEllipse",   "\x1A\x00\x96\xAB""(Lflint/drawing/Pen;IIII)V",                                       nativeDrawEllipse),
    NATIVE_METHOD("\x0B\x00\x45\x81""fillEllipse",   "\x1C\x00\xFB\xB2""(Lflint/drawing/Color;IIII)V",                                     nativeFillEllipse),
    NATIVE_METHOD("\x07\x00\x59\x0A""drawArc",       "\x1C\x00\x60\x38""(Lflint/drawing/Pen;IIIIII)V",                                     nativeDrawArc),
    NATIVE_METHOD("\x07\x00\x52\x04""fillArc",       "\x1E\x00\x8D\x62""(Lflint/drawing/Color;IIIIII)V",                                   nativeFillArc),
    NATIVE_METHOD("\x0C\x00\x75\xD9""drawPolyline",  "\x1B\x00\x41\xC3""(Lflint/drawing/Pen;[I[II)V",                                      nativeDrawPolyline),
    NATIVE_METHOD("\x0B\x00\x9F\x57""drawPolygon",   "\x1B\x00\x41\xC3""(Lflint/drawing/Pen;[I[II)V",                                      nativeDrawPolygon),
    NATIVE_METHOD("\x0B\x00\xFC\xB0""fillPolygon",   "\x1D\x00\x99\x2E""(Lflint/drawing/Color;[I[II)V",                                    nativeFillPolygon),
    NATIVE_METHOD("\x0A\x00\x6B\x54""drawString",    "\x40\x00\x21\x24""(Ljava/lang/String;Lflint/drawing/Font;Lflint/drawing/Color;II)V", nativeDrawString),
    NATIVE_METHOD("\x09\x00\xE9\xD6""drawImage",     "\x1A\x00\xC3\x33""(Lflint/drawing/Image;II)V",                                       nativeDrawImage1),
    NATIVE_METHOD("\x09\x00\xE9\xD6""drawImage",     "\x1C\x00\x5E\xC2""(Lflint/drawing/Image;IIII)V",                                     nativeDrawImage2),
};

const FlintNativeClass GRAPHICS_CLASS = NATIVE_CLASS(flintGraphicsClassName, methods);
