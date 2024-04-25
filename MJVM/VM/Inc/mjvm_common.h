
#ifndef __MJVM_COMMON_H
#define __MJVM_COMMON_H

#include <stdint.h>

#define LENGTH(array)           (sizeof(array) / sizeof(array[0]))

uint16_t swap16(uint16_t value);
uint32_t swap32(uint32_t value);
uint64_t swap64(uint64_t value);

#endif /* __MJVM_COMMON_H */
