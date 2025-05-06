
#ifndef __FLINT_COMMON_H
#define __FLINT_COMMON_H

#include <stdint.h>

#define FLINT_VERSION_MAJOR         1
#define FLINT_VERSION_MINOR         1
#define FLINT_VERSION_PATCH         4

#define KILO_BYTE(_value)           ((_value) * 1024)
#define MEGA_BYTE(_value)           ((_value) * KILO_BYTE(1024))

#define FLINT_MAX(_a, _b)           ((_a) > (_b) ? (_a) : (_b))
#define FLINT_MIN(_a, _b)           ((_a) < (_b) ? (_a) : (_b))
#define FLINT_ABS(_val)             (((_val) < 0) ? -(_val) : (_val))
#define FLINT_SWAP(_a, _b) {        \
    int32_t tmp = _a;               \
    _a = _b;                        \
    _b = tmp;                       \
}

#define LENGTH(_array)              (sizeof(_array) / sizeof(_array[0]))

#define RETURN_IF_ERR(expr)         if(FlintError _err = (expr); _err != ERR_OK) return (_err)
#define RETURN_IF_NOT_THROW(expr)   if(FlintError _err = (expr); _err != ERR_THROW) return (_err)

#define SWAP16(value)               (((value) << 8) | ((value) >> 8))
uint32_t SWAP32(uint32_t value);
uint64_t SWAP64(uint64_t value);

uint16_t Flint_CalcCrc(const uint8_t *data, uint32_t length);
uint32_t Flint_CalcHash(const char *text, uint32_t length, bool isTypeName);
uint32_t Flint_HashIndex(uint32_t hash, uint32_t hashTableLength);

int64_t Flint_GetUnixTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);

#endif /* __FLINT_COMMON_H */
