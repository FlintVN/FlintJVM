
#include "flint_system_api.h"
#include "flint_array_object.h"
#include "flint_native_common.h"
#include "flint_native_flint_socket_input_stream.h"

#if FLINT_API_NET_ENABLED

using namespace FlintAPI::Net;
using namespace FlintAPI::System;

extern jint NativeFlintSocketImpl_GetSock(FNIEnv *env, jobject socketObj, jbool throwable);

jint NativeFlintSocketInputStream_SocketRead(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock == -1) return -1;

    if(!CheckArrayIndexSize(env, b, off, len)) return -1;

    jobject impl = env->getObjField(env->getFieldId(obj, "impl"));
    int32_t timeout = env->getIntField(env->getFieldId(impl, "timeout"));
    uint64_t startTime = getTimeMillis();

    while(!env->hasTerminateRequest() && (timeout <= 0 || ((uint64_t)(getTimeMillis() - startTime)) < timeout)) {
        int32_t n;
        SockError err = recv(sock, (uint8_t *)&b->getData()[off], len, &n);
        if(err == SOCK_OK) return n;
        else if(err == SOCK_CLOSED) return -1;
        else if(err == SOCK_ERR) {
            env->throwNew(env->findClass("java/io/IOException"), "Read error");
            return -1;
        }
    }
    if(!env->hasTerminateRequest())
        env->throwNew(env->findClass("java/net/SocketTimeoutException"), "Read timed out");
    return -1;
}

#else

jint NativeFlintSocketInputStream_SocketRead(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len) {
    (void)env;
    (void)obj;
    (void)b;
    (void)off;
    (void)len;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "Network system not supported");
    return NULL;
}

#endif /* FLINT_API_NET_ENABLED */
