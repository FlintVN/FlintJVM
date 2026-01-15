
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "flint.h"
#include "flint_array_object.h"
#include "flint_native_dsp.h"

#define HANN                0
#define HAMMING             1
#define BLACKMAN            2

#define PI                  3.141592653589793f
#define PIx2                6.283185307179586f
#define PIx4                12.56637061435917f

static bool NativeDsp_CheckFftInput(FNIEnv *env, jfloatArray reals, jfloatArray imags) {
    if(reals == NULL || imags == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return false;
    }
    uint32_t len = reals->getLength();
    if(len == 0 || len != imags->getLength()) {
        env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "The input arrays must be the same size and not empty");
        return false;
    }
    return true;
}

static bool NativeDsp_CheckDctInput(FNIEnv *env, jarray data) {
    if(data == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return false;
    }
    uint32_t len = data->getLength();
    if(len == 0) {
        env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "The size of the input array must be not empty");
        return false;
    }
    return true;
}

static inline uint32_t NativeDsp_BitRevIncrement(uint32_t x, uint32_t msb) {
    uint32_t m = msb;
    while(x & m) {
        x ^= m;
        m >>= 1;
    }
    return x | m;
}

static void NativeDsp_BitReversePermute(float *reals, float *imags, uint32_t N) {
    uint32_t j = 0;
    uint32_t msb = N >> 1;

    for(uint32_t i = 1; i < N - 1; i++) {
        j = NativeDsp_BitRevIncrement(j, msb);

        if(i < j) {
            float tmp = reals[i];
            reals[i] = reals[j];
            reals[j] = tmp;

            tmp = imags[i];
            imags[i] = imags[j];
            imags[j] = tmp;
        }
    }
}

static void NativeDsp_FftRadix2Recursive(float *reals, float *imags, uint32_t N) {
    if(N >= 2) {
        N /= 2;
        NativeDsp_FftRadix2Recursive(reals, imags, N);
        NativeDsp_FftRadix2Recursive(reals + N, imags + N, N);

        float tmp = reals[0];
        reals[0] += reals[N];
        reals[N] = tmp - reals[N];

        tmp = imags[0];
        imags[0] += imags[N];
        imags[N] = tmp - imags[N];

        for(uint32_t i = 1; i < N; i++) {
            float angle = PI * i / N;
            float wr = cosf(angle);
            float wi = -sinf(angle);

            float x1 = (wr * reals[i + N] - wi * imags[i + N]);
            float x2 = (wr * imags[i + N] + wi * reals[i + N]);

            reals[i + N] = reals[i] - x1;
            reals[i] += x1;

            imags[i + N] = imags[i] - x2;
            imags[i] += x2;
        }
    }
}

static void NativeDsp_FftRadix2(float *reals, float *imags, uint32_t N) {
    NativeDsp_BitReversePermute(reals, imags, N);
    NativeDsp_FftRadix2Recursive(reals, imags, N);
}

static void NativeDsp_Dft(const float *reIn, const float *imIn, float *reOut, float *imOut, uint32_t N) {
    for(uint32_t k = 0; k < N; k++) {
        float sumRe = 0.0f;
        float sumIm = 0.0f;
        for(uint32_t n = 0; n < N; n++) {
            float angle = -PIx2 * k * n / N;
            float c = cosf(angle);
            float s = sinf(angle);
            sumRe += reIn[n] * c - imIn[n] * s;
            sumIm += reIn[n] * s + imIn[n] * c;
        }
        reOut[k] = sumRe;
        imOut[k] = sumIm;
    }
}

static bool NativeDsp_Fft(FNIEnv *env, float *reals, float *imags, uint32_t N) {
    if(((N - 1) & N) == 0)
        NativeDsp_FftRadix2(reals, imags, N);
    else {
        float *rIn = (float *)Flint::malloc(env->exec, N * sizeof(float));
        float *iIn = (float *)Flint::malloc(env->exec, N * sizeof(float));

        if(rIn == NULL || iIn == NULL) {
            if(rIn != NULL) Flint::free(rIn);
            if(iIn != NULL) Flint::free(iIn);
            return false;
        }

        memcpy(rIn, reals, N * sizeof(float));
        memcpy(iIn, imags, N * sizeof(float));

        NativeDsp_Dft(rIn, iIn, reals, imags, N);

        Flint::free(rIn);
        Flint::free(iIn);
    }
    return true;
}

template <typename T>
static bool NativeDsp_Dct(FNIEnv *env, T *input, float *output, uint32_t N) {
    if(((N - 1) & N) == 0) {
        uint32_t Nx2 = N * 2;
        float *re = (float *)Flint::malloc(env->exec, Nx2 * sizeof(float));
        float *im = (float *)Flint::malloc(env->exec, Nx2 * sizeof(float));
        if(re == NULL || im == NULL) {
            if(re != NULL) Flint::free(re);
            if(im != NULL) Flint::free(im);
            return false;
        }

        for(uint32_t i = 0; i < N; i++) {
            re[i] = input[i];
            im[i] = 0;

            re[Nx2 - 1 - i] = input[i];
            im[Nx2 - 1 - i] = 0;
        }

        NativeDsp_FftRadix2(re, im, Nx2);

        output[0] = sqrtf(1.0f / N) * re[0] / 2.0f;

        float alpha = sqrtf(2.0f / N);
        for(uint32_t k = 1; k < N; k++) {
            float angle = PI * k / Nx2;
            output[k] = alpha * (re[k] * cosf(angle) + im[k] * sinf(angle)) / 2.0f;
        }

        Flint::free(re);
        Flint::free(im);
    }
    else {
        float alpha1 = sqrtf(1.0f / N);
        float alpha2 = sqrtf(2.0f / N);
        for(uint32_t k = 0; k < N; k++) {
            float sum = 0.0f;
            float alpha = (k == 0) ? alpha1 : alpha2;
            for(uint32_t n = 0; n < N; n++)
                sum += cosf(PI * (0.5f + n) * k / N) * input[n];
            output[k] = alpha * sum;
        }
    }
    return true;
}

template <typename T>
static bool NativeDsp_Idct(FNIEnv *env, T *input, float *output, uint32_t N) {
    float alpha1 = sqrtf(1.0f / N);
    float alpha2 = sqrtf(2.0f / N);
    for(uint32_t n = 0; n < N; n++) {
        float sum = alpha1 * input[0];
        for(uint32_t k = 1; k < N; k++)
            sum += alpha2 * input[k] * cosf(PI * (n + 0.5f) * k / N);
        output[n] = sum;
    }
    return true;
}

jfloatArray NativeDsp_ToFloatArray(FNIEnv *env, jintArray values) {
    if(values == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return NULL;
    }
    int32_t len = values->getLength();
    jfloatArray arr = env->newFloatArray(len);
    int32_t *idata = values->getData();
    jfloat *fdata = arr->getData();
    for(uint32_t i = 0; i < len; i++)
        fdata[i] = (jfloat)idata[i];
    return arr;
}

jvoid NativeDsp_ApplyWindowInPlace(FNIEnv *env, jfloatArray values, jobject type) {
    if(values == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return;
    }
    int32_t windowsType = type->getFieldByIndex(0)->getInt32();
    uint32_t N = values->getLength();
    uint32_t Nm1 = N - 1;
    jfloat *fdata = values->getData();
    switch(windowsType) {
        case HANN: {
            for(uint32_t i = 0; i < N; i++) {
                float w = 0.5f * (1 - cosf(PIx2 * i / Nm1));
                fdata[i] *= w;
            }
            return;
        }
        case HAMMING: {
            for(uint32_t i = 0; i < N; i++) {
                float w = 0.54f - 0.46f * cosf(PIx2 * i / Nm1);
                fdata[i] *= w;
            }
            return;
        }
        case BLACKMAN:
            for(uint32_t i = 0; i < N; i++) {
                float w = 0.42f - 0.5f * cosf(PIx2 * i / Nm1) + 0.08f * cosf(PIx4 * i / Nm1);
                fdata[i] *= w;
            }
            break;
        default:
            env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "Window type is not supported");
            return;
    }
}

jvoid NativeDsp_FftInPlace(FNIEnv *env, jfloatArray reals, jfloatArray imags) {
    if(!NativeDsp_CheckFftInput(env, reals, imags))
        return;
    NativeDsp_Fft(env, reals->getData(), imags->getData(), reals->getLength());
}

jvoid NativeDsp_IfftInPlace(FNIEnv *env, jfloatArray reals, jfloatArray imags) {
    if(!NativeDsp_CheckFftInput(env, reals, imags))
        return;

    uint32_t N = reals->getLength();
    float *rdata = reals->getData();
    float *idata = imags->getData();

    for(uint32_t i = 0; i < N; i++)
        idata[i] = -idata[i];

    if(!NativeDsp_Fft(env, rdata, idata, N))
        return;

    for(uint32_t i = 0; i < N; i++) {
        rdata[i] /= N;
        idata[i] = -idata[i] / N;
    }
}

jfloatArray NativeDsp_Dct1(FNIEnv *env, jintArray values) {
    if(!NativeDsp_CheckDctInput(env, values)) return NULL;

    uint32_t N = values->getLength();
    jfloatArray ret = env->newFloatArray(N);
    if(ret == NULL) return NULL;

    if(!NativeDsp_Dct(env, values->getData(), ret->getData(), N)) {
        env->freeObject(ret);
        return NULL;
    }

    return ret;
}

jfloatArray NativeDsp_Dct2(FNIEnv *env, jfloatArray values) {
    if(!NativeDsp_CheckDctInput(env, values)) return NULL;

    uint32_t N = values->getLength();
    jfloatArray ret = env->newFloatArray(N);
    if(ret == NULL) return NULL;

    if(!NativeDsp_Dct(env, values->getData(), ret->getData(), N)) {
        env->freeObject(ret);
        return NULL;
    }

    return ret;
}

jfloatArray NativeDsp_Idct1(FNIEnv *env, jintArray values) {
    if(!NativeDsp_CheckDctInput(env, values)) return NULL;
    
    uint32_t N = values->getLength();
    jfloatArray ret = env->newFloatArray(N);
    if(ret == NULL) return NULL;

    if(!NativeDsp_Idct(env, values->getData(), ret->getData(), N)) {
        env->freeObject(ret);
        return NULL;
    }

    return ret;
}

jfloatArray NativeDsp_Idct2(FNIEnv *env, jfloatArray values) {
    if(!NativeDsp_CheckDctInput(env, values)) return NULL;
    
    uint32_t N = values->getLength();
    jfloatArray ret = env->newFloatArray(N);
    if(ret == NULL) return NULL;

    if(!NativeDsp_Idct(env, values->getData(), ret->getData(), N)) {
        env->freeObject(ret);
        return NULL;
    }

    return ret;
}
