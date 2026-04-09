
#include <string.h>
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/netif.h"
#include "flint_system_api.h"

using namespace FlintAPI::Net;

const char *FlintAPI::Net::getLocalHostName(void) {
    if(netif_default == NULL) return NULL;
    return netif_default->hostname;
}

AddrInfo FlintAPI::Net::getAddrInfo(const char *hostname, const char *servname) {
    struct addrinfo *res;
    struct addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int status = lwip_getaddrinfo(hostname, servname, &hints, &res);
    if(status != 0) return NULL;
    return (AddrInfo)res;
}

AddrInfo FlintAPI::Net::nextAddrInfo(AddrInfo addrInfo) {
    return (AddrInfo)((struct addrinfo *)addrInfo)->ai_next;
}

AddrFamily FlintAPI::Net::getAddrFamily(AddrInfo addrInfo) {
    struct addrinfo *p = (struct addrinfo *)addrInfo;
    return p->ai_family == AF_INET ? NET_INET4 : (p->ai_family == AF_INET6 ? NET_INET6 : NET_UNSPEC);
}

uint32_t FlintAPI::Net::getIPv4Addr(AddrInfo addInfo) {
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)((struct addrinfo *)addInfo)->ai_addr;
    return ipv4->sin_addr.s_addr;
}

void FlintAPI::Net::getIPv6Addr(AddrInfo addInfo, uint8_t *ip6, uint32_t *scopeId) {
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)((struct addrinfo *)addInfo)->ai_addr;
    *scopeId = ipv6->sin6_scope_id;
    memcpy(ip6, ipv6->sin6_addr.un.u8_addr, 16);
}

void FlintAPI::Net::freeAddrInfo(AddrInfo addrInfo) {
    lwip_freeaddrinfo((struct addrinfo *)addrInfo);
}

int32_t FlintAPI::Net::socket(bool stream) {
    int32_t sock = lwip_socket(AF_INET6, stream ? SOCK_STREAM : SOCK_DGRAM, IPPROTO_IP);
    if(sock < 0) return -1;
    int flags = lwip_fcntl(sock, F_GETFL, 0);
    lwip_fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    return sock;
}

static void ConvertToIPV6(FlintAPI::Net::SockAddr *addr, struct sockaddr_in6 *ipv6) {
    memset(ipv6, 0, sizeof(*ipv6));
    ipv6->sin6_family = AF_INET6;
    ipv6->sin6_port = lwip_htons(addr->port);
    ipv6->sin6_scope_id = addr->scopeId;
    memcpy(ipv6->sin6_addr.un.u8_addr, addr->addr, 16);
}

static void ConvertToSockAddr(struct sockaddr_in6 *ipv6, FlintAPI::Net::SockAddr *addr) {
    addr->port = ipv6->sin6_port;
    addr->scopeId = lwip_ntohs(ipv6->sin6_scope_id);
    memcpy(addr->addr, ipv6->sin6_addr.un.u8_addr, 16);
}

SockError FlintAPI::Net::connect(int32_t sock, SockAddr *addr) {
    struct sockaddr_in6 ipv6;
    ConvertToIPV6(addr, &ipv6);
    int32_t ret = lwip_connect(sock, (struct sockaddr *)&ipv6, sizeof(ipv6));
    if(ret == -1) return (errno == EINPROGRESS) ? SOCK_INPROGRESS : SOCK_ERR;
    return SOCK_OK;
}

SockError FlintAPI::Net::isConnected(int32_t sock, bool *connected) {
    struct pollfd pfd = {};
    pfd.fd = sock;
    pfd.events = POLLOUT;

    int32_t ret = lwip_poll(&pfd, 1, 0);
    if(ret > 0) {
        if(pfd.revents & (POLLOUT | POLLIN)) {
            int32_t err;
            socklen_t len = sizeof(err);
            lwip_getsockopt(pfd.fd, SOL_SOCKET, SO_ERROR, &err, &len);
            if(err == 0) {
                *connected = true;
                return SOCK_OK;
            }
            return SOCK_ERR;
        }
        else if(pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
            return SOCK_ERR;
        *connected = false;
        return SOCK_OK;
    }
    else if(ret < 0 && errno != EINTR)
        return SOCK_ERR;
    *connected = false;
    return SOCK_OK;
}

SockError FlintAPI::Net::bind(int32_t sock, SockAddr *addr) {
    struct sockaddr_in6 ipv6;
    ConvertToIPV6(addr, &ipv6);
    return lwip_bind(sock, (struct sockaddr *)&ipv6, sizeof(ipv6)) == 0 ? SOCK_OK : SOCK_ERR;
}

SockError FlintAPI::Net::listen(int32_t sock, int32_t count) {
    return lwip_listen(sock, count) == 0 ? SOCK_OK : SOCK_ERR;
}

SockError FlintAPI::Net::accept(int32_t server, SockAddr *addr, int32_t *client) {
    struct sockaddr_in6 ipv6;
    socklen_t addrLen = sizeof(ipv6);
    int32_t cl = lwip_accept(server, (struct sockaddr *)&ipv6, &addrLen);
    if(cl >= 0) {
        ConvertToSockAddr(&ipv6, addr);
        *client = cl;
        return SOCK_OK;
    }
    else if(errno == EINTR || errno == EAGAIN) return SOCK_TIMEOUT;
    return SOCK_ERR;
}

SockError FlintAPI::Net::send(int32_t sock, uint8_t *data, uint32_t len, int32_t *sent) {
    *sent = lwip_send(sock, data, len, 0);
    if(*sent == 0) return SOCK_CLOSED;
    if(*sent < 0) {
        if(errno == EAGAIN || errno == EWOULDBLOCK) return SOCK_PENDING;
        return SOCK_ERR;
    }
    return SOCK_OK;
}

SockError FlintAPI::Net::sendTo(int32_t sock, SockAddr *addr, uint8_t *data, uint32_t len, int32_t *sent) {
    struct sockaddr_in6 ipv6;
    ConvertToIPV6(addr, &ipv6);
    *sent = lwip_sendto(sock, data, len, 0, (struct sockaddr *)&ipv6, sizeof(ipv6));
    if(*sent > 0) return SOCK_OK;
    else if(*sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return SOCK_PENDING;
    return SOCK_ERR;
}

SockError FlintAPI::Net::recv(int32_t sock, uint8_t *buf, uint32_t len, int32_t *read) {
    *read = lwip_recv(sock, buf, len, 0);
    if(*read > 0) return SOCK_OK;
    else if(*read == 0) return SOCK_CLOSED;
    else if(errno == EINTR || errno == EAGAIN) return SOCK_TIMEOUT;
    return SOCK_ERR;
}

SockError FlintAPI::Net::recvFrom(int32_t sock, SockAddr *addr, uint8_t *buf, uint32_t len, int32_t *read) {
    struct sockaddr_in6 ipv6;
    ConvertToIPV6(addr, &ipv6);
    socklen_t addrlen = sizeof(ipv6);
    *read = lwip_recvfrom(sock, buf, len, 0, (struct sockaddr *)&ipv6, &addrlen);
    if(*read > 0) return SOCK_OK;
    else if(*read < 0 && (errno == EINTR || errno == EAGAIN)) return SOCK_TIMEOUT;
    return SOCK_ERR;
}

int32_t FlintAPI::Net::available(int32_t sock) {
    int32_t bytes = 0;
    if(lwip_ioctl(sock, FIONREAD, &bytes) < 0) return -1;
    return bytes;
}

SockError FlintAPI::Net::getSockOpt(int32_t sock, SockOpt opt, bool *on, void *value) {
    switch(opt) {
        case SOCK_TCP_NODELAY: {
            if(on == NULL) return SOCK_ERR;
            socklen_t optlen = sizeof(int32_t);
            if(lwip_getsockopt(sock, IPPROTO_TCP, TCP_NODELAY, value, &optlen) != 0)
                return SOCK_ERR;
            *on = !!*(int32_t *)value;
            return SOCK_OK;
        }
        case SOCK_SO_LINGER: {
            if(on == NULL || value == NULL) return SOCK_ERR;
            struct linger ling = {};
            socklen_t optlen = sizeof(ling);
            if(lwip_getsockopt(sock, SOL_SOCKET, SO_LINGER, &ling, &optlen) != 0)
                return SOCK_ERR;
            *on = ling.l_onoff;
            *(int32_t *)value = ling.l_linger;
            return SOCK_OK;
        }
        case SOCK_SO_REUSEADDR: {
            if(value == NULL) return SOCK_ERR;
            socklen_t optlen = sizeof(int32_t);
            if(lwip_getsockopt(sock, SOL_SOCKET, SO_REUSEADDR, value, &optlen) != 0)
                return SOCK_ERR;
            return SOCK_OK;
        }
#ifdef IPV6_UNICAST_HOPS
        case SOCK_UNICAST_HOPS: {
            if(value == NULL) return SOCK_ERR;
            socklen_t optlen = sizeof(int32_t);
            if(lwip_getsockopt(sock, IPPROTO_IPV6, IPV6_UNICAST_HOPS, value, &optlen) != 0)
                return SOCK_ERR;
            return SOCK_OK;
        }
#endif /* IPV6_UNICAST_HOPS */
        case SOCK_SO_BINDADDR: {
            if(value == NULL) return SOCK_ERR;
            struct sockaddr_in6 ipv6;
            socklen_t addrlen = sizeof(ipv6);
            if(lwip_getsockname(sock, (struct sockaddr *)&ipv6, &addrlen) != 0)
                return SOCK_ERR;
            ConvertToSockAddr(&ipv6, (SockAddr *)value);
            return SOCK_OK;
        }
        default:
            return SOCK_ERR;
    }
}

SockError FlintAPI::Net::setSockOpt(int32_t sock, SockOpt opt, bool on, void *value) {
    switch(opt) {
        case SOCK_TCP_NODELAY: {
            int32_t tmp = on ? 1 : 0;
            return lwip_setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &tmp, sizeof(tmp)) == 0 ? SOCK_OK : SOCK_ERR;
        }
        case SOCK_SO_LINGER: {
            if(value == NULL) return SOCK_ERR;
            struct linger ling = {};
            ling.l_onoff = on ? 1 : 0;
            ling.l_linger = *(int32_t *)value;
            return lwip_setsockopt(sock, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling)) == 0 ? SOCK_OK : SOCK_ERR;
        }
        case SOCK_SO_REUSEADDR:
            if(value == NULL) return SOCK_ERR;
            return lwip_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, value, sizeof(int32_t)) == 0 ? SOCK_OK : SOCK_ERR;
#ifdef IPV6_UNICAST_HOPS
        case SOCK_UNICAST_HOPS:
            if(value == NULL) return SOCK_ERR;
            return lwip_setsockopt(sock, IPPROTO_IPV6, IPV6_UNICAST_HOPS, value, sizeof(int32_t)) == 0 ? SOCK_OK : SOCK_ERR;
#endif /* IPV6_UNICAST_HOPS */
        case SOCK_JOIN_GROUP: {
            if(value == NULL) return SOCK_ERR;
            SockAddr *addr = (SockAddr *)value;
            struct ipv6_mreq mreq6 = {};
            memcpy(mreq6.ipv6mr_multiaddr.un.u8_addr, addr->addr, 16);
            mreq6.ipv6mr_interface = addr->scopeId;
            return lwip_setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq6, sizeof(mreq6)) != 0 ? SOCK_OK : SOCK_ERR;
        }
        case SOCK_LEAVE_GROUP: {
            if(value == NULL) return SOCK_ERR;
            SockAddr *addr = (SockAddr *)value;
            struct ipv6_mreq mreq6 = {};
            memcpy(mreq6.ipv6mr_multiaddr.un.u8_addr, addr->addr, 16);
            mreq6.ipv6mr_interface = addr->scopeId;
            return lwip_setsockopt(sock, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &mreq6, sizeof(mreq6)) != 0 ? SOCK_OK : SOCK_ERR;
        }
        default:
            return SOCK_ERR;
    }
}

void FlintAPI::Net::close(int32_t sock) {
    lwip_close(sock);
}
