
#ifndef __FLINT_NATIVE_ARGB565_GRAPHICS_H
#define __FLINT_NATIVE_ARGB565_GRAPHICS_H

#include "flint_native.h"
#include "flint_default_conf.h"

#if FLINT_API_DRAW_ENABLED

jvoid NativeRgb565Graphics_Clear(FNIEnv *env, jobject obj, jobject c);
jvoid NativeRgb565Graphics_DrawLine(FNIEnv *env, jobject obj, jobject pen, jint x1, jint y1, jint x2, jint y2);
jvoid NativeRgb565Graphics_DrawRect(FNIEnv *env, jobject obj, jobject pen, jint x, jint y, jint w, jint h);
jvoid NativeRgb565Graphics_FillRect(FNIEnv *env, jobject obj, jobject c, jint x, jint y, jint w, jint h);
jvoid NativeRgb565Graphics_DrawRoundRect(FNIEnv *env, jobject obj, jobject pen, jint x, jint y, jint w, jint h, jint r1, jint r2, jint r3, jint r4);
jvoid NativeRgb565Graphics_FillRoundRect(FNIEnv *env, jobject obj, jobject c, jint x, jint y, jint w, jint h, jint r1, jint r2, jint r3, jint r4);
jvoid NativeRgb565Graphics_DrawEllipse(FNIEnv *env, jobject obj, jobject pen, jint x, jint y, jint w, jint h);
jvoid NativeRgb565Graphics_FillEllipse(FNIEnv *env, jobject obj, jobject c, jint x, jint y, jint w, jint h);
jvoid NativeRgb565Graphics_DrawArc(FNIEnv *env, jobject obj, jobject pen, jint x, jint y, jint w, jint h, jfloat startAngle, jfloat sweepAngle);
jvoid NativeRgb565Graphics_FillArc(FNIEnv *env, jobject obj, jobject c, jint x, jint y, jint w, jint h, jfloat startAngle, jfloat sweepAngle);
jvoid NativeRgb565Graphics_DrawPolygon(FNIEnv *env, jobject obj, jobject pen, jobject points);
jvoid NativeRgb565Graphics_FillPolygon(FNIEnv *env, jobject obj, jobject c, jobject points);
jvoid NativeRgb565Graphics_DrawString(FNIEnv *env, jobject obj, jstring str, jobject font, jobject c, jint x, jint y);
jvoid NativeRgb565Graphics_DrawImage(FNIEnv *env, jobject obj, jobject img, jint x, jint y, jint w, jint h);

inline constexpr NativeMethod argb565GraphicsMethods[] = {
    NATIVE_METHOD("clear",         "(Lflint/drawing/Color;)V",                                          NativeRgb565Graphics_Clear),
    NATIVE_METHOD("drawLine",      "(Lflint/drawing/Pen;IIII)V",                                        NativeRgb565Graphics_DrawLine),
    NATIVE_METHOD("drawRect",      "(Lflint/drawing/Pen;IIII)V",                                        NativeRgb565Graphics_DrawRect),
    NATIVE_METHOD("fillRect",      "(Lflint/drawing/Color;IIII)V",                                      NativeRgb565Graphics_FillRect),
    NATIVE_METHOD("drawRoundRect", "(Lflint/drawing/Pen;IIIIIIII)V",                                    NativeRgb565Graphics_DrawRoundRect),
    NATIVE_METHOD("fillRoundRect", "(Lflint/drawing/Color;IIIIIIII)V",                                  NativeRgb565Graphics_FillRoundRect),
    NATIVE_METHOD("drawEllipse",   "(Lflint/drawing/Pen;IIII)V",                                        NativeRgb565Graphics_DrawEllipse),
    NATIVE_METHOD("fillEllipse",   "(Lflint/drawing/Color;IIII)V",                                      NativeRgb565Graphics_FillEllipse),
    NATIVE_METHOD("drawArc",       "(Lflint/drawing/Pen;IIIIFF)V",                                      NativeRgb565Graphics_DrawArc),
    NATIVE_METHOD("fillArc",       "(Lflint/drawing/Color;IIIIFF)V",                                    NativeRgb565Graphics_FillArc),
    NATIVE_METHOD("drawPolygon",   "(Lflint/drawing/Pen;[Lflint/drawing/Point;)V",                      NativeRgb565Graphics_DrawPolygon),
    NATIVE_METHOD("fillPolygon",   "(Lflint/drawing/Color;[Lflint/drawing/Point;)V",                    NativeRgb565Graphics_FillPolygon),
    NATIVE_METHOD("drawString",    "(Ljava/lang/String;Lflint/drawing/Font;Lflint/drawing/Color;II)V",  NativeRgb565Graphics_DrawString),
    NATIVE_METHOD("drawImage",     "(Lflint/drawing/Image,IIII)V",                                      NativeRgb565Graphics_DrawImage),
};

#endif /* FLINT_API_DRAW_ENABLED */

#endif /* __FLINT_NATIVE_ARGB565_GRAPHICS_H */
