
#ifndef __FLINT_NATIVE_DSP_H
#define __FLINT_NATIVE_DSP_H

#include "flint_native.h"

jfloatArray NativeDsp_ToFloatArray(FNIEnv *env, jintArray values);
jvoid NativeDsp_ApplyWindowInPlace(FNIEnv *env, jfloatArray values, jobject type);
jvoid NativeDsp_FftInPlace(FNIEnv *env, jfloatArray reals, jfloatArray imags);
jvoid NativeDsp_IfftInPlace(FNIEnv *env, jfloatArray reals, jfloatArray imags);
jvoid NativeDsp_DctInPlace(FNIEnv *env, jfloatArray values);
jvoid NativeDsp_IdctInPlace(FNIEnv *env, jfloatArray values);

inline constexpr NativeMethod dspMethods[] = {
    NATIVE_METHOD("toFloatArray",          "([I)[F",                       NativeDsp_ToFloatArray),
    NATIVE_METHOD("applyWindowInPlace",    "([FLflint/math/WindowType;)V", NativeDsp_ApplyWindowInPlace),
    NATIVE_METHOD("fftInPlace",            "([F[F)V",                      NativeDsp_FftInPlace),
    NATIVE_METHOD("ifftInPlace",           "([F[F)V",                      NativeDsp_IfftInPlace),
    NATIVE_METHOD("dctInPlace",            "([F)V",                        NativeDsp_DctInPlace),
    NATIVE_METHOD("idctInPlace",           "([F)V",                        NativeDsp_IdctInPlace),
};

#endif /* __FLINT_NATIVE_DSP_H */
