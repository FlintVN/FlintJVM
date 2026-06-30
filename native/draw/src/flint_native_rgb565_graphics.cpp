
#include "flint_default_conf.h"

#if FLINT_API_DRAW_ENABLED

#include "flint_java_string.h"
#include "flint_array_object.h"
#include "flint_rgb565_gfx.h"
#include "flint_native_rgb565_graphics.h"

typedef class : public JObject {
public:
    int32_t getX() { return getFieldByIndex(0)->getInt32(); }
    int32_t getY() { return getFieldByIndex(1)->getInt32(); }
    int32_t getW() { return getFieldByIndex(2)->getInt32(); }
    int32_t getH() { return getFieldByIndex(3)->getInt32(); }
    int32_t getClipX1() { return getFieldByIndex(4)->getInt32(); }
    int32_t getClipY1() { return getFieldByIndex(5)->getInt32(); }
    int32_t getClipX2() { return getClipX1() + getFieldByIndex(6)->getInt32() - 1; }
    int32_t getClipY2() { return getClipY1() + getFieldByIndex(7)->getInt32() - 1; }
    uint8_t *getData() { return (uint8_t *)((jbyteArray)getFieldByIndex(8)->getObj())->getData(); }
} *JRgb565Gfx;

class Rgb565GfxInitHelper : public Rgb565Gfx {
public:
    Rgb565GfxInitHelper(JRgb565Gfx jgfx) : Rgb565Gfx(
        jgfx->getW(),
        jgfx->getH(),
        jgfx->getClipX1(),
        jgfx->getClipY1(),
        jgfx->getClipX2(),
        jgfx->getClipY2(),
        jgfx->getData()
    ) {

    }
};

static inline __attribute__((always_inline)) uint32_t ColorRgb565(uint32_t rgb888) {
    return __builtin_bswap16(((rgb888 >> 8) & 0xF800) | ((rgb888 >> 5) & 0x07E0) | ((rgb888 >> 3) & 0x1F)) | (rgb888 & 0xFF000000);
}

jvoid NativeRgb565Graphics_Clear1(FNIEnv *env, jobject obj) {
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    g.clear(0);
}

jvoid NativeRgb565Graphics_Clear2(FNIEnv *env, jobject obj, jobject c) {
    if(c == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "color cannot be null");
        return;
    }
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    jint color = ColorRgb565(c->getFieldByIndex(0)->getInt32());
    g.clear(color);
}

jvoid NativeRgb565Graphics_DrawLine(FNIEnv *env, jobject obj, jobject c, jint thk, jint x1, jint y1, jint x2, jint y2) {
    if(c == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "color cannot be null");
        return;
    }
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    jint color = ColorRgb565(c->getFieldByIndex(0)->getInt32());
    jint ox = ((JRgb565Gfx)obj)->getX();
    jint oy = ((JRgb565Gfx)obj)->getY();
    g.drawLine(color, thk, x1 + ox, y1 + oy, x2 + ox, y2 + oy);
}

jvoid NativeRgb565Graphics_DrawRect(FNIEnv *env, jobject obj, jobject c, jint thk, jint x, jint y, jint w, jint h) {
    if(c == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "color cannot be null");
        return;
    }
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    jint color = ColorRgb565(c->getFieldByIndex(0)->getInt32());
    jint ox = ((JRgb565Gfx)obj)->getX();
    jint oy = ((JRgb565Gfx)obj)->getY();
    g.drawRect(color, thk, x + ox, y + oy, w, h);
}

jvoid NativeRgb565Graphics_FillRect(FNIEnv *env, jobject obj, jobject c, jint x, jint y, jint w, jint h) {
    if(c == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "color cannot be null");
        return;
    }
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    jint color = ColorRgb565(c->getFieldByIndex(0)->getInt32());
    jint ox = ((JRgb565Gfx)obj)->getX();
    jint oy = ((JRgb565Gfx)obj)->getY();
    g.fillRect(color, x + ox, y + oy, w, h);
}

jvoid NativeRgb565Graphics_DrawRoundRect(FNIEnv *env, jobject obj, jobject c, jint thk, jint x, jint y, jint w, jint h, jint r1, jint r2, jint r3, jint r4) {
    if(c == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "color cannot be null");
        return;
    }
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    jint color = ColorRgb565(c->getFieldByIndex(0)->getInt32());
    jint ox = ((JRgb565Gfx)obj)->getX();
    jint oy = ((JRgb565Gfx)obj)->getY();
    if(r1 < 0) r1 = 0;
    if(r2 < 0) r2 = 0;
    if(r3 < 0) r3 = 0;
    if(r4 < 0) r4 = 0;
    if(!r1 && !r2 && !r3 && !r4)
        g.fillRect(color, x + ox, y + oy, w, h);
    else
        g.drawRoundRect(color, thk, x + ox, y + oy, w, h, r1, r2, r3, r4);
}

jvoid NativeRgb565Graphics_FillRoundRect(FNIEnv *env, jobject obj, jobject c, jint x, jint y, jint w, jint h, jint r1, jint r2, jint r3, jint r4) {
    if(c == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "color cannot be null");
        return;
    }
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    jint color = ColorRgb565(c->getFieldByIndex(0)->getInt32());
    jint ox = ((JRgb565Gfx)obj)->getX();
    jint oy = ((JRgb565Gfx)obj)->getY();
    if(r1 < 0) r1 = 0;
    if(r2 < 0) r2 = 0;
    if(r3 < 0) r3 = 0;
    if(r4 < 0) r4 = 0;
    if(!r1 && !r2 && !r3 && !r4)
        g.fillRect(color, x + ox, y + oy, w, h);
    else
        g.fillRoundRect(color, x + ox, y + oy, w, h, r1, r2, r3, r4);
}

jvoid NativeRgb565Graphics_DrawEllipse(FNIEnv *env, jobject obj, jobject c, jint thk, jint x, jint y, jint w, jint h) {
    if(c == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "color cannot be null");
        return;
    }
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    jint color = ColorRgb565(c->getFieldByIndex(0)->getInt32());
    jint ox = ((JRgb565Gfx)obj)->getX();
    jint oy = ((JRgb565Gfx)obj)->getY();
    g.drawEllipse(color, thk, x + ox, y + oy, w, h);
}

jvoid NativeRgb565Graphics_FillEllipse(FNIEnv *env, jobject obj, jobject c, jint x, jint y, jint w, jint h) {
    if(c == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "color cannot be null");
        return;
    }
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    jint color = ColorRgb565(c->getFieldByIndex(0)->getInt32());
    jint ox = ((JRgb565Gfx)obj)->getX();
    jint oy = ((JRgb565Gfx)obj)->getY();
    g.fillEllipse(color, x + ox, y + oy, w, h);
}

jvoid NativeRgb565Graphics_DrawArc(FNIEnv *env, jobject obj, jobject c, jint thk, jint x, jint y, jint w, jint h, jfloat startAngle, jfloat sweepAngle) {
    if(c == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "color cannot be null");
        return;
    }
    // TODO
}

jvoid NativeRgb565Graphics_FillArc(FNIEnv *env, jobject obj, jobject c, jint x, jint y, jint w, jint h, jfloat startAngle, jfloat sweepAngle) {
    if(c == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "color cannot be null");
        return;
    }
    // TODO
}

jvoid NativeRgb565Graphics_DrawPolygon(FNIEnv *env, jobject obj, jobject c, jint thk, jobject points) {
    if(c == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "color cannot be null");
        return;
    }
    // TODO
}

jvoid NativeRgb565Graphics_FillPolygon(FNIEnv *env, jobject obj, jobject c, jobject points) {
    if(c == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "color cannot be null");
        return;
    }
    // TODO
}

jvoid NativeRgb565Graphics_DrawString(FNIEnv *env, jobject obj, jstring str, jobject font, jobject c, jint x, jint y) {
    if(str == NULL) return;
    if(font == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "font cannot be null");
        return;
    }
    if(c == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "color cannot be null");
        return;
    }
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    Font *f = (Font *)((jbyteArray)font->getFieldByIndex(0)->getObj())->getData();
    jint color = ColorRgb565(c->getFieldByIndex(0)->getInt32());
    jint ox = ((JRgb565Gfx)obj)->getX();
    jint oy = ((JRgb565Gfx)obj)->getY();
    if(str->getCoder() == 0)
        g.drawLatin1((uint8_t *)str->getAscii(), str->getLength(), f, color, x + ox, y + oy);
    else
        g.drawUTF16((uint8_t *)str->getAscii(), str->getLength(), f, color, x + ox, y + oy);
}

jvoid NativeRgb565Graphics_DrawImage1(FNIEnv *env, jobject obj, jobject img, jint x, jint y) {
    if(img == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "img cannot be null");
        return;
    }
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    jint format = img->getFieldByIndex(0)->getInt32();
    jint imgw = img->getFieldByIndex(1)->getInt32();
    jint imgh = img->getFieldByIndex(2)->getInt32();
    int8_t *data = ((jbyteArray)img->getFieldByIndex(3)->getObj())->getData();
    jint ox = ((JRgb565Gfx)obj)->getX();
    jint oy = ((JRgb565Gfx)obj)->getY();
    Image cimg((ImgFormat)format, imgw, imgh, data);
    g.drawImage(&cimg, x + ox, y + oy);
}

jvoid NativeRgb565Graphics_DrawImage2(FNIEnv *env, jobject obj, jobject img, jint x, jint y, jint w, jint h) {
    if(img == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "img cannot be null");
        return;
    }
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    jint format = img->getFieldByIndex(0)->getInt32();
    jint imgw = img->getFieldByIndex(1)->getInt32();
    jint imgh = img->getFieldByIndex(2)->getInt32();
    int8_t *data = ((jbyteArray)img->getFieldByIndex(3)->getObj())->getData();
    jint ox = ((JRgb565Gfx)obj)->getX();
    jint oy = ((JRgb565Gfx)obj)->getY();
    Image cimg((ImgFormat)format, imgw, imgh, data);
    g.drawImage(&cimg, x + ox, y + oy, w, h);
}

#endif /* FLINT_API_DRAW_ENABLED */
