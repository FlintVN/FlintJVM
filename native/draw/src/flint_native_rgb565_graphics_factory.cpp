
#include <string.h>
#include "flint_array_object.h"
#include "flint_native_rgb565_graphics_factory.h"

typedef class : public JObject {
public:
    jbool getHasAlpha() { return (jbool)getFieldByIndex(0)->getInt32(); }
    jint getWidth() { return getFieldByIndex(1)->getInt32(); }
    jint getHeight() { return getFieldByIndex(2)->getInt32(); }
    jbyteArray getData() { return (jbyteArray)getFieldByIndex(3)->getObj(); }
    jbool getMutable() { return (jbool)getFieldByIndex(4)->getInt32(); }

    void setHasAlpha(jbool val) { getFieldByIndex(0)->setInt32(val); }
    void setWidth(jint val) { getFieldByIndex(1)->setInt32(val); }
    void setHeight(jint val) { getFieldByIndex(2)->setInt32(val); }
    void setData(jbyteArray val) { getFieldByIndex(3)->setObj(val); }
    void setMutable(jbool val) { getFieldByIndex(4)->setInt32(val); }
} *JRgb565Image;

static inline __attribute__((always_inline)) uint16_t colorRgb565(uint32_t rgb888) {
    return __builtin_bswap16(((rgb888 >> 8) & 0xF800) | ((rgb888 >> 5) & 0x07E0) | ((rgb888 >> 3) & 0x1F));
}

jobject NativeRgb565GraphicsFactory_CreateImage1(FNIEnv *env, jobject obj, jintArray rgb, jint w, jint h, jbool processAlpha) {
    if (rgb == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "rgb array cannot be null");
        return NULL;
    }
    if (w <= 0 || h <= 0) {
        env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "width and height must be greater than zero");
        return NULL;
    }
    uint32_t len = w * h;
    if (rgb->getLength() < len) {
        env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "The rgb array is of insufficient size");
        return NULL;
    }
    if (processAlpha) {
        uint32_t aBytes = (len + 1) >> 1;
        jbyteArray d = env->newByteArray(len * 2 + aBytes);
        if (d == NULL) return NULL;
        JRgb565Image img = (JRgb565Image)env->newObject(env->findClass("flint/drawing/Rgb565Image"));
        if (img == NULL) {
            env->freeObject(d);
            return NULL;
        }
        uint16_t *imgData = (uint16_t *)d->getData();
        uint8_t *alphaData = (uint8_t *)&imgData[len];
        uint32_t *rgbData = (uint32_t *)rgb->getData();
        for (uint32_t i = 0; i < len; i++) {
            imgData[i] = colorRgb565(rgbData[i]);
            if (i & 1)
                alphaData[i >> 1] |= (rgbData[i] >> 24) & 0xF0;
            else
                alphaData[i >> 1] |= rgbData[i] >> 28;
        }
        img->setHasAlpha(true);
        img->setWidth(w);
        img->setHeight(h);
        img->setData(d);
        img->setMutable(true);
        return img;
    }
    else {
        jbyteArray d = env->newByteArray(len * 2);
        if (d == NULL) return NULL;
        JRgb565Image img = (JRgb565Image)env->newObject(env->findClass("flint/drawing/Rgb565Image"));
        if (img == NULL) {
            env->freeObject(d);
            return NULL;
        }
        uint16_t *imgData = (uint16_t *)d->getData();
        uint32_t *rgbData = (uint32_t *)rgb->getData();
        for (uint32_t i = 0; i < len; i++)
            imgData[i] = colorRgb565(rgbData[i]);
        img->setHasAlpha(false);
        img->setWidth(w);
        img->setHeight(h);
        img->setData(d);
        img->setMutable(true);
        return img;
    }
}

jobject NativeRgb565GraphicsFactory_CreateImage2(FNIEnv *env, jobject obj, jobject src, jint x, jint y, jint w, jint h, jint transform) {
    if (src == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "src cannot be null");
        return NULL;
    }
    if (transform < 0 || transform > 7) {
        env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "Invalid transform");
        return NULL;
    }
    if (w <= 0 || h <= 0) {
        env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "width and height must be greater than zero");
        return NULL;
    }
    JRgb565Image srcImg = (JRgb565Image)src;
    jbool hasAlpha = srcImg->getHasAlpha();
    jint iw = srcImg->getWidth();
    jint ih = srcImg->getHeight();
    if (x < 0 || y < 0 || x > iw - w || y > ih - h) {
        env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "The region exceeds the bounds of the source image");
        return NULL;
    }

    uint32_t len = (uint32_t)w * (uint32_t)h;
    jbyteArray d = env->newByteArray(hasAlpha ? (len * 2 + ((len + 1) >> 1)) : (len * 2));
    if (d == NULL) return NULL;
    JRgb565Image img = (JRgb565Image)env->newObject(env->findClass("flint/drawing/Rgb565Image"));
    if (img == NULL) {
        env->freeObject(d);
        return NULL;
    }

    const int32_t top = y * iw, bottom = (y + h - 1) * iw;
    const int32_t left = x, right = x + w - 1;
    int32_t base, du, dv;
    switch (transform) {
        case 0:  base = top + left;     du = 1;   dv = iw;  break; // NONE
        case 1:  base = bottom + left;  du = 1;   dv = -iw; break; // MIRROR_ROT180
        case 2:  base = top + right;    du = -1;  dv = iw;  break; // MIRROR
        case 3:  base = bottom + right; du = -1;  dv = -iw; break; // ROT180
        case 4:  base = top + left;     du = iw;  dv = 1;   break; // MIRROR_ROT270
        case 5:  base = bottom + left;  du = -iw; dv = 1;   break; // ROT90
        case 6:  base = top + right;    du = iw;  dv = -1;  break; // ROT270
        default: base = bottom + right; du = -iw; dv = -1;  break; // MIRROR_ROT90
    }
    const int32_t dw = (transform & 4) ? h : w;
    const int32_t dh = (transform & 4) ? w : h;

    const uint16_t *srcData = (const uint16_t *)srcImg->getData()->getData();
    uint16_t *dstData = (uint16_t *)d->getData();
    if (!hasAlpha) {
        for (int32_t py = 0; py < dh; py++) {
            int32_t idx = base + py * dv;
            uint16_t *p = &dstData[py * dw];
            if (du == 1)
                memcpy(p, &srcData[idx], (size_t)dw * 2);
            else for (int32_t px = 0; px < dw; px++, idx += du)
                p[px] = srcData[idx];
        }
    }
    else {
        const uint8_t *srcAlpha = (const uint8_t *)&srcData[(uint32_t)iw * ih];
        uint8_t *dstAlpha = (uint8_t *)&dstData[len];
        uint32_t di = 0;
        for (int32_t py = 0; py < dh; py++) {
            int32_t idx = base + py * dv;
            for (int32_t px = 0; px < dw; px++, idx += du, di++) {
                dstData[di] = srcData[idx];
                uint8_t a = (idx & 1) ? (srcAlpha[idx >> 1] >> 4) : (srcAlpha[idx >> 1] & 0x0F);
                if (di & 1)
                    dstAlpha[di >> 1] |= a << 4;
                else
                    dstAlpha[di >> 1] = a;
            }
        }
    }

    img->setHasAlpha(hasAlpha);
    img->setWidth(dw);
    img->setHeight(dh);
    img->setData(d);
    img->setMutable(true);
    return img;
}
