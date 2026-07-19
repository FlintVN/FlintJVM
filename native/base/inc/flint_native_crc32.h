
#ifndef __FLINT_NATIVE_CRC32_H
#define __FLINT_NATIVE_CRC32_H

#include "flint_native.h"

jint NativeCrc32_Update(FNIEnv *env, jint crc, jint b);
jint NativeCrc32_UpdateBytes0(FNIEnv *env, jint crc, jbyteArray b, jint off, jint len);

inline constexpr NativeMethod crc32Methods[] = {
    NATIVE_METHOD("update",       "(II)I",    NativeCrc32_Update),
    NATIVE_METHOD("updateBytes0", "(I[BII)I", NativeCrc32_UpdateBytes0),
};

#endif /* __FLINT_NATIVE_CRC32_H */
