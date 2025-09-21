
#ifndef __FLINT_SYSTEM_API_H
#define __FLINT_SYSTEM_API_H

#include <stdint.h>
#include "flint_native.h"

#define	FLINT_FILE_ATTR_RDO         0x01
#define	FLINT_FILE_ATTR_HID         0x02
#define	FLINT_FILE_ATTR_SYS         0x04
#define FLINT_FILE_ATTR_DIR         0x10
#define FLINT_FILE_ATTR_ARC         0x20

typedef void * FileHandle;
typedef void * DirHandle;
typedef void * ThreadHandle;

typedef enum : uint8_t {
    FLINT_FILE_OPEN_EXISTING = 0x00,
    FLINT_FILE_READ = 0x01,
    FLINT_FILE_WRITE = 0x02,
    FLINT_FILE_CREATE_NEW = 0x04,
    FLINT_FILE_CREATE_ALWAYS = 0x08,
    FLINT_FILE_OPEN_ALWAYS = 0x10
} FileMode;

typedef enum : uint8_t {
    FILE_RESULT_OK = 0,
    FILE_RESULT_ERR,
    FILE_RESULT_NO_PATH,
    FILE_RESULT_DENIED,
    FILE_RESULT_WRITE_PROTECTED
} FileResult;

namespace FlintAPI {
    namespace System {
        void reset(void);
        void *malloc(uint32_t size);
        void *realloc(void *p, uint32_t size);
        void free(void *p);
        void print(const char *text, uint32_t length, uint8_t coder);
        uint64_t getNanoTime(void);
        JNMPtr findNativeMethod(MethodInfo *methodInfo);
    };

    namespace IO {
        FileResult finfo(const char *fileName, uint32_t *size, int64_t *time);
        FileHandle fopen(const char *fileName, FileMode mode);
        FileResult fread(FileHandle handle, void *buff, uint32_t btr, uint32_t *br);
        FileResult fwrite(FileHandle handle, void *buff, uint32_t btw, uint32_t *bw);
        uint32_t ftell(FileHandle handle);
        FileResult fseek(FileHandle handle, uint32_t offset);
        FileResult fclose(FileHandle handle);
        FileResult fremove(const char *fileName);

        DirHandle opendir(const char *dirName);
        FileResult readdir(DirHandle handle, uint8_t *attribute, char *nameBuff, uint32_t buffSize, uint32_t *size, int64_t *time);
        FileResult closedir(DirHandle handle);
        FileResult mkdir(const char *path);
    };

    namespace Thread {
        ThreadHandle create(void (*task)(void *), void *param, uint32_t stackSize = 0);
        ThreadHandle getCurrentThread(void);
        void terminate(ThreadHandle handle);
        void sleep(uint32_t ms);
        void yield(void);
    };
};

#endif /* __FLINT_SYSTEM_API_H */
