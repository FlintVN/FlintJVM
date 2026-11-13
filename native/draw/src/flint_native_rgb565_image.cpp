
#include "flint_array_object.h"
#include "flint_native_rgb565_image.h"

typedef class : public JObject {
public:
    jbool getHasAlpha() { return (jbool)getFieldByIndex(0)->getInt32(); }
    jint getWidth() { return getFieldByIndex(1)->getInt32(); }
    jint getHeight() { return getFieldByIndex(2)->getInt32(); }
    jbyteArray getData() { return (jbyteArray)getFieldByIndex(3)->getObj(); }
} *JRgb565Image;

static inline __attribute__((always_inline)) uint32_t colorRgb888(uint16_t rgb565) {
    uint32_t c = __builtin_bswap16(rgb565);
    uint32_t r = (c >> 11) & 0x1F;
    uint32_t g = (c >> 5) & 0x3F;
    uint32_t b = c & 0x1F;
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);
    return (r << 16) | (g << 8) | b;
}

jvoid NativeRgb565Image_GetRgb(FNIEnv *env, jobject obj, jintArray rgb, jint offset, jint scanlength, jint x, jint y, jint w, jint h) {
    if (rgb == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "rgb array cannot be null");
        return;
    }
    if (w <= 0 || h <= 0) return;

    JRgb565Image img = (JRgb565Image)obj;
    jint imgW = img->getWidth();
    jint imgH = img->getHeight();
    if (x < 0 || y < 0 || x > imgW - w || y > imgH - h) {
        env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "The region to be retrieved exceeds the bounds of the image");
        return;
    }
    if ((scanlength < 0 ? -(int64_t)scanlength : (int64_t)scanlength) < w) {
        env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "The absolute value of scanlength is less than width");
        return;
    }

    int64_t lastRow = (int64_t)(h - 1) * scanlength;
    int64_t minIdx = (int64_t)offset + (lastRow < 0 ? lastRow : 0);
    int64_t maxIdx = (int64_t)offset + (lastRow > 0 ? lastRow : 0) + w - 1;
    if (minIdx < 0 || maxIdx >= (int64_t)rgb->getLength()) {
        env->throwNew(env->findClass("java/lang/ArrayIndexOutOfBoundsException"), "The rgb array is of insufficient size");
        return;
    }

    jbyteArray data = img->getData();
    if (data == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "Image data cannot be null");
        return;
    }
    bool hasAlpha = img->getHasAlpha();
    uint32_t pixels = (uint32_t)imgW * (uint32_t)imgH;
    uint32_t size = pixels * 2 + (hasAlpha ? ((pixels + 1) >> 1) : 0);
    if (data->getLength() < size) {
        env->throwNew(env->findClass("java/lang/IllegalStateException"), "Image data is too small");
        return;
    }

    const uint16_t *srcPixels = (const uint16_t *)data->getData();
    uint32_t *desData = (uint32_t *)rgb->getData();
    if (hasAlpha) {
        const uint8_t *srcAlpha = (const uint8_t *)&srcPixels[pixels];
        for (int32_t row = 0; row < h; row++) {
            uint32_t *des = &desData[offset + row * scanlength];
            uint32_t srcIdx = (uint32_t)(y + row) * imgW + x;
            for (int32_t col = 0; col < w; col++, srcIdx++) {
                uint32_t a = (srcIdx & 1) ? (srcAlpha[srcIdx >> 1] >> 4) : (srcAlpha[srcIdx >> 1] & 0x0F);
                des[col] = (a * 0x11) << 24 | colorRgb888(srcPixels[srcIdx]);
            }
        }
    }
    else {
        for (int32_t row = 0; row < h; row++) {
            uint32_t *des = &desData[offset + row * scanlength];
            const uint16_t *src = &srcPixels[(y + row) * imgW + x];
            for (int32_t col = 0; col < w; col++)
                des[col] = 0xFF000000 | colorRgb888(src[col]);
        }
    }
}
