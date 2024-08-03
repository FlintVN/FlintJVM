
#ifndef __FLINT_SYSTEM_TYPE_H
#define __FLINT_SYSTEM_TYPE_H

#include <stdint.h>

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

#endif /* __FLINT_SYSTEM_TYPE_H */
