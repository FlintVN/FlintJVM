
#ifndef __FLINT_NATIVE_RGB565_IMAGE_H
#define __FLINT_NATIVE_RGB565_IMAGE_H

#include "flint_native.h"

jvoid NativeRgb565Image_GetRgb(FNIEnv *env, jobject obj, jintArray rgb, jint offset, jint scanlength, jint x, jint y, jint w, jint h);

inline constexpr NativeMethod rgb565ImageMethods[] = {
    NATIVE_METHOD("getRgb", "([IIIIIII)V", NativeRgb565Image_GetRgb),
};

#endif /* __FLINT_NATIVE_RGB565_IMAGE_H */
