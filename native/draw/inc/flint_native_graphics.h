#ifndef __FLINT_NATIVE_GRAPHICS_H
#define __FLINT_NATIVE_GRAPHICS_H

#include "flint_native.h"
#include "flint_default_conf.h"

#if FLINT_API_DRAW_ENABLED

jbool NativeGraphics_IsVisible(FNIEnv *env, jobject obj, jint x, jint y, jint w, jint h);
jvoid NativeGraphics_SetClip0(FNIEnv *env, jobject obj, jint x, jint y, jint w, jint h, jint mode);
jobject NativeGraphics_MeasureString(FNIEnv *env, jstring str, jobject font);
jint NativeGraphics_MeasureStringWidth(FNIEnv *env, jstring str, jobject font);
jint NativeGraphics_MeasureStringHeight(FNIEnv *env, jstring str, jobject font);

inline constexpr NativeMethod graphicsMethods[] = {
    NATIVE_METHOD("isVisible",           "(IIII)Z",                                                      NativeGraphics_IsVisible),
    NATIVE_METHOD("setClip0",            "(IIIII)V",                                                     NativeGraphics_SetClip0),
    NATIVE_METHOD("measureString",       "(Ljava/lang/String;Lflint/drawing/Font;)Lflint/drawing/Size;", NativeGraphics_MeasureString),
    NATIVE_METHOD("measureStringWidth",  "(Ljava/lang/String;Lflint/drawing/Font;)I",                    NativeGraphics_MeasureStringWidth),
    NATIVE_METHOD("measureStringHeight", "(Ljava/lang/String;Lflint/drawing/Font;)I",                    NativeGraphics_MeasureStringHeight),
};

#endif /* FLINT_API_DRAW_ENABLED */

#endif /* __FLINT_NATIVE_GRAPHICS_H */
