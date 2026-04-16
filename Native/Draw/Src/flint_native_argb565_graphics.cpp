
#include "flint_default_conf.h"

#if FLINT_API_DRAW_ENABLED

#include "flint_java_string.h"
#include "flint_array_object.h"
#include "flint_rgb565_gfx.h"
#include "flint_native_argb565_graphics.h"

typedef class : public JObject {
public:
    int32_t getOX() { return getFieldByIndex(0)->getInt32(); }
    int32_t getOY() { return getFieldByIndex(1)->getInt32(); }
    int32_t getWidth() { return getFieldByIndex(2)->getInt32(); }
    int32_t getHeight() { return getFieldByIndex(3)->getInt32(); }
    int32_t getClipX1() { return getFieldByIndex(4)->getInt32(); }
    int32_t getClipY1() { return getFieldByIndex(5)->getInt32(); }
    int32_t getClipX2() { return getClipX1() + getFieldByIndex(6)->getInt32() - 1; }
    int32_t getClipY2() { return getClipY1() + getFieldByIndex(7)->getInt32() - 1; }
    uint8_t *getData() { return (uint8_t *)((jbyteArray)getFieldByIndex(8)->getObj())->getData(); }
} *JRgb565Gfx;

class Rgb565GfxInitHelper : public Rgb565Gfx {
public:
    Rgb565GfxInitHelper(JRgb565Gfx jgfx) : Rgb565Gfx(
        jgfx->getWidth(),
        jgfx->getHeight(),
        jgfx->getClipX1(),
        jgfx->getClipY1(),
        jgfx->getClipX2(),
        jgfx->getClipY2(),
        jgfx->getData()
    ) {

    }
};

jvoid NativeRgb565Graphics_Clear(FNIEnv *env, jobject obj, jobject c) {
    if(c == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "color cannot be null");
        return;
    }
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    g.clear(c->getFieldByIndex(0)->getInt32());
}

jvoid NativeRgb565Graphics_DrawLine(FNIEnv *env, jobject obj, jobject pen, jint x1, jint y1, jint x2, jint y2) {
    if(pen == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "pen cannot be null");
        return;
    }
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    jint color = pen->getFieldByIndex(0)->getObj()->getFieldByIndex(0)->getInt32();
    jint thickness = pen->getFieldByIndex(1)->getInt32();
    jint ox = ((JRgb565Gfx)obj)->getOX();
    jint oy = ((JRgb565Gfx)obj)->getOY();
    g.drawLine(color, thickness, x1 + ox, y1 + oy, x2 + ox, y2 + oy);
}

jvoid NativeRgb565Graphics_DrawRect(FNIEnv *env, jobject obj, jobject pen, jint x, jint y, jint w, jint h) {
    if(pen == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "pen cannot be null");
        return;
    }
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    jint color = pen->getFieldByIndex(0)->getObj()->getFieldByIndex(0)->getInt32();
    jint thickness = pen->getFieldByIndex(1)->getInt32();
    jint ox = ((JRgb565Gfx)obj)->getOX();
    jint oy = ((JRgb565Gfx)obj)->getOY();
    g.drawRect(color, thickness, x + ox, y + oy, w, h);
}

jvoid NativeRgb565Graphics_FillRect(FNIEnv *env, jobject obj, jobject c, jint x, jint y, jint w, jint h) {
    if(c == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "color cannot be null");
        return;
    }
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    jint ox = ((JRgb565Gfx)obj)->getOX();
    jint oy = ((JRgb565Gfx)obj)->getOY();
    g.fillRect(c->getFieldByIndex(0)->getInt32(), x + ox, y + oy, w, h);
}

jvoid NativeRgb565Graphics_DrawRoundRect(FNIEnv *env, jobject obj, jobject pen, jint x, jint y, jint w, jint h, jint r1, jint r2, jint r3, jint r4) {
    if(pen == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "pen cannot be null");
        return;
    }
    // TODO
}

jvoid NativeRgb565Graphics_FillRoundRect(FNIEnv *env, jobject obj, jobject c, jint x, jint y, jint w, jint h, jint r1, jint r2, jint r3, jint r4) {
    if(c == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "color cannot be null");
        return;
    }
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    jint color = c->getFieldByIndex(0)->getInt32();
    jint ox = ((JRgb565Gfx)obj)->getOX();
    jint oy = ((JRgb565Gfx)obj)->getOY();
    g.fillRoundRect(color, x + ox, y + oy, w, h, r1, r2, r3, r4);
}

jvoid NativeRgb565Graphics_DrawEllipse(FNIEnv *env, jobject obj, jobject pen, jint x, jint y, jint w, jint h) {
    if(pen == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "pen cannot be null");
        return;
    }
    // TODO
}

jvoid NativeRgb565Graphics_FillEllipse(FNIEnv *env, jobject obj, jobject c, jint x, jint y, jint w, jint h) {
    if(c == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "color cannot be null");
        return;
    }
    Rgb565GfxInitHelper g((JRgb565Gfx)obj);
    jint ox = ((JRgb565Gfx)obj)->getOX();
    jint oy = ((JRgb565Gfx)obj)->getOY();
    g.fillEllipse(c->getFieldByIndex(0)->getInt32(), x + ox, y + oy, w, h);
}

jvoid NativeRgb565Graphics_DrawArc(FNIEnv *env, jobject obj, jobject pen, jint x, jint y, jint w, jint h, jfloat startAngle, jfloat sweepAngle) {
    if(pen == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "pen cannot be null");
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

jvoid NativeRgb565Graphics_DrawPolygon(FNIEnv *env, jobject obj, jobject pen, jobject points) {
    if(pen == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "pen cannot be null");
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
    jint color = c->getFieldByIndex(0)->getInt32();
    jint ox = ((JRgb565Gfx)obj)->getOX();
    jint oy = ((JRgb565Gfx)obj)->getOY();
    if(str->getCoder() == 0)
        g.drawLatin1((uint8_t *)str->getAscii(), str->getLength(), f, color, x + ox, y + oy);
    else
        g.drawUTF16((uint8_t *)str->getAscii(), str->getLength(), f, color, x + ox, y + oy);
}

jvoid NativeRgb565Graphics_DrawImage(FNIEnv *env, jobject obj, jobject img, jint x, jint y, jint w, jint h) {
    if(img == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "img cannot be null");
        return;
    }
    // TODO
}

#endif /* FLINT_API_DRAW_ENABLED */
