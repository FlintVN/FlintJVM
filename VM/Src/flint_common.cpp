
#include "flint_common.h"

uint16_t Swap16(uint16_t value) {
    return ((value) << 8) | ((value) >> 8);
}

uint32_t Swap32(uint32_t value) {
    uint32_t ret;
    ((uint8_t *)&ret)[0] = ((uint8_t *)&value)[3];
    ((uint8_t *)&ret)[1] = ((uint8_t *)&value)[2];
    ((uint8_t *)&ret)[2] = ((uint8_t *)&value)[1];
    ((uint8_t *)&ret)[3] = ((uint8_t *)&value)[0];
    return ret;
}

uint64_t Swap64(uint64_t value) {
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

int64_t UnixTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
    static const uint16_t dayCount[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    int64_t ret = dayCount[month - 1] + day;
    if(year % 4 == 0 && month > 2)
        ret++;
    if(year >= 1) {
        year--;
        ret = ret + year * 365 + year / 4 - year / 100 + year / 400;
    }
    ret *= 24 * 60 * 60;
    ret += hour * 60 * 60;
    ret += minute * 60;
    ret += second;
    ret -= 62135683200ULL; /* 00:00:00 - 1/1/1970 */
    return ret;
}
