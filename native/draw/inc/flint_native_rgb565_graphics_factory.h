
#ifndef __FLINT_NATIVE_RGB565_GRAPHICS_FACTORY_H
#define __FLINT_NATIVE_RGB565_GRAPHICS_FACTORY_H

#include "flint_native.h"

jobject NativeRgb565GraphicsFactory_CreateImage(FNIEnv *env, jobject obj, jintArray rgb, jint w, jint h, jbool processAlpha);

inline constexpr NativeMethod rgb565GraphicsFactoryMethods[] = {
    NATIVE_METHOD("createImage", "([IIIZ)Lflint/drawing/Image;", NativeRgb565GraphicsFactory_CreateImage),
};

#endif /* __FLINT_NATIVE_RGB565_GRAPHICS_FACTORY_H */
