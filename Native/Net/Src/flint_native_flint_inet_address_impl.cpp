
#include <string.h>
#include "flint_system_api.h"
#include "flint_java_inet_address.h"
#include "flint_native_flint_inet_address_impl.h"

#if FLINT_API_NET_ENABLED

using namespace FlintAPI::Net;

jstring NativeFlintInetAddressImpl_GetLocalHostName(FNIEnv *env, jobject obj) {
    (void)obj;
    const char *hostname = getLocalHostName();
    if(hostname == NULL) {
        env->throwNew(env->findClass("java/net/UnknownHostException"));
        return NULL;
    }
    return env->newString(hostname);
}

jobjectArray NativeFlintInetAddressImpl_LookupAllHostAddr(FNIEnv *env, jobject obj, jstring hostname) {
    (void)obj;
    char buff[FILE_NAME_BUFF_SIZE];

    uint32_t len = hostname->getLength();
    if(len >= FILE_NAME_BUFF_SIZE) {
        jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
        env->throwNew(excpCls, "hostname cannot exceed %d characters", FILE_NAME_BUFF_SIZE - 1);
        return NULL;
    }
    memcpy(buff, hostname->getAscii(), len);
    buff[len] = 0;

    AddrInfo res = getAddrInfo(buff, NULL);
    if(res == 0) {
        env->throwNew(env->findClass("java/net/UnknownHostException"));
        return NULL;
    }

    uint32_t count = 0;
    for(AddrInfo p = res; p != NULL; p = nextAddrInfo(p)) count++;

    jobjectArray array = env->newObjectArray(env->findClass("java/net/InetAddress"), count);
    if(array == NULL) {
        freeAddrInfo(res);
        return NULL;
    }

    count = 0;
    for(AddrInfo p = res; p != NULL; p = nextAddrInfo(p)) {
        if(getAddrFamily(p) == NET_INET4) {
            Inet4Address *inetAddr = (Inet4Address *)env->newObject(env->findClass("java/net/Inet4Address"));
            if(inetAddr == NULL) break;

            inetAddr->setHostName(hostname);
            inetAddr->setFamily(NET_INET4);
            inetAddr->setAddress(getIPv4Addr(p));

            array->getData()[count++] = inetAddr;
        }
        else {
            Inet6Address *inetAddr = (Inet6Address *)env->newObject(env->findClass("java/net/Inet6Address"));
            jbyteArray byteArr = env->newByteArray(16);
            if(byteArr == NULL || inetAddr == NULL) {
                if(inetAddr != NULL) env->freeObject(inetAddr);
                if(byteArr != NULL) env->freeObject(byteArr);
                break;
            }

            uint32_t scopeId;
            getIPv6Addr(p, (uint8_t *)byteArr->getData(), &scopeId);

            inetAddr->setHostName(hostname);
            inetAddr->setFamily(NET_INET6);
            inetAddr->setAddress(byteArr);
            byteArr->clearProtected();
            inetAddr->setScopeId(scopeId);
            if(scopeId > 0)
                inetAddr->setScopeIdSet(true);

            array->getData()[count++] = inetAddr;
        }
    }

    freeAddrInfo(res);
    if(count != array->getLength()) {
        for(uint32_t i = 0; i < count; i++)
            env->freeObject(array->getData()[i]);
        env->freeObject(array);
        return NULL;
    }
    return array;
}

jstring NativeFlintInetAddressImpl_GetHostByAddr(FNIEnv *env, jobject obj, jbyteArray addr) {
    (void)obj;
    uint32_t len = addr->getLength();
    uint8_t *data = (uint8_t *)addr->getData();
    if(len == 4)
        return env->newString("%d.%d.%d.%d", data[0], data[1], data[2], data[3]);
    else {
        uint16_t a1 = (data[0] << 8) | data[1];
        uint16_t a2 = (data[2] << 8) | data[3];
        uint16_t a3 = (data[4] << 8) | data[5];
        uint16_t a4 = (data[6] << 8) | data[7];
        uint16_t a5 = (data[8] << 8) | data[9];
        uint16_t a6 = (data[10] << 8) | data[11];
        uint16_t a7 = (data[12] << 8) | data[13];
        uint16_t a8 = (data[14] << 8) | data[15];
        return env->newString("[%x:%x:%x:%x:%x:%x:%x:%x]", a1, a2, a3, a4, a5, a6, a7, a8);
    }
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"));
    return NULL;
}

jobject NativeFlintInetAddressImpl_AnyLocalAddress(FNIEnv *env, jobject obj) {
    (void)obj;

    Inet4Address *inetAddr = (Inet4Address *)env->newObject(env->findClass("java/net/Inet4Address"));
    jstring hostname = env->newString("0.0.0.0");

    if(inetAddr == NULL || hostname == NULL) {
        if(inetAddr != NULL) env->freeObject(inetAddr);
        if(hostname != NULL) env->freeObject(hostname);
        return NULL;
    }

    inetAddr->setHostName(hostname);
    inetAddr->setFamily(NET_INET4);
    inetAddr->setAddress(0);

    return inetAddr;
}

#endif /* FLINT_API_NET_ENABLED */
