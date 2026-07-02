
#include <cstdlib>
#include "flint_fixed_point.h"

const FP FP::ONE(1);
const FP FP::HALF(1 << (FP_PRECISION - 1), true);

FP sqrt(const FP x) {
    uint64_t n = (uint64_t)x.value << FP_PRECISION;
    uint64_t g, y = 1ull << ((63 - __builtin_clzll(n)) >> 1);
    do {
        g = y;
        y = (g + n / g) >> 1;
    } while(std::abs((int64_t)(y - g)) > 1);
    return FP((uint32_t)y, true);
}

FP FP::sqrt(void) const {
    uint64_t n = (uint64_t)value << FP_PRECISION;
    uint64_t g, y = 1ull << ((63 - __builtin_clzll(n)) >> 1);
    do {
        g = y;
        y = (g + n / g) >> 1;
    } while(std::abs((int64_t)(y - g)) > 1);
    return FP((uint32_t)y, true);
}
