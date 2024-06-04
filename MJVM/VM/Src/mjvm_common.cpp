
#include "mjvm_common.h"

uint16_t swap16(uint16_t value) {
    return (value << 8) | (value >> 8);
}

uint32_t swap32(uint32_t value) {
    uint32_t ret;
    ((uint8_t *)&ret)[0] = ((uint8_t *)&value)[3];
    ((uint8_t *)&ret)[1] = ((uint8_t *)&value)[2];
    ((uint8_t *)&ret)[2] = ((uint8_t *)&value)[1];
    ((uint8_t *)&ret)[3] = ((uint8_t *)&value)[0];
    return ret;
}

uint64_t swap64(uint64_t value) {
    uint64_t ret;
    ((uint8_t *)&ret)[0] = ((uint8_t *)&value)[7];
    ((uint8_t *)&ret)[1] = ((uint8_t *)&value)[6];
    ((uint8_t *)&ret)[2] = ((uint8_t *)&value)[5];
    ((uint8_t *)&ret)[3] = ((uint8_t *)&value)[4];
    ((uint8_t *)&ret)[4] = ((uint8_t *)&value)[3];
    ((uint8_t *)&ret)[5] = ((uint8_t *)&value)[2];
    ((uint8_t *)&ret)[6] = ((uint8_t *)&value)[1];
    ((uint8_t *)&ret)[7] = ((uint8_t *)&value)[0];
    return ret;
}

uint16_t calcCrc(const uint8_t *data, uint32_t length) {
    uint16_t ret = 0;
    for(int i = 0; i < length; i++)
        ret += data[i];
    return ret;
}
