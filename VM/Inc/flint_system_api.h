
#ifndef __FLINT_SYSTEM_API_H
#define __FLINT_SYSTEM_API_H

#include <stdint.h>
#include "flint_native.h"
#include "flint_default_conf.h"

namespace FlintAPI {
    namespace System {
        void reset(void);
        void *malloc(uint32_t size);
        void *realloc(void *p, uint32_t size);
        void free(void *p);
        void consoleWrite(uint8_t *utf8, uint32_t length);
        uint64_t getNanoTime(void);
        JNMPtr findNativeMethod(MethodInfo *methodInfo);
    };

    namespace IO {
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

    namespace Thread {
        typedef void * ThreadHandle;

        ThreadHandle create(void (*task)(void *), void *param, uint32_t stackSize = 0);
        ThreadHandle getCurrentThread(void);
        void terminate(ThreadHandle handle);
        void sleep(uint32_t ms);
        void yield(void);
    };
};

#endif /* __FLINT_SYSTEM_API_H */
