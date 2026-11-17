
#ifndef __FLINT_NATIVE_RGB565_GRAPHICS_FACTORY_H
#define __FLINT_NATIVE_RGB565_GRAPHICS_FACTORY_H

#include "flint_native.h"

jobject NativeRgb565GraphicsFactory_CreateImage1(FNIEnv *env, jobject obj, jintArray rgb, jint w, jint h, jbool processAlpha);
jobject NativeRgb565GraphicsFactory_CreateImage2(FNIEnv *env, jobject obj, jobject src, jint x, jint y, jint w, jint h, jint transform);

inline constexpr NativeMethod rgb565GraphicsFactoryMethods[] = {
    NATIVE_METHOD("createImage", "([IIIZ)Lflint/drawing/Image;",                      NativeRgb565GraphicsFactory_CreateImage1),
    NATIVE_METHOD("createImage", "(Lflint/drawing/Image;IIIII)Lflint/drawing/Image;", NativeRgb565GraphicsFactory_CreateImage2),
};

#endif /* __FLINT_NATIVE_RGB565_GRAPHICS_FACTORY_H */
