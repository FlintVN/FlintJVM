
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

jobject NativeRgb565GraphicsFactory_CreateImage(FNIEnv *env, jobject obj, jintArray rgb, jint w, jint h, jbool processAlpha) {
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
