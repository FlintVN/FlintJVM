
#ifndef __FLINT_COMMON_H
#define __FLINT_COMMON_H

#include <stdint.h>

#define FLINT_VERSION_MAJOR     1
#define FLINT_VERSION_MINOR     1
#define FLINT_VERSION_PATCH     1

#define KILO_BYTE(_value)       ((_value) * 1024)
#define MEGA_BYTE(_value)       ((_value) * KILO_BYTE(1024))

#define FLINT_MAX(_a, _b)       ((_a) > (_b) ? (_a) : (_b))
#define FLINT_MIN(_a, _b)       ((_a) < (_b) ? (_a) : (_b))
#define FLINT_ABS(_val)         (((_val) < 0) ? -(_val) : (_val))
#define FLINT_SWAP(_a, _b) {    \
    int32_t tmp = _a;           \
    _a = _b;                    \
    _b = tmp;                   \
}

#define LENGTH(_array)          (sizeof(_array) / sizeof(_array[0]))

uint16_t Flint_Swap16(uint16_t value);
uint32_t Flint_Swap32(uint32_t value);
uint64_t Flint_Swap64(uint64_t value);

uint16_t Flint_CalcCrc(const uint8_t *data, uint32_t length);

int64_t Flint_GetUnixTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);

#endif /* __FLINT_COMMON_H */
