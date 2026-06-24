
#include "flint_default_conf.h"

#if FLINT_API_DRAW_ENABLED

#include "flint_gfx_common.h"
#include "flint_java_object.h"
#include "flint_java_string.h"
#include "flint_native_graphics.h"

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
            uint8_t h = c->getHeight() + c->getYOffset();
            if(h > txtH) txtH = h;
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
            uint8_t h = c->getHeight() + c->getYOffset();
            if(h > txtH) txtH = h;
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
