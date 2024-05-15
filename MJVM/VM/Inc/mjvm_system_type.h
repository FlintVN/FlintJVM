
#ifndef __MJVM_SYSTEM_TYPE_H
#define __MJVM_SYSTEM_TYPE_H

#include <stdint.h>

typedef enum : uint8_t {
    MJVM_FILE_OPEN_EXISTING	= 0x00,
    MJVM_FILE_READ = 0x01,
    MJVM_FILE_WRITE = 0x02,
    MJVM_FILE_CREATE_NEW = 0x04,
    MJVM_FILE_CREATE_ALWAYS	= 0x08,
    MJVM_FILE_OPEN_ALWAYS = 0x10
} MjvmSys_FileMode;

typedef enum : uint8_t {
    FILE_RESULT_OK = 0,
	FILE_RESULT_ERR,
	FILE_RESULT_NO_PATH,
    FILE_RESULT_DENIED,
    FILE_RESULT_WRITE_PROTECTED
} MjvmSys_FileResult;

#endif /* __MJVM_SYSTEM_TYPE_H */
