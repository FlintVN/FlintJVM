
#ifndef __FLINT_SYSTEM_API_H
#define __FLINT_SYSTEM_API_H

#include <stdint.h>
#include "flint_method_info.h"
#include "flint_attribute_info.h"

#define	FLINT_FILE_ATTR_RDO         0x01
#define	FLINT_FILE_ATTR_HID         0x02
#define	FLINT_FILE_ATTR_SYS         0x04
#define FLINT_FILE_ATTR_DIR         0x10
#define FLINT_FILE_ATTR_ARC         0x20

typedef enum : uint8_t {
    FLINT_FILE_OPEN_EXISTING = 0x00,
    FLINT_FILE_READ = 0x01,
    FLINT_FILE_WRITE = 0x02,
    FLINT_FILE_CREATE_NEW = 0x04,
    FLINT_FILE_CREATE_ALWAYS = 0x08,
    FLINT_FILE_OPEN_ALWAYS = 0x10
} FlintFileMode;

typedef enum : uint8_t {
    FILE_RESULT_OK = 0,
    FILE_RESULT_ERR,
    FILE_RESULT_NO_PATH,
    FILE_RESULT_DENIED,
    FILE_RESULT_WRITE_PROTECTED
} FlintFileResult;

class Flint;

namespace FlintAPI {
    namespace System {
        void reset(Flint &flint);
        void *malloc(uint32_t size);
        void *realloc(void *p, uint32_t size);
        void free(void *p);
        void print(const char *text, uint32_t length, uint8_t coder);
        uint64_t getNanoTime(void);
        FlintNativeMethodPtr findNativeMethod(const FlintMethodInfo &methodInfo);
    };

    namespace IO {
        FlintFileResult finfo(const char *fileName, uint32_t *size, int64_t *time);
        void *fopen(const char *fileName, FlintFileMode mode);
        FlintFileResult fread(void *handle, void *buff, uint32_t btr, uint32_t *br);
        FlintFileResult fwrite(void *handle, void *buff, uint32_t btw, uint32_t *bw);
        uint32_t fsize(void *handle);
        uint32_t ftell(void *handle);
        FlintFileResult fseek(void *handle, uint32_t offset);
        FlintFileResult fclose(void *handle);
        FlintFileResult fremove(const char *fileName);

        void *opendir(const char *dirName);
        FlintFileResult readdir(void *handle, uint8_t *attribute, char *nameBuff, uint32_t buffSize, uint32_t *size, int64_t *time);
        FlintFileResult closedir(void *handle);
        FlintFileResult mkdir(const char *path);
    };

    namespace Thread {
        typedef struct {
            void *mutexHandle;
            void *lockThreadId;
            uint32_t lockNest;
        } LockHandle;

        LockHandle *createLockHandle(void);
        void lock(LockHandle *lockHandle);
        void unlock(LockHandle *lockHandle);
        void *create(void (*task)(void *), void *param, uint32_t stackSize = 0);
        void terminate(void *threadHandle);
        void sleep(uint32_t ms);
        void yield(void);
    };
};

#endif /* __FLINT_SYSTEM_API_H */
