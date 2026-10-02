#ifndef __FLINT_NATIVE_IMAGE_DECODER_H
#define __FLINT_NATIVE_IMAGE_DECODER_H

#include "flint_native.h"
#include "flint_default_conf.h"

jobject NativeImageDecoder_DecodeBmpToRgb565(FNIEnv *env, jbyteArray imageData, jint off, jint len);
jobject NativeImageDecoder_DecodePngToRgb565(FNIEnv *env, jbyteArray imageData, jint off, jint len);

inline constexpr NativeMethod imageDecoderMethods[] = {
    NATIVE_METHOD("decodeBmpToRgb565", "([BII)Lflint/drawing/Rgb565Image;", NativeImageDecoder_DecodeBmpToRgb565),
    NATIVE_METHOD("decodePngToRgb565", "([BII)Lflint/drawing/Rgb565Image;", NativeImageDecoder_DecodePngToRgb565),
};

#endif /* __FLINT_NATIVE_IMAGE_DECODER_H */
