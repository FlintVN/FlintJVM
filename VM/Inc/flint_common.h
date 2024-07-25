
#ifndef __FLINT_COMMON_H
#define __FLINT_COMMON_H

#include <stdint.h>

#define KILO_BYTE(value)        (value * 1024)
#define MEGA_BYTE(value)        (value * KILO_BYTE(1024))

#define LENGTH(array)           (sizeof(array) / sizeof(array[0]))

uint16_t Flint_Swap16(uint16_t value);
uint32_t Flint_Swap32(uint32_t value);
uint64_t Flint_Swap64(uint64_t value);

uint16_t Flint_CalcCrc(const uint8_t *data, uint32_t length);

#endif /* __FLINT_COMMON_H */
