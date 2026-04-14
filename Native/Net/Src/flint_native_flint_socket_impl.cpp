
#include <string.h>
#include "flint_system_api.h"
#include "flint_java_inet_address.h"
#include "flint_native_flint_socket_impl.h"

#if FLINT_API_NET_ENABLED

#define NATIVE_TCP_NODELAY      0x0001
#define NATIVE_SO_BINDADDR      0x000F
#define NATIVE_SO_REUSEADDR     0x04
#define NATIVE_IP_MULTICAST_IF  0x10
#define NATIVE_SO_LINGER        0x0080
#define NATIVE_SO_TIMEOUT       0x1006

using namespace FlintAPI::Net;
using namespace FlintAPI::System;

jbool ConvertToSockAddr(InetAddress *inetAddr, jint port, SockAddr *addr) {
    int32_t family = inetAddr->getFamily();
    if(family != NET_INET4 && family != NET_INET6)
        return false;

    addr->port = port;
    if(family == NET_INET4) {
        Inet4Address *inet4Addr = (Inet4Address *)inetAddr;
        memset(addr->addr, 0, 10);
        addr->addr[10] = 0xFF;
        addr->addr[11] = 0xFF;
        addr->addr[12] = inet4Addr->getAddress() >> 24;
        addr->addr[13] = inet4Addr->getAddress() >> 16;
        addr->addr[14] = inet4Addr->getAddress() >> 8;
        addr->addr[15] = inet4Addr->getAddress() >> 0;
        addr->scopeId = 0;
    }
    else {
        Inet6Address *inet6Addr = (Inet6Address *)inetAddr;
        memcpy(addr->addr, inet6Addr->getAddress()->getData(), 16);
        if(inet6Addr->getScopeIdSet())
            addr->scopeId = inet6Addr->getScopeId();
        else
            addr->scopeId = 0;
    }

    return true;
}

Hook *NativeFlintSocketImpl_GetHook(FNIEnv *env, jobject socketObj, bool throwable) {
    jobject fdObj = env->getObjField(env->getFieldId(socketObj, "fd"));
    if(fdObj == NULL) {
        if(throwable)
            env->throwNew(env->findClass("java/io/IOException"), "Socket has not been created");
        return NULL;
    }
    int32_t fd = env->getIntField(env->getFieldId(fdObj, "fd"));
    if(fd == -1 || (Hook *)fd == NULL) {
        if(throwable)
            env->throwNew(env->findClass("java/io/IOException"), "Socket has not been created");
        return NULL;
    }
    return (Hook *)fd;
}

jint NativeFlintSocketImpl_GetSock(FNIEnv *env, jobject socketObj, jbool throwable) {
    Hook *hook = NativeFlintSocketImpl_GetHook(env, socketObj, throwable);
    if(hook == NULL) return -1;
    return (int32_t)hook->getHandle();
}

static bool IsIPv4MappedAddress(uint8_t *addr) {
    if(
        (addr[0] == 0x00) && (addr[1] == 0x00) &&
        (addr[2] == 0x00) && (addr[3] == 0x00) &&
        (addr[4] == 0x00) && (addr[5] == 0x00) &&
        (addr[6] == 0x00) && (addr[7] == 0x00) &&
        (addr[8] == 0x00) && (addr[9] == 0x00) &&
        (addr[10] == 0xFF) && (addr[11] == 0xFF)
    ) {
        return true;
    }
    return false;
}

InetAddress *NativeFlintSocketImpl_CreateInetAddress(FNIEnv *env, SockAddr *addr) {
    if(IsIPv4MappedAddress(addr->addr)) {
        int32_t ipv4 = addr->addr[12] << 24;
        ipv4 |= addr->addr[13] << 16;
        ipv4 |= addr->addr[14] << 8;
        ipv4 |= addr->addr[15] << 0;

        Inet4Address *inetAddr = (Inet4Address *)env->newObject(env->findClass("java/net/Inet4Address"));
        if(inetAddr == NULL) return NULL;

        inetAddr->setHostName(NULL);
        inetAddr->setFamily(NET_INET4);
        inetAddr->setAddress(ipv4);

        return inetAddr;
    }
    else {
        Inet6Address *inetAddr = (Inet6Address *)env->newObject(env->findClass("java/net/Inet6Address"));
        jbyteArray byteArr = env->newByteArray(16);
        if(byteArr == NULL || inetAddr == NULL) {
            if(inetAddr != NULL) env->freeObject(inetAddr);
            if(byteArr != NULL) env->freeObject(byteArr);
            return NULL;
        }
        memcpy(byteArr->getData(), addr->addr, 16);

        inetAddr->setHostName(NULL);
        inetAddr->setFamily(NET_INET6);
        inetAddr->setAddress(byteArr);
        inetAddr->setScopeId(addr->scopeId);
        if(addr->scopeId > 0)
            inetAddr->setScopeIdSet(true);
        return inetAddr;
    }
}

void NativeFlintSocketImpl_SocketClose(void *handle) {
    close((int32_t)handle);
}

jvoid NativeFlintSocketImpl_SocketCreate(FNIEnv *env, jobject obj) {
    FExec *exec = (FExec *)env;
    int32_t sock = socket(true);
    Hook *hook = sock == -1 ? NULL : exec->getFlint()->addShutdownHook(exec, (void *)sock, NativeFlintSocketImpl_SocketClose);
    if(hook == NULL) {
        if(sock != -1) close(sock);
        env->throwNew(env->findClass("java/io/IOException"), "Create socket error");
        return;
    }
    jobject fdObj = env->getObjField(env->getFieldId(obj, "fd"));
    env->setIntField(env->getFieldId(fdObj, "fd"), (int32_t)hook);
}

jvoid NativeFlintSocketImpl_SocketConnect(FNIEnv *env, jobject obj, jobject address, jint port) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock == -1) return;

    InetAddress *inetAddr = (InetAddress *)address;
    SockAddr addr;
    if(!ConvertToSockAddr(inetAddr, port, &addr)) {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid address");
        return;
    }

    SockError err = connect(sock, &addr);
    if(err == SOCK_OK) return;
    else if(err == SOCK_INPROGRESS) {
        while(!env->hasTerminateRequest()) {
            bool connected;
            if(isConnected(sock, &connected) != SOCK_OK)
                break;
            if(connected == true)
                return;
        }
    }
    env->throwNew(env->findClass("java/io/IOException"), "Connect error");
}

jvoid NativeFlintSocketImpl_SocketBind(FNIEnv *env, jobject obj, jobject address, jint port) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock == -1) return;

    InetAddress *inetAddr = (InetAddress *)address;
    SockAddr addr;
    if(!ConvertToSockAddr(inetAddr, port, &addr)) {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid address");
        return;
    }
    if(bind(sock, &addr) != SOCK_OK)
        env->throwNew(env->findClass("java/io/IOException"), "Bind error");
}

jvoid NativeFlintSocketImpl_SocketListen(FNIEnv *env, jobject obj, jint count) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock == -1) return;

    if(listen(sock, count) != SOCK_OK)
        env->throwNew(env->findClass("java/io/IOException"), "Listen error");
}

jvoid NativeFlintSocketImpl_SocketAccept(FNIEnv *env, jobject obj, jobject s) {
    int32_t listenSock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(listenSock == -1) return;

    SockAddr addr;
    int32_t timeout = env->getIntField(env->getFieldId(obj, "timeout"));
    uint64_t startTime = getTimeMillis();

    while(!env->hasTerminateRequest() && (timeout <= 0 || ((uint64_t)(getTimeMillis() - startTime)) < timeout)) {
        int32_t client;
        SockError err = accept(listenSock, &addr, &client);
        if(err == SOCK_ERR) {
            env->throwNew(env->findClass("java/io/IOException"), "Accept error");
            return;
        }
        else if(err == SOCK_OK) {
            FExec *exec = (FExec *)env;
            Hook *hook = exec->getFlint()->addShutdownHook(exec, (void *)client, NativeFlintSocketImpl_SocketClose);
            if(hook == NULL) {
                env->throwNew(env->findClass("java/io/IOException"), "Accept error");
                close(client);
                return;
            }
            InetAddress *inetAddr = NativeFlintSocketImpl_CreateInetAddress(env, &addr);
            if(inetAddr == NULL) return;
            env->setObjField(env->getFieldId(s, "address"), inetAddr);
            inetAddr->clearProtected();
            if(inetAddr->getFamily() == NET_INET6)
                ((Inet6Address *)inetAddr)->getAddress()->clearProtected();
            env->setIntField(env->getFieldId(env->getObjField(env->getFieldId(s, "fd")), "fd"), (int32_t)hook);
            env->setIntField(env->getFieldId(s, "port"), env->getIntField(env->getFieldId(obj, "port")));
            env->setIntField(env->getFieldId(s, "localport"), env->getIntField(env->getFieldId(obj, "localport")));
            return;
        }
    }
    if(!env->hasTerminateRequest())
        env->throwNew(env->findClass("java/net/SocketTimeoutException"), "Accept timed out");
}

jint NativeFlintSocketImpl_SocketAvailable(FNIEnv *env, jobject obj) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock == -1) return -1;
    return available(sock);
}

jvoid NativeFlintSocketImpl_SocketClose(FNIEnv *env, jobject obj) {
    Hook *hook = NativeFlintSocketImpl_GetHook(env, obj, false);
    if(hook == NULL) return;
    ((FExec *)env)->getFlint()->removeShutdownHook(hook);
    NativeFlintSocketImpl_SocketClose(hook->getHandle());
}

jvoid NativeFlintSocketImpl_InitProto(FNIEnv *env) {
    /* Do nothing */
}

jvoid NativeFlintSocketImpl_SocketSetOption(FNIEnv *env, jobject obj, jint cmd, jbool on, jobject value) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock == -1) return;

    switch(cmd) {
        case NATIVE_TCP_NODELAY: {
            if(setSockOpt(sock, SOCK_TCP_NODELAY, on, NULL) != SOCK_OK)
                env->throwNew(env->findClass("java/io/IOException"), "Set TCP_NODELAY error");
            break;
        }
        case NATIVE_SO_LINGER: {
            int32_t val = (value != NULL) ? value->getFieldByIndex(0)->getInt32() : 0;
            if(setSockOpt(sock, SOCK_SO_LINGER, on, &val) != SOCK_OK)
                env->throwNew(env->findClass("java/io/IOException"), "Set SO_LINGER error");
            break;
        }
        default:
            env->throwNew(env->findClass("java/io/IOException"), "Unsupported socket option");
            break;
    }
}

jobject NativeFlintSocketImpl_SocketGetOption(FNIEnv *env, jobject obj, jint opt) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock == -1) return NULL;

    switch(opt) {
        case NATIVE_SO_TIMEOUT: {
            jobject ret = env->newObject(env->findClass("java/lang/Integer"));
            if(ret == NULL) return NULL;
            ret->getFieldByIndex(0)->setInt32(env->getIntField(env->getFieldId(obj, "timeout")));
            return ret;
        }
        case NATIVE_TCP_NODELAY: {
            bool val = 0;
            if(getSockOpt(sock, SOCK_TCP_NODELAY, &val, NULL) != SOCK_OK) {
                env->throwNew(env->findClass("java/io/IOException"), "Get TCP_NODELAY error");
                return NULL;
            }
            jobject ret = env->newObject(env->findClass("java/lang/Integer"));
            if(ret == NULL) return NULL;
            ret->getFieldByIndex(0)->setInt32(val);
            return ret;
        }
        case NATIVE_SO_LINGER: {
            bool on;
            int32_t linger;
            if(getSockOpt(sock, SOCK_SO_LINGER, &on, &linger) != SOCK_OK) {
                env->throwNew(env->findClass("java/io/IOException"), "Get SO_LINGER error");
                return NULL;
            }
            if(on == 0) {
                jobject ret = env->newObject(env->findClass("java/lang/Boolean"));
                if(ret == NULL) return NULL;
                ret->getFieldByIndex(0)->setInt32(0);
                return ret;
            }
            else {
                jobject ret = env->newObject(env->findClass("java/lang/Integer"));
                if(ret == NULL) return NULL;
                ret->getFieldByIndex(0)->setInt32(linger);
                return ret;
            }
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

#endif /* FLINT_API_NET_ENABLED */
