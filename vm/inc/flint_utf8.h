
#ifndef __FLINT_HUT8_H
#define __FLINT_HUT8_H

#include "flint_std.h"

uint8_t Utf8DecodeSizeOneChar(char c);
uint8_t Utf8EncodeSize(uint16_t c);
uint32_t Utf8DecodeOneChar(const char *c);
uint8_t Utf8EncodeOneChar(uint16_t c, char *buff);
uint32_t Utf8StrLen(const char *utf8);

#endif /* __FLINT_HUT8_H */
