
#include "flint_system_api.h"
#include "flint_array_object.h"
#include "flint_native_common.h"
#include "flint_native_flint_socket_output_stream.h"

#if FLINT_API_NET_ENABLED

using namespace FlintAPI::Net;

extern jint NativeFlintSocketImpl_GetSock(FNIEnv *env, jobject socketObj, jbool throwable);

jvoid NativeFlintSocketOutputStream_SocketWrite(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock == -1) return;

    if(!CheckArrayIndexSize(env, b, off, len)) return;

    while(len > 0 && !env->hasTerminateRequest()) {
        int32_t sent;
        SockError err = send(sock, (uint8_t *)&b->getData()[off], len, &sent);
        if(err == SOCK_OK) {
            len -= sent;
            off += sent;
        }
        else if(err == SOCK_CLOSED)
            return;
        else if(err == SOCK_ERR) {
            env->throwNew(env->findClass("java/io/IOException"), "Write error");
            return;
        }
    }
}

#else

jvoid NativeFlintSocketOutputStream_SocketWrite(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len) {
    (void)env;
    (void)obj;
    (void)b;
    (void)off;
    (void)len;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "Network system not supported");
    return;
}

#endif /* FLINT_API_NET_ENABLED */
