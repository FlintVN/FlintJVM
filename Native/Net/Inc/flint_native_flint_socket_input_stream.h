

#ifndef __FLINT_NATIVE_FLINT_SOCKET_INPUT_STREAM_H
#define __FLINT_NATIVE_FLINT_SOCKET_INPUT_STREAM_H

#include "flint_native.h"
#include "flint_default_conf.h"

#if FLINT_API_NET_ENABLED

jint NativeFlintSocketInputStream_SocketRead(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len);

inline constexpr NativeMethod flintSocketInputStreamMethods[] = {
    NATIVE_METHOD("socketRead", "([BII)I", NativeFlintSocketInputStream_SocketRead),
};

#endif /* FLINT_API_NET_ENABLED */

#endif /* __FLINT_NATIVE_FLINT_SOCKET_INPUT_STREAM_H */
