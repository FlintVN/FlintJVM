
#include "flint_fixed_point.h"

const FP FP::ONE(1);
const FP FP::HALF(1 << (FP_PRECISION - 1), true);

static inline uint32_t sqrt32(uint32_t n) {
    if (n < 2) return n;
    uint32_t r = 1u << ((33 - __builtin_clz(n)) >> 1);
    while (true) {
        uint32_t y = (r + n / r) >> 1;
        if (y >= r) return r;
        r = y;
    }
}

static uint32_t sqrt64(uint64_t n) {
    uint64_t r = 1ull << ((65 - __builtin_clzll(n)) >> 1);
    while (true) {
        uint64_t y = (r + n / r) >> 1;
        if (y >= r) return (uint32_t)r;
        r = y;
    }
}

FP sqrt(const FP x) {
    return x.sqrt();
}

FP FP::sqrt(void) const {
    if (value <= 0) return FP();
    uint64_t n = (uint64_t)value << FP_PRECISION;
    return FP((int32_t)((n >> 32) ? sqrt64(n) : sqrt32((uint32_t)n)), true);
}
