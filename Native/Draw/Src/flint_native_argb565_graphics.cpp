
#include "flint_default_conf.h"
#include "flint_array_object.h"
#include "flint_rgb565_drawing.h"
#include "flint_native_argb565_graphics.h"

#if FLINT_API_DRAW_ENABLED

static void FGfx_Init(FGfx *g, jobject rgb565Graphics) {
    g->w = rgb565Graphics->getFieldByIndex(0)->getInt32();
    g->h = rgb565Graphics->getFieldByIndex(1)->getInt32();
    g->clipX1 = rgb565Graphics->getFieldByIndex(2)->getInt32();
    g->clipY1 = rgb565Graphics->getFieldByIndex(3)->getInt32();
    g->clipX2 = g->clipX1 + rgb565Graphics->getFieldByIndex(4)->getInt32();
    g->clipY2 = g->clipY1 + rgb565Graphics->getFieldByIndex(5)->getInt32();
    g->data = (uint8_t *)((jbyteArray)rgb565Graphics->getFieldByIndex(6)->getObj())->getData();
}

jvoid NativeRgb565Graphics_DrawLine(FNIEnv *env, jobject obj, jobject pen, jint x1, jint y1, jint x2, jint y2) {
    jobject color = pen->getFieldByIndex(0)->getObj();
    jint colorVal = color->getFieldByIndex(0)->getInt32();
    jint penWidth = pen->getFieldByIndex(1)->getInt32();
    FGfx g;
    FGfx_Init(&g, obj);
    Rgb565_DrawLine(&g, colorVal, penWidth, x1, y1, x2, y2);
}

jvoid NativeRgb565Graphics_DrawRect(FNIEnv *env, jobject obj, jobject pen, jint x, jint y, jint w, jint h) {

}

jvoid NativeRgb565Graphics_FillRect(FNIEnv *env, jobject obj, jobject c, jint x, jint y, jint w, jint h) {

}

jvoid NativeRgb565Graphics_DrawRoundRect(FNIEnv *env, jobject obj, jobject pen, jint x, jint y, jint w, jint h, jint r1, jint r2, jint r3, jint r4) {

}

jvoid NativeRgb565Graphics_FillRoundRect(FNIEnv *env, jobject obj, jobject c, jint x, jint y, jint w, jint h, jint r1, jint r2, jint r3, jint r4) {

}

jvoid NativeRgb565Graphics_DrawEllipse(FNIEnv *env, jobject obj, jobject pen, jint x, jint y, jint w, jint h) {

}

jvoid NativeRgb565Graphics_FillEllipse(FNIEnv *env, jobject obj, jobject c, jint x, jint y, jint w, jint h) {

}

jvoid NativeRgb565Graphics_DrawArc(FNIEnv *env, jobject obj, jobject pen, jint x, jint y, jint w, jint h, jfloat startAngle, jfloat sweepAngle) {

}

jvoid NativeRgb565Graphics_FillArc(FNIEnv *env, jobject obj, jobject c, jint x, jint y, jint w, jint h, jfloat startAngle, jfloat sweepAngle) {

}

jvoid NativeRgb565Graphics_DrawPolygon(FNIEnv *env, jobject obj, jobject pen, jobject points) {

}

jvoid NativeRgb565Graphics_FillPolygon(FNIEnv *env, jobject obj, jobject c, jobject points) {
    
}

jvoid NativeRgb565Graphics_DrawString(FNIEnv *env, jobject obj, jstring str, jobject font, jobject c, jint x, jint y) {

}

jvoid NativeRgb565Graphics_DrawImage(FNIEnv *env, jobject obj, jobject img, jint x, jint y, jint w, jint h) {

}

#endif /* FLINT_API_DRAW_ENABLED */
