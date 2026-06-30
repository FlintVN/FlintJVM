
#include "flint_default_conf.h"

#if FLINT_API_DRAW_ENABLED

#include "flint_gfx_common.h"
#include "flint_java_object.h"
#include "flint_java_string.h"
#include "flint_native_graphics.h"

typedef class : public JObject {
public:
    int32_t getX() { return getFieldByIndex(0)->getInt32(); }
    int32_t getY() { return getFieldByIndex(1)->getInt32(); }
    int32_t getW() { return getFieldByIndex(2)->getInt32(); }
    int32_t getH() { return getFieldByIndex(3)->getInt32(); }
    int32_t getClipX() { return getFieldByIndex(4)->getInt32(); }
    int32_t getClipY() { return getFieldByIndex(5)->getInt32(); }
    int32_t getClipW() { return getFieldByIndex(6)->getInt32(); }
    int32_t getClipH() { return getFieldByIndex(7)->getInt32(); }

    void setX(int32_t x) { getFieldByIndex(0)->setInt32(x); }
    void setY(int32_t y) { getFieldByIndex(1)->setInt32(y); }
    void setW(int32_t w) { getFieldByIndex(2)->setInt32(w); }
    void setH(int32_t h) { getFieldByIndex(3)->setInt32(h); }
    void setClipX(int32_t x) { getFieldByIndex(4)->setInt32(x); }
    void setClipY(int32_t y) { getFieldByIndex(5)->setInt32(y); }
    void setClipW(int32_t w) { getFieldByIndex(6)->setInt32(w); }
    void setClipH(int32_t h) { getFieldByIndex(7)->setInt32(h); }
} *JGfx;

static void MeasureStringLatin1(uint8_t *str, uint32_t len, Font *font, uint32_t *width, uint32_t *height) {
    uint8_t stdHeight = font->getStdHeight();

    if(len == 0) {
        *width = 0;
        *height = stdHeight;
        return;
    }

    uint8_t stdWidth = font->getStdWidth();
    uint8_t space = font->getSpaceWidth();
    space = GFX_MAX(space / 4, 1);
    uint32_t txtW = 0;
    uint32_t txtH = 0;

    while(len--) {
        const CharInfo *c = font->getChar(*str++);
        if(c != NULL) {
            txtW += c->getWidth() + space;
            int32_t h = c->getHeight() + c->getYOffset();
            if(h > (int32_t)txtH) txtH = h;
        }
        else {
            txtW += stdWidth + space;
            if(stdHeight > txtH) txtH = stdHeight;
        }
    }
    if(txtW > space) txtW -= space;

    *width = txtW;
    *height = txtH;
}

static void MeasureStringUTF16(uint8_t *str, uint32_t len, Font *font, uint32_t *width, uint32_t *height) {
    uint8_t stdHeight = font->getStdHeight();

    if(len == 0) {
        *width = 0;
        *height = stdHeight;
        return;
    }

    uint8_t stdWidth = font->getStdWidth();
    uint8_t space = font->getSpaceWidth();
    space = GFX_MAX(space / 4, 1);
    uint32_t txtW = 0;
    uint32_t txtH = 0;

    while(len--) {
        uint16_t unicode = str[0] | (str[1] << 8);
        const CharInfo *c = font->getChar(unicode);
        if(c != NULL) {
            txtW += c->getWidth() + space;
            int32_t h = c->getHeight() + c->getYOffset();
            if(h > (int32_t)txtH) txtH = h;
        }
        else {
            txtW += stdWidth + space;
            if(stdHeight > txtH) txtH = stdHeight;
        }
        str += 2;
    }
    if(txtW > space) txtW -= space;

    *width = txtW;
    *height = txtH;
}

jbool NativeGraphics_IsVisible(FNIEnv *env, jobject obj, jint x, jint y, jint w, jint h) {
    int32_t clipX1 = ((JGfx)obj)->getClipX();
    if(x + w <= clipX1) return false;
    int32_t clipY1 = ((JGfx)obj)->getClipY();
    if(y + h <= clipY1) return false;
    int32_t clipX2 = clipX1 + ((JGfx)obj)->getClipW();
    if(x >= clipX2) return false;
    int32_t clipY2 = clipY1 + ((JGfx)obj)->getClipH();
    return y < clipY2;
}

jvoid NativeGraphics_SetClip0(FNIEnv *env, jobject obj, jint x, jint y, jint w, jint h, jint mode) {
    if(w < 0 || h < 0) {
        env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "width and height cannot be negative");
        return;
    }
    JGfx g = ((JGfx)obj);
    if(mode == 0) {     /* REPLACE mode */
        int32_t xend = x + w;
        int32_t yend = y + h;
        g->setClipX(GFX_MAX(x, 0));
        g->setClipY(GFX_MAX(y, 0));
        g->setClipW(GFX_MIN(xend, g->getW()) - g->getClipX());
        g->setClipH(GFX_MIN(yend, g->getH()) - g->getClipY());
    }
    else {
        x += g->getX();
        y += g->getY();
        int32_t xend1 = g->getClipX() + g->getClipW();
        int32_t yend1 = g->getClipY() + g->getClipH();
        int32_t xend2 = x + w;
        int32_t yend2 = y + h;
        g->setClipX(GFX_MAX(x, g->getClipX()));
        g->setClipY(GFX_MAX(y, g->getClipY()));
        g->setClipW(GFX_MIN(xend2, xend1) - g->getClipX());
        g->setClipH(GFX_MIN(yend2, yend1) - g->getClipY());
    }
}

jobject NativeGraphics_MeasureString(FNIEnv *env, jstring str, jobject font) {
    uint32_t w = 0, h = 0;
    Font *f = (Font *)((jbyteArray)font->getFieldByIndex(0)->getObj())->getData();

    if(str == NULL)
        MeasureStringLatin1(NULL, 0, f, &w, &h);
    else {
        uint8_t coder = str->getCoder();
        uint8_t *txt = (uint8_t *)str->getAscii();
        uint32_t len = str->getLength();
        if(coder == 0)
            MeasureStringLatin1(txt, len, f, &w, &h);
        else
            MeasureStringUTF16(txt, len, f, &w, &h);
    }

    jobject size = env->newObject(env->findClass("flint/drawing/Size"));
    if(size != NULL) {
        size->getFieldByIndex(0)->setInt32(w);
        size->getFieldByIndex(1)->setInt32(h);
    }
    return size;
}

jint NativeGraphics_MeasureStringWidth(FNIEnv *env, jstring str, jobject font) {
    uint32_t w = 0, h = 0;
    Font *f = (Font *)((jbyteArray)font->getFieldByIndex(0)->getObj())->getData();

    if(str == NULL)
        MeasureStringLatin1(NULL, 0, f, &w, &h);
    else {
        uint8_t coder = str->getCoder();
        uint8_t *txt = (uint8_t *)str->getAscii();
        uint32_t len = str->getLength();
        if(coder == 0)
            MeasureStringLatin1(txt, len, f, &w, &h);
        else
            MeasureStringUTF16(txt, len, f, &w, &h);
    }
    return w;
}

jint NativeGraphics_MeasureStringHeight(FNIEnv *env, jstring str, jobject font) {
    uint32_t w = 0, h = 0;
    Font *f = (Font *)((jbyteArray)font->getFieldByIndex(0)->getObj())->getData();

    if(str == NULL)
        MeasureStringLatin1(NULL, 0, f, &w, &h);
    else {
        uint8_t coder = str->getCoder();
        uint8_t *txt = (uint8_t *)str->getAscii();
        uint32_t len = str->getLength();
        if(coder == 0)
            MeasureStringLatin1(txt, len, f, &w, &h);
        else
            MeasureStringUTF16(txt, len, f, &w, &h);
    }
    return h;
}

#endif /* FLINT_API_DRAW_ENABLED */
