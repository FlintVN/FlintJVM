
#include "flint.h"
#include "flint_system_api.h"
#include "flint_png_decoder.h"
#include "flint_native_image_decoder.h"

static void throwImageFormatException(FNIEnv *env, const char *msg) {
    env->throwNew(env->findClass("flint/drawing/ImageFormatException"), msg);
}

jobject NativeImageDecoder_DecodeBmpToRgb565(FNIEnv *env, jbyteArray imageData, jint off, jint len) {
    // TODO
    return NULL;
}

jobject NativeImageDecoder_DecodePngToRgb565(FNIEnv *env, jbyteArray imageData, jint off, jint len) {
    PngDecoder decoder;

    if(!decoder.setImageData((uint8_t *)&imageData->getData()[off], len)) {
        throwImageFormatException(env, "Png file format error");
        return NULL;
    }

    uint32_t pixels = decoder.getWidth() * decoder.getHeight();
    uint32_t rgbBytes = pixels * 2;
    uint32_t aBytes = decoder.hasAlpha() ? ((pixels + 1) >> 1) : 0;

    jbyteArray data = env->newByteArray(rgbBytes + aBytes);
    if(data == NULL) return NULL;

    if(!decoder.decodeToRgb565((uint8_t *)data->getData(), (uint8_t *)&data->getData()[rgbBytes])) {
        env->freeObject(data);
        throwImageFormatException(env, "Png file format error");
        return NULL;
    }

    jobject rgb565Img = env->newObject(env->findClass("flint/drawing/Rgb565Image"));
    if(rgb565Img != NULL) {
        rgb565Img->getFieldByIndex(0)->setInt32(decoder.hasAlpha());
        rgb565Img->getFieldByIndex(1)->setInt32(decoder.getWidth());
        rgb565Img->getFieldByIndex(2)->setInt32(decoder.getHeight());
        rgb565Img->getFieldByIndex(3)->setObj(data);
    }
    else
        env->freeObject(data);
    return rgb565Img;
}
