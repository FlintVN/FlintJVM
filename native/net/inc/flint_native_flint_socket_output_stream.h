

#ifndef __FLINT_NATIVE_FLINT_SOCKET_OUTPUT_STREAM_H
#define __FLINT_NATIVE_FLINT_SOCKET_OUTPUT_STREAM_H

#include "flint_native.h"
#include "flint_default_conf.h"

#if FLINT_API_NET_ENABLED

jvoid NativeFlintSocketOutputStream_SocketWrite(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len);

inline constexpr NativeMethod flintSocketOutputStreamMethods[] = {
    NATIVE_METHOD("socketWrite", "([BII)V", NativeFlintSocketOutputStream_SocketWrite),
};

#endif /* FLINT_API_NET_ENABLED */

#endif /* __FLINT_NATIVE_FLINT_SOCKET_OUTPUT_STREAM_H */
