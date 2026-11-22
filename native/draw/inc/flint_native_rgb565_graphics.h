
#ifndef __FLINT_NATIVE_RGB565_GRAPHICS_H
#define __FLINT_NATIVE_RGB565_GRAPHICS_H

#include "flint_native.h"

jvoid NativeRgb565Graphics_Clear1(FNIEnv *env, jobject obj);
jvoid NativeRgb565Graphics_Clear2(FNIEnv *env, jobject obj, jint c);
jvoid NativeRgb565Graphics_Clear3(FNIEnv *env, jobject obj, jint x, jint y, jint w, jint h);
jvoid NativeRgb565Graphics_Clear4(FNIEnv *env, jobject obj, jint c, jint x, jint y, jint w, jint h);
jvoid NativeRgb565Graphics_DrawLine(FNIEnv *env, jobject obj, jint c, jint x1, jint y1, jint x2, jint y2);
jvoid NativeRgb565Graphics_DrawRect(FNIEnv *env, jobject obj, jint c, jint x, jint y, jint w, jint h);
jvoid NativeRgb565Graphics_FillRect(FNIEnv *env, jobject obj, jint c, jint x, jint y, jint w, jint h);
jvoid NativeRgb565Graphics_DrawRoundRect(FNIEnv *env, jobject obj, jint c, jint x, jint y, jint w, jint h, jint r1, jint r2, jint r3, jint r4);
jvoid NativeRgb565Graphics_FillRoundRect(FNIEnv *env, jobject obj, jint c, jint x, jint y, jint w, jint h, jint r1, jint r2, jint r3, jint r4);
jvoid NativeRgb565Graphics_DrawEllipse(FNIEnv *env, jobject obj, jint c, jint x, jint y, jint w, jint h);
jvoid NativeRgb565Graphics_FillEllipse(FNIEnv *env, jobject obj, jint c, jint x, jint y, jint w, jint h);
jvoid NativeRgb565Graphics_DrawArc(FNIEnv *env, jobject obj, jint c, jint x, jint y, jint w, jint h, jfloat startAngle, jfloat sweepAngle);
jvoid NativeRgb565Graphics_FillArc(FNIEnv *env, jobject obj, jint c, jint x, jint y, jint w, jint h, jfloat startAngle, jfloat sweepAngle);
jvoid NativeRgb565Graphics_DrawPolygon(FNIEnv *env, jobject obj, jint c, jobject points);
jvoid NativeRgb565Graphics_FillPolygon(FNIEnv *env, jobject obj, jint c, jobject points);
jvoid NativeRgb565Graphics_DrawString(FNIEnv *env, jobject obj, jstring str, jobject font, jint c, jint x, jint y);
jvoid NativeRgb565Graphics_DrawImage1(FNIEnv *env, jobject obj, jobject img, jint x, jint y);
jvoid NativeRgb565Graphics_DrawImage2(FNIEnv *env, jobject obj, jobject img, jint x, jint y, jint w, jint h);
jvoid NativeRgb565Graphics_DrawRgb(FNIEnv *env, jobject obj, jintArray rgb, jint offset, jint scanlength, jint x, jint y, jint width, jint height, jbool processAlpha);
jvoid NativeRgb565Graphics_DrawRegion(FNIEnv *env, jobject obj, jobject src, jint sx, jint sy, jint sw, jint sh, jint transform, jint dx, jint dy);

inline constexpr NativeMethod rgb565GraphicsMethods[] = {
    NATIVE_METHOD("clear",         "()V",                                           NativeRgb565Graphics_Clear1),
    NATIVE_METHOD("clear",         "(I)V",                                          NativeRgb565Graphics_Clear2),
    NATIVE_METHOD("clear",         "(IIII)V",                                       NativeRgb565Graphics_Clear3),
    NATIVE_METHOD("clear",         "(IIIII)V",                                      NativeRgb565Graphics_Clear4),
    NATIVE_METHOD("drawLine",      "(IIIII)V",                                      NativeRgb565Graphics_DrawLine),
    NATIVE_METHOD("drawRect",      "(IIIII)V",                                      NativeRgb565Graphics_DrawRect),
    NATIVE_METHOD("fillRect",      "(IIIII)V",                                      NativeRgb565Graphics_FillRect),
    NATIVE_METHOD("drawRoundRect", "(IIIIIIIII)V",                                  NativeRgb565Graphics_DrawRoundRect),
    NATIVE_METHOD("fillRoundRect", "(IIIIIIIII)V",                                  NativeRgb565Graphics_FillRoundRect),
    NATIVE_METHOD("drawEllipse",   "(IIIII)V",                                      NativeRgb565Graphics_DrawEllipse),
    NATIVE_METHOD("fillEllipse",   "(IIIII)V",                                      NativeRgb565Graphics_FillEllipse),
    NATIVE_METHOD("drawArc",       "(IIIIIFF)V",                                    NativeRgb565Graphics_DrawArc),
    NATIVE_METHOD("fillArc",       "(IIIIIFF)V",                                    NativeRgb565Graphics_FillArc),
    NATIVE_METHOD("drawPolygon",   "(I[Lflint/drawing/Point;)V",                    NativeRgb565Graphics_DrawPolygon),
    NATIVE_METHOD("fillPolygon",   "(I[Lflint/drawing/Point;)V",                    NativeRgb565Graphics_FillPolygon),
    NATIVE_METHOD("drawString",    "(Ljava/lang/String;Lflint/drawing/Font;III)V",  NativeRgb565Graphics_DrawString),
    NATIVE_METHOD("drawImage",     "(Lflint/drawing/Image;II)V",                    NativeRgb565Graphics_DrawImage1),
    NATIVE_METHOD("drawImage",     "(Lflint/drawing/Image;IIII)V",                  NativeRgb565Graphics_DrawImage2),
    NATIVE_METHOD("drawRgb",       "([IIIIIIIZ)V",                                  NativeRgb565Graphics_DrawRgb),
    NATIVE_METHOD("drawRegion",    "(Lflint/drawing/Image;IIIIIII)V",               NativeRgb565Graphics_DrawRegion),
};

#endif /* __FLINT_NATIVE_RGB565_GRAPHICS_H */
