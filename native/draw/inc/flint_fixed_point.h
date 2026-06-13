
#ifndef __FLINT_FIXED_POINT_H
#define __FLINT_FIXED_POINT_H

#include <stdint.h>
#include <math.h>

#define FP_PRECISION            4

class FP {
private:
    static constexpr uint16_t fpmax = (1 << FP_PRECISION);
    static constexpr uint16_t fphalf = (1 << (FP_PRECISION - 1));
    static constexpr uint16_t fpmask = fpmax - 1;

    int32_t value;

    constexpr inline __attribute__((always_inline)) FP(int32_t value, bool isRaw) : value(isRaw ? value : (value << FP_PRECISION)) {

    }
public:
    static const FP ONE;
    static const FP HALF;

    constexpr inline __attribute__((always_inline)) FP(void) : value(0) {

    }

    constexpr inline __attribute__((always_inline)) FP(int32_t value) : value(value << FP_PRECISION) {

    }

    constexpr inline __attribute__((always_inline)) FP(const FP &value) : value(value.value) {

    }

    constexpr inline __attribute__((always_inline)) FP &operator=(int32_t x) {
        value = x << FP_PRECISION;
        return *this;
    }

    constexpr inline __attribute__((always_inline)) FP &operator=(const FP x) {
        value = x.value;
        return *this;
    }

    constexpr inline __attribute__((always_inline)) FP &operator++(void) {
        value += (1 << FP_PRECISION);
        return *this;
    }

    constexpr inline __attribute__((always_inline)) FP operator++(int) {
        FP tmp = *this;
        value += (1 << FP_PRECISION);
        return tmp;
    }

    constexpr inline __attribute__((always_inline)) FP &operator--(void) {
        value -= (1 << FP_PRECISION);
        return *this;
    }

    constexpr inline __attribute__((always_inline)) FP operator--(int) {
        FP tmp = *this;
        value -= (1 << FP_PRECISION);
        return tmp;
    }

    friend constexpr inline __attribute__((always_inline)) FP operator+(int32_t x, const FP y) {
        return FP((x << FP_PRECISION) + y.value, true);
    }

    constexpr inline __attribute__((always_inline)) FP operator+(const FP x) const {
        return FP(value + x.value, true);
    }

    constexpr inline __attribute__((always_inline)) FP &operator+=(const FP x) {
        value += x.value;
        return *this;
    }

    constexpr inline __attribute__((always_inline)) FP operator+(int32_t x) const {
        return FP(value + (x << FP_PRECISION), true);
    }

    constexpr inline __attribute__((always_inline)) FP &operator+=(int32_t x) {
        value += (x << FP_PRECISION);
        return *this;
    }

    friend constexpr inline __attribute__((always_inline)) FP operator-(int32_t x, const FP y) {
        return FP((x << FP_PRECISION) - y.value, true);
    }

    constexpr inline __attribute__((always_inline)) FP operator-(const FP x) const {
        return FP(value - x.value, true);
    }

    constexpr inline __attribute__((always_inline)) FP &operator-=(const FP x) {
        value -= x.value;
        return *this;
    }

    constexpr inline __attribute__((always_inline)) FP operator-(int32_t x) const {
        return FP(value - (x << FP_PRECISION), true);
    }

    constexpr inline __attribute__((always_inline)) FP &operator-=(int32_t x) {
        value -= (x << FP_PRECISION);
        return *this;
    }

    friend constexpr inline __attribute__((always_inline)) FP operator*(int32_t x, const FP y) {
        return FP(x * y.value, true);
    }

    constexpr inline __attribute__((always_inline)) FP operator*(const FP x) const {
        return FP(((int64_t)value * x.value) >> FP_PRECISION, true);
    }

    constexpr inline __attribute__((always_inline)) FP &operator*=(const FP x) {
        value = ((int64_t)value * x.value) >> FP_PRECISION;
        return *this;
    }

    constexpr inline __attribute__((always_inline)) FP operator*(int32_t x) const {
        return FP(value * x, true);
    }

    constexpr inline __attribute__((always_inline)) FP &operator*=(int32_t x) {
        value = value * x;
        return *this;
    }

    friend constexpr inline __attribute__((always_inline)) FP operator/(int32_t x, const FP y) {
        return FP(((int64_t)x << (FP_PRECISION << 1)) / y.value, true);
    }

    constexpr inline __attribute__((always_inline)) FP operator/(const FP x) const {
        return FP(((int64_t)value << FP_PRECISION) / x.value, true);
    }

    constexpr inline __attribute__((always_inline)) FP &operator/=(const FP x) {
        value = ((int64_t)value << FP_PRECISION) / x.value;
        return *this;
    }

    constexpr inline __attribute__((always_inline)) FP operator/(int32_t x) const {
        return FP(value / x, true);
    }

    constexpr inline __attribute__((always_inline)) FP &operator/=(int32_t x) {
        value = value / x;
        return *this;
    }

    friend constexpr inline __attribute__((always_inline)) bool operator==(int32_t x, const FP y) {
        return (x << FP_PRECISION) == y.value;
    }

    constexpr inline __attribute__((always_inline)) bool operator==(const FP x) const {
        return value == x.value;
    }

    constexpr inline __attribute__((always_inline)) bool operator==(int32_t x) const {
        return value == (x << FP_PRECISION);
    }

    friend constexpr inline __attribute__((always_inline)) bool operator!=(int32_t x, const FP y) {
        return (x << FP_PRECISION) != y.value;
    }

    constexpr inline __attribute__((always_inline)) bool operator!=(const FP x) const {
        return value != x.value;
    }

    constexpr inline __attribute__((always_inline)) bool operator!=(int32_t x) const {
        return value != (x << FP_PRECISION);
    }

    friend constexpr inline __attribute__((always_inline)) bool operator>(int32_t x, const FP y) {
        return (x << FP_PRECISION) > y.value;
    }

    constexpr inline __attribute__((always_inline)) bool operator>(const FP x) const {
        return value > x.value;
    }

    constexpr inline __attribute__((always_inline)) bool operator>(int32_t x) const {
        return value > (x << FP_PRECISION);
    }

    friend constexpr inline __attribute__((always_inline)) bool operator>=(int32_t x, const FP y) {
        return (x << FP_PRECISION) >= y.value;
    }

    constexpr inline __attribute__((always_inline)) bool operator>=(const FP x) const {
        return value >= x.value;
    }

    constexpr inline __attribute__((always_inline)) bool operator>=(int32_t x) const {
        return value >= (x << FP_PRECISION);
    }

    friend constexpr inline __attribute__((always_inline)) bool operator<(int32_t x, const FP y) {
        return (x << FP_PRECISION) < y.value;
    }

    constexpr inline __attribute__((always_inline)) bool operator<(const FP x) const {
        return value < x.value;
    }

    constexpr inline __attribute__((always_inline)) bool operator<(int32_t x) const {
        return value < (x << FP_PRECISION);
    }

    friend constexpr inline __attribute__((always_inline)) bool operator<=(int32_t x, const FP y) {
        return (x << FP_PRECISION) <= y.value;
    }

    constexpr inline __attribute__((always_inline)) bool operator<=(const FP x) const {
        return value <= x.value;
    }

    constexpr inline __attribute__((always_inline)) bool operator<=(int32_t x) const {
        return value <= (x << FP_PRECISION);
    }

    friend constexpr inline __attribute__((always_inline)) FP square(const FP x) {
        return x.square();
    }

    constexpr inline __attribute__((always_inline)) FP square(void) const {
        return FP(((int64_t)value * value) >> FP_PRECISION, true);
    }

    friend FP sqrt(const FP x);

    FP sqrt(void) const;

    friend constexpr inline __attribute__((always_inline)) FP abs(const FP x) {
        return (x.value > 0) ? x : FP(-x.value, true);
    }

    constexpr inline __attribute__((always_inline)) FP abs(void) const {
        return (value > 0) ? *this : FP(-value, true);
    }

    friend constexpr inline __attribute__((always_inline)) FP ceil(const FP x) {
        if(x.value & fpmask)
            return FP((x.value + fpmax) & ~fpmask, true);
        return x;
    }

    constexpr inline __attribute__((always_inline)) FP ceil(void) const {
        if(value & fpmask)
            return FP((value + fpmax) & ~fpmask, true);
        return *this;
    }

    friend constexpr inline __attribute__((always_inline)) FP floor(const FP x) {
        return FP(x.value & ~fpmask, true);
    }

    constexpr inline __attribute__((always_inline)) FP floor(void) const {
        return FP(value & ~fpmask, true);
    }

    friend constexpr inline __attribute__((always_inline)) FP round(const FP x) {
        uint32_t f = x.value & fpmask;
        return (f >= fphalf) ? FP((x.value + fpmax) & ~fpmask, true) : FP(x.value & ~fpmask);
    }

    constexpr inline __attribute__((always_inline)) FP round(void) const {
        uint32_t f = value & ~fpmask;
        return (f >= fphalf) ? FP((value + fpmax) & ~fpmask, true) : FP(value & ~fpmask);
    }

    static inline __attribute__((always_inline)) FP absMax(const FP x, const FP y) {
        FP a = x.abs();
        FP b = y.abs();
        return a > b ? a : b;
    }

    static inline __attribute__((always_inline)) FP absMin(const FP x, const FP y) {
        FP a = x.abs();
        FP b = y.abs();
        return a < b ? a : b;
    }

    constexpr inline __attribute__((always_inline)) operator float(void) const {
        return (int32_t)*this + fraction();
    }

    constexpr inline __attribute__((always_inline)) operator double(void) const {
        return (int32_t)*this + fraction();
    }

    constexpr inline __attribute__((always_inline)) operator int32_t(void) const {
        return value >> FP_PRECISION;
    }

    constexpr inline __attribute__((always_inline)) __attribute__((used)) float toFloat(void) const {
        return (int32_t)*this + fraction();
    }

    constexpr inline __attribute__((always_inline)) __attribute__((used)) double toDouble(void) const {
        return (int32_t)*this + fraction();
    }

    constexpr inline __attribute__((always_inline)) __attribute__((used)) int32_t toInt(void) const {
        return value >> FP_PRECISION;
    }

    constexpr inline __attribute__((always_inline)) float fraction(void) const {
        return (float)(value & fpmask) / (1 << FP_PRECISION);
    }

    constexpr inline __attribute__((always_inline)) int32_t fraction(int32_t ref) const {
        return (value & fpmask) * ref / (1 << FP_PRECISION);
    }
};

#endif /* __FLINT_FIXED_POINT_H */
