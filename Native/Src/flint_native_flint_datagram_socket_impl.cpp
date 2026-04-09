
#include "flint_system_api.h"
#include "flint_native_common.h"
#include "flint_java_inet_address.h"
#include "flint_native_flint_socket_impl.h"
#include "flint_native_flint_datagram_socket_impl.h"

#if FLINT_API_NET_ENABLED

#define NATIVE_TCP_NODELAY      0x0001
#define NATIVE_SO_BINDADDR      0x000F
#define NATIVE_SO_REUSEADDR     0x04
#define NATIVE_IP_MULTICAST_IF  0x10
#define NATIVE_SO_LINGER        0x0080
#define NATIVE_SO_TIMEOUT       0x1006

using namespace FlintAPI::Net;
using namespace FlintAPI::System;

extern void NativeFlintSocketImpl_SocketClose(void *handle);
extern jbool ConvertToSockAddr(InetAddress *inetAddr, jint port, SockAddr *addr);
extern Hook *NativeFlintSocketImpl_GetHook(FNIEnv *env, jobject socketObj, bool throwable);
extern jint NativeFlintSocketImpl_GetSock(FNIEnv *env, jobject socketObj, jbool throwable);
extern InetAddress *NativeFlintSocketImpl_CreateInetAddress(FNIEnv *env, SockAddr *addr);

class DatagramPacket : public JObject {
public:
    jbyteArray getBuf() { return (jbyteArray)getFieldByIndex(0)->getObj(); }
    jint getLength() { return getFieldByIndex(1)->getInt32(); }
    InetAddress *getAddress() { return (InetAddress *)getFieldByIndex(2)->getObj(); };
    jint getPort() { return getFieldByIndex(3)->getInt32(); }

    void setBuf(jbyteArray val) { getFieldByIndex(0)->setObj(val); }
    void setLength(jint val) { getFieldByIndex(1)->setInt32(val); }
    void setAddress(InetAddress *val) { getFieldByIndex(2)->setObj(val); };
    void setPort(jint val) { getFieldByIndex(3)->setInt32(val); }
};

jvoid NativeFlintDatagramSocketImpl_Bind(FNIEnv *env, jobject obj, jint lport, jobject laddr) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock == -1) return;

    InetAddress *inetAddr = (InetAddress *)laddr;
    SockAddr addr;
    if(!ConvertToSockAddr(inetAddr, lport, &addr)) {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid address");
        return;
    }
    if(bind(sock, &addr) != SOCK_OK)
        env->throwNew(env->findClass("java/io/IOException"), "Bind error");
    else
        env->setIntField(env->getFieldId(obj, "localPort"), lport);
}

jvoid NativeFlintDatagramSocketImpl_Send(FNIEnv *env, jobject obj, jobject p) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock == -1) return;

    if(p == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return;
    }

    jbyteArray b = ((DatagramPacket *)p)->getBuf();
    jint len = ((DatagramPacket *)p)->getLength();
    InetAddress *inetAddr = ((DatagramPacket *)p)->getAddress();
    jint port = ((DatagramPacket *)p)->getPort();

    jint off = 0;
    SockAddr addr;
    if(inetAddr == NULL) {
        env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "Address not set");
        return;
    }
    else if(!ConvertToSockAddr(inetAddr, port, &addr)) {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid address");
        return;
    }

    if(!CheckArrayIndexSize(env, b, 0, len)) return;

    while(len > 0 && !env->hasTerminateRequest()) {
        int32_t sent;
        SockError err = sendTo(sock, &addr, (uint8_t *)&b->getData()[off], len, &sent);
        if(err == SOCK_OK) {
            len -= sent;
            off += sent;
        }
        else if(err == SOCK_ERR) {
            env->throwNew(env->findClass("java/io/IOException"), "Write error");
            return;
        }
    }
}

jint NativeFlintDatagramSocketImpl_Peek(FNIEnv *env, jobject obj, jobject i) {
    // TODO
    (void)env;
    (void)obj;
    (void)i;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"));
    return -1;
}

jvoid NativeFlintDatagramSocketImpl_Receive(FNIEnv *env, jobject obj, jobject p) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock == -1) return;

    if(p == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return;
    }

    jint off = 0;
    DatagramPacket *packet = (DatagramPacket *)p;
    jbyteArray b = packet->getBuf();
    jint len = packet->getLength();

    if(!CheckArrayIndexSize(env, b, off, len)) return;

    int32_t timeout = env->getIntField(env->getFieldId(obj, "timeout"));
    uint64_t startTime = getTimeMillis();

    while(!env->hasTerminateRequest() && (timeout <= 0 || ((uint64_t)(getTimeMillis() - startTime)) < timeout)) {
        int32_t n;
        SockAddr addr;
        SockError err = recvFrom(sock, &addr, (uint8_t *)b->getData(), len, &n);
        if(err == SOCK_OK) {
            InetAddress *inetAddr = NativeFlintSocketImpl_CreateInetAddress(env, &addr);
            if(inetAddr == NULL) return;
            packet->setAddress(inetAddr);
            inetAddr->clearProtected();
            if(inetAddr->getFamily() == NET_INET6)
                ((Inet6Address *)inetAddr)->getAddress()->clearProtected();
            packet->setPort(addr.port);
            packet->setLength(n);
            return;
        }
        else if(err == SOCK_ERR) {
            env->throwNew(env->findClass("java/io/IOException"), "Read error");
            return;
        }
    }
    if(!env->hasTerminateRequest())
        env->throwNew(env->findClass("java/net/SocketTimeoutException"), "Read timed out");
}

jvoid NativeFlintDatagramSocketImpl_SetTTL(FNIEnv *env, jobject obj, jbyte ttl) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock == -1) return;
    int32_t tmp = ttl;
    if(setSockOpt(sock, SOCK_UNICAST_HOPS, true, &tmp) != SOCK_OK)
        env->throwNew(env->findClass("java/io/IOException"), "Set TTL error");
}

jbyte NativeFlintDatagramSocketImpl_GetTTL(FNIEnv *env, jobject obj) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock == -1) return 0;

    int32_t hopLimit;
    if(getSockOpt(sock, SOCK_UNICAST_HOPS, NULL, &hopLimit) != SOCK_OK)
        env->throwNew(env->findClass("java/io/IOException"), "Get TTL error");
    return hopLimit;
}

jvoid NativeFlintDatagramSocketImpl_Join(FNIEnv *env, jobject obj, jobject inetaddr) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock == -1) return;

    InetAddress *inetObj = (InetAddress *)inetaddr;
    SockAddr addr;
    if(!ConvertToSockAddr(inetObj, 0, &addr)) {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid address");
        return;
    }
    if(setSockOpt(sock, SOCK_JOIN_GROUP, NULL, &addr) != SOCK_OK)
        env->throwNew(env->findClass("java/net/IOException"), "Join error");
}

jvoid NativeFlintDatagramSocketImpl_Leave(FNIEnv *env, jobject obj, jobject inetaddr) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock == -1) return;

    InetAddress *inetObj = (InetAddress *)inetaddr;
    SockAddr addr;
    if(!ConvertToSockAddr(inetObj, 0, &addr)) {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid address");
        return;
    }
    if(setSockOpt(sock, SOCK_LEAVE_GROUP, NULL, &addr) != SOCK_OK)
        env->throwNew(env->findClass("java/net/IOException"), "Leave error");
}

jvoid NativeFlintDatagramSocketImpl_DatagramSocketCreate(FNIEnv *env, jobject obj) {
    FExec *exec = (FExec *)env;
    int32_t sock = socket(false);
    Hook *hook = sock == -1 ? NULL : exec->getFlint()->addShutdownHook(exec, (void *)sock, NativeFlintSocketImpl_SocketClose);
    if(hook == NULL) {
        if(sock != -1) close(sock);
        env->throwNew(env->findClass("java/io/IOException"), "Create DatagramSocket error");
        return;
    }
    jobject fdObj = env->getObjField(env->getFieldId(obj, "fd"));
    env->setIntField(env->getFieldId(fdObj, "fd"), (int32_t)hook);
}

jvoid NativeFlintDatagramSocketImpl_DatagramSocketClose(FNIEnv *env, jobject obj) {
    Hook *hook = NativeFlintSocketImpl_GetHook(env, obj, true);
    if(hook == NULL) return;
    ((FExec *)env)->getFlint()->removeShutdownHook(hook);
    NativeFlintSocketImpl_SocketClose(hook->getHandle());
}

jvoid NativeFlintDatagramSocketImpl_SocketSetOption(FNIEnv *env, jobject obj, jint opt, jobject val) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock == -1) return;

    switch(opt) {
        case NATIVE_SO_REUSEADDR: {
            if(val == NULL) {
                env->throwNew(env->findClass("java/lang/NullPointerException"));
                return;
            }
            int32_t tmp = val->getFieldByIndex(0)->getInt32();
            if(setSockOpt(sock, SOCK_SO_REUSEADDR, true, &tmp) != SOCK_OK)
                env->throwNew(env->findClass("java/io/IOException"), "Set SO_REUSEADDR error");
            break;
        }
        default:
            env->throwNew(env->findClass("java/io/IOException"), "Unsupported socket option");
            break;
    }
}

jobject NativeFlintDatagramSocketImpl_SocketGetOption(FNIEnv *env, jobject obj, jint opt) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock == -1) return NULL;

    switch(opt) {
        case NATIVE_SO_REUSEADDR: {
            int32_t val = 0;
            if(getSockOpt(sock, SOCK_SO_REUSEADDR, NULL, &val) != SOCK_OK) {
                env->throwNew(env->findClass("java/io/IOException"), "Get SO_REUSEADDR error");
                return NULL;
            }
            jobject ret = env->newObject(env->findClass("java/lang/Integer"));
            if(ret == NULL) return NULL;
            ret->getFieldByIndex(0)->setInt32(val);
            return ret;
        }
        case NATIVE_SO_BINDADDR: {
            SockAddr addr;
            if(getSockOpt(sock, SOCK_SO_BINDADDR, NULL, &addr) != SOCK_OK) {
                env->throwNew(env->findClass("java/io/IOException"), "Get SO_BINDADDR error");
                return NULL;
            }
            return NativeFlintSocketImpl_CreateInetAddress(env, &addr);
        }
        default:
            env->throwNew(env->findClass("java/io/IOException"), "Unsupported socket option");
            return NULL;
    }
}

#else

jvoid NativeFlintDatagramSocketImpl_Bind(FNIEnv *env, jobject obj, jint lport, jobject laddr) {
    (void)env;
    (void)obj;
    (void)lport;
    (void)laddr;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "Network system not supported");
    return;
}

jvoid NativeFlintDatagramSocketImpl_Send(FNIEnv *env, jobject obj, jobject p) {
    (void)env;
    (void)obj;
    (void)p;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "Network system not supported");
    return;
}

jint NativeFlintDatagramSocketImpl_Peek(FNIEnv *env, jobject obj, jobject i) {
    (void)env;
    (void)obj;
    (void)i;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "Network system not supported");
    return -1;
}

jvoid NativeFlintDatagramSocketImpl_Receive(FNIEnv *env, jobject obj, jobject p) {
    (void)env;
    (void)obj;
    (void)p;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "Network system not supported");
    return;
}

jvoid NativeFlintDatagramSocketImpl_SetTTL(FNIEnv *env, jobject obj, jbyte ttl) {
    (void)env;
    (void)obj;
    (void)ttl;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "Network system not supported");
    return;
}

jbyte NativeFlintDatagramSocketImpl_GetTTL(FNIEnv *env, jobject obj) {
    (void)env;
    (void)obj;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "Network system not supported");
    return -1;
}

jvoid NativeFlintDatagramSocketImpl_Join(FNIEnv *env, jobject obj, jobject inetaddr) {
    (void)env;
    (void)obj;
    (void)inetaddr;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "Network system not supported");
    return;
}

jvoid NativeFlintDatagramSocketImpl_Leave(FNIEnv *env, jobject obj, jobject inetaddr) {
    (void)env;
    (void)obj;
    (void)inetaddr;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "Network system not supported");
    return;
}

jvoid NativeFlintDatagramSocketImpl_DatagramSocketCreate(FNIEnv *env, jobject obj) {
    (void)env;
    (void)obj;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "Network system not supported");
    return;
}

jvoid NativeFlintDatagramSocketImpl_DatagramSocketClose(FNIEnv *env, jobject obj) {
    (void)env;
    (void)obj;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "Network system not supported");
    return;
}

jvoid NativeFlintDatagramSocketImpl_SocketSetOption(FNIEnv *env, jobject obj, jint opt, jobject val) {
    (void)env;
    (void)obj;
    (void)val;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "Network system not supported");
    return;
}

jobject NativeFlintDatagramSocketImpl_SocketGetOption(FNIEnv *env, jobject obj, jint opt) {
    (void)env;
    (void)obj;
    (void)opt;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "Network system not supported");
    return NULL;
}

#endif /* FLINT_API_NET_ENABLED */
