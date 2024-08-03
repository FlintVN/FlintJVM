
#ifndef __FLINT_SYSTEM_API_H
#define __FLINT_SYSTEM_API_H

#include <stdint.h>
#include "flint_system_type.h"

namespace FlintAPI {
    namespace System {
        void *malloc(uint32_t size);
        void *realloc(void *p, uint32_t size);
        void free(void *p);
        void print(const char *text, uint32_t length, uint8_t coder);
        int64_t getNanoTime(void);
    };

    namespace File {
        void *open(const char *fileName, FlintFileMode mode);
        FlintFileResult read(void *fileHandle, void *buff, uint32_t btr, uint32_t *br);
        FlintFileResult write(void *fileHandle, void *buff, uint32_t btw, uint32_t *bw);
        uint32_t size(void *fileHandle);
        uint32_t tell(void *fileHandle);
        FlintFileResult seek(void *fileHandle, uint32_t offset);
        FlintFileResult close(void *fileHandle);
    };

    namespace Thread {
        void *create(void (*task)(void *), void *param, uint32_t stackSize = 0);
        void terminate(void *threadHandle);
        void sleep(uint32_t ms);
    };
};

#endif /* __FLINT_SYSTEM_API_H */
