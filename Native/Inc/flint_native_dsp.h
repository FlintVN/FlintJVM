
#ifndef __FLINT_NATIVE_DSP_H
#define __FLINT_NATIVE_DSP_H

#include "flint_native.h"

jfloatArray NativeDsp_ToFloatArray(FNIEnv *env, jintArray values);
jvoid NativeDsp_ApplyWindowInPlace(FNIEnv *env, jfloatArray values, jobject type);
jvoid NativeDsp_FftInPlace(FNIEnv *env, jfloatArray reals, jfloatArray imags);
jvoid NativeDsp_IfftInPlace(FNIEnv *env, jfloatArray reals, jfloatArray imags);
jfloatArray NativeDsp_Dct1(FNIEnv *env, jintArray values);
jfloatArray NativeDsp_Dct2(FNIEnv *env, jfloatArray values);
jfloatArray NativeDsp_Idct1(FNIEnv *env, jintArray values);
jfloatArray NativeDsp_Idct2(FNIEnv *env, jfloatArray values);

inline constexpr NativeMethod dspMethods[] = {
    NATIVE_METHOD("toFloatArray",          "([I)[F",                       NativeDsp_ToFloatArray),
    NATIVE_METHOD("applyWindowInPlace",    "([FLflint/math/WindowType;)V", NativeDsp_ApplyWindowInPlace),
    NATIVE_METHOD("fftInPlace",            "([F[F)V",                      NativeDsp_FftInPlace),
    NATIVE_METHOD("ifftInPlace",           "([F[F)V",                      NativeDsp_IfftInPlace),
    NATIVE_METHOD("dct",                   "([I)[F",                       NativeDsp_Dct1),
    NATIVE_METHOD("dct",                   "([F)[F",                       NativeDsp_Dct2),
    NATIVE_METHOD("idct",                  "([I)[F",                       NativeDsp_Idct1),
    NATIVE_METHOD("idct",                  "([F)[F",                       NativeDsp_Idct2),
};

#endif /* __FLINT_NATIVE_DSP_H */
