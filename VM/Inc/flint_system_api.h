
#ifndef __FLINT_SYSTEM_API_H
#define __FLINT_SYSTEM_API_H

#include <stdint.h>
#include "flint_native.h"
#include "flint_default_conf.h"

namespace FlintAPI::System {
    void reset(void);
    void *malloc(uint32_t size);
    void *realloc(void *p, uint32_t size);
    void free(void *p);
    void consoleWrite(uint8_t *utf8, uint32_t length);
    int64_t getTimeNanos(void);
    int64_t getTimeMillis(void);
    const char *getClassPath(uint32_t index);
    JNMPtr findNativeMethod(MethodInfo *methodInfo);
};

namespace FlintAPI::IO {
    typedef void * FileHandle;
    typedef void * DirHandle;

    typedef enum : uint8_t {
        FILE_MODE_OPEN_EXISTING = 0x00,
        FILE_MODE_READ = 0x01,
        FILE_MODE_WRITE = 0x02,
        FILE_MODE_CREATE_NEW = 0x04,
        FILE_MODE_CREATE_ALWAYS = 0x08,
        FILE_MODE_OPEN_ALWAYS = 0x10,
        FILE_MODE_APPEND = 0x30
    } FileMode;

    typedef enum : uint8_t {
        FILE_RESULT_OK = 0,
        FILE_RESULT_ERR,
        FILE_RESULT_NO_PATH,
        FILE_RESULT_DENIED,
        FILE_RESULT_WRITE_PROTECTED
    } FileResult;

    typedef struct {
        union {
            uint8_t attribute;
            struct {
                uint8_t readOnly : 1;
                uint8_t hidden : 1;
                uint8_t system : 1;
                uint8_t archive : 1;
                uint8_t directory : 1;
            };
        };
        char name[FILE_NAME_BUFF_SIZE];
        uint32_t size;
        uint64_t time;
    } FileInfo;

    FileResult finfo(const char *fileName, FileInfo *fileInfo);
    FileHandle fopen(const char *fileName, FileMode mode);
    FileResult fread(FileHandle handle, void *buff, uint32_t btr, uint32_t *br);
    FileResult fwrite(FileHandle handle, void *buff, uint32_t btw, uint32_t *bw);
    uint32_t fsize(FileHandle handle);
    uint32_t ftell(FileHandle handle);
    FileResult fseek(FileHandle handle, uint32_t offset);
    FileResult fclose(FileHandle handle);
    FileResult fremove(const char *fileName);
    FileResult frename(const char *oldName, const char *newName);

    DirHandle opendir(const char *dirName);
    FileResult readdir(DirHandle handle, FileInfo *fileInfo);
    FileResult closedir(DirHandle handle);
    FileResult mkdir(const char *path);
};

namespace FlintAPI::Thread {
    typedef void * ThreadHandle;

    ThreadHandle create(void (*task)(void *), void *param, uint32_t stackSize = 0);
    ThreadHandle getCurrentThread(void);
    void terminate(ThreadHandle handle);
    void sleep(uint32_t ms);
    void yield(void);
};

#ifdef FLINT_API_NET_ENABLED
namespace FlintAPI::Net {
    typedef void * AddrInfo;

    typedef enum {
        NET_UNSPEC = 0,
        NET_INET4 = 1,
        NET_INET6 = 2
    } AddrFamily;

    typedef struct {
        uint8_t addr[16];
        uint16_t port;
        uint32_t scopeId;
    } SockAddr;

    typedef enum {
        SOCK_TCP_NODELAY,
        SOCK_SO_LINGER,
        SOCK_SO_REUSEADDR,
        SOCK_UNICAST_HOPS,
        SOCK_JOIN_GROUP,
        SOCK_LEAVE_GROUP,
        SOCK_SO_BINDADDR,
    } SockOpt;

    typedef enum {
        SOCK_OK,
        SOCK_CLOSED,
        SOCK_TIMEOUT,
        SOCK_PENDING,
        SOCK_INPROGRESS,
        SOCK_ERR,
    } SockError;

    const char *getLocalHostName(void);

    AddrInfo getAddrInfo(const char *hostname, const char *servname);
    AddrInfo nextAddrInfo(AddrInfo addrInfo);
    AddrFamily getAddrFamily(AddrInfo addrInfo);
    uint32_t getIPv4Addr(AddrInfo addrInfo);
    void getIPv6Addr(AddrInfo addrInfo, uint8_t *ip6, uint32_t *scopeId);
    void freeAddrInfo(AddrInfo addrInfo);

    int32_t socket(bool stream);
    SockError connect(int32_t sock, SockAddr *addr);
    SockError isConnected(int32_t sock, bool *connected);
    SockError bind(int32_t sock, SockAddr *addr);
    SockError listen(int32_t sock, int32_t count);
    SockError accept(int32_t server, SockAddr *addr, int32_t *client);
    SockError send(int32_t sock, uint8_t *data, uint32_t len, int32_t *sent);
    SockError sendTo(int32_t sock, SockAddr *addr, uint8_t *data, uint32_t len, int32_t *sent);
    SockError recv(int32_t sock, uint8_t *buf, uint32_t len, int32_t *read);
    SockError recvFrom(int32_t sock, SockAddr *addr, uint8_t *buf, uint32_t len, int32_t *read);
    int32_t available(int32_t sock);

    SockError getSockOpt(int32_t sock, SockOpt opt, bool *on, void *value);
    SockError setSockOpt(int32_t sock, SockOpt opt, bool on, void *value);

    void close(int32_t sock);
};
#endif /* FLINT_API_NET_ENABLED */

#endif /* __FLINT_SYSTEM_API_H */
