
#ifndef __FLINT_COMMON_H
#define __FLINT_COMMON_H

#include "flint_std.h"

#define FLINT_VERSION_MAJOR         2
#define FLINT_VERSION_MINOR         0
#define FLINT_VERSION_PATCH         1

#define LENGTH(_array)              (sizeof(_array) / sizeof(_array[0]))

constexpr uint32_t Hash(const char *txt, uint16_t length = 0xFFFF, uint32_t initValue = 0) {
    uint16_t index = 0;
    uint32_t h = initValue;
    while(txt[index] && index < length) {
        h = 31 * h + (uint8_t)txt[index];
        index++;
    }
    return h;
}

uint16_t Swap16(uint16_t value);
uint32_t Swap32(uint32_t value);
uint64_t Swap64(uint64_t value);

uint16_t Crc(const uint8_t *data, uint32_t length);

int64_t UnixTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);

#endif /* __FLINT_COMMON_H */
