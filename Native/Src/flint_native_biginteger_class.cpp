
#include <math.h>
#include <string.h>
#include <iostream>
#include "flint.h"
#include "flint_class.h"
#include "flint_const_name.h"
#include "flint_native_biginteger_class.h"

#define KARATSUBA_THRESHOLD         80

static FlintInt32Array *square(FlintExecution &execution, FlintInt32Array *x);
static FlintInt32Array *multiply(FlintExecution &execution, FlintInt32Array *x, FlintInt32Array *y);

static uint32_t bitLength(uint32_t value) {
    uint32_t len = 0;
    if(value & 0xFFFF0000) {len += 16; value >>= 16;}
    if(value & 0xFF00) {len += 8; value >>= 8;}
    if(value & 0xF0) {len += 4; value >>= 4;}
    if(value & 0x0C) {len += 2; value >>= 2;}
    if(value & 0x02) {len += 1; value >>= 1;}
    return len + (value & 0x01);
}

static uint32_t bitLength(uint32_t *value, uint32_t length) {
    if(value == NULL)
        return 0;
    while((*value == 0) && length) {
        value++;
        length--;
    }
    if(!length)
        return 0;
    return bitLength(*value) + (length - 1) * 32;
}

static bool isPowerOfTwo(uint32_t value) {
    return (value & (value - 1)) == 0;
}

static uint8_t getExponentOfTwo(uint32_t i) {
    uint32_t n = 0;
    if (i > 1 << 16) {n += 16; i >>= 16;}
    if (i > 1 <<  8) {n +=  8; i >>=  8;}
    if (i > 1 <<  4) {n +=  4; i >>=  4;}
    if (i > 1 <<  2) {n +=  2; i >>=  2;}
    return n + (i >> 1);
}

static void setValueByIntImpl(uint32_t *x, uint32_t xLen, uint32_t value) {
    memset(x, 0, (xLen - 1) * sizeof(uint32_t));
    x[xLen - 1] = value;
}

static void setValueImpl(uint32_t *x, uint32_t xLen, uint32_t *val, uint32_t valLen) {
    if(xLen > valLen) {
        memset(x, 0, (xLen - valLen) * sizeof(uint32_t));
        x += (xLen - valLen);
        xLen = valLen;
    }
    else if(valLen > xLen) {
        val += (valLen - xLen);
        valLen = xLen;
    }
    memcpy(x, val, xLen * sizeof(uint32_t));
}

static int32_t compareMagnitudeImpl(uint32_t *x, uint32_t xLen, uint32_t *y, uint32_t yLen) {
    if(x == NULL)
        xLen = 0;
    if(y == NULL)
        yLen = 0;
    while(xLen > yLen) {
        if(*x > 0)
            return 1;
        x++;
        xLen--;
    }
    while(yLen > xLen) {
        if(*y > 0)
            return -1;
        y++;
        yLen--;
    }
    for(uint32_t i = 0; i < xLen; i++) {
        if(x[i] > y[i])
            return 1;
        else if(x[i] < y[i])
            return -1;
    }
    return 0;
}

static uint32_t addImpl(uint32_t *ret, uint32_t retLen, uint32_t *x, uint32_t xLen, uint32_t *y, uint32_t yLen) {
    if(x == NULL)
        xLen = 0;
    if(y == NULL)
        yLen = 0;
    if(ret == NULL || retLen == 0)
        return (xLen || yLen) ? 1 : 0;
    int32_t index = 0;
    uint64_t sum = 0;
    uint32_t len = MIN(retLen, MIN(xLen, yLen));
    ret = &ret[retLen - 1];
    x = &x[xLen - 1];
    y = &y[yLen - 1];
    while(index < len) {
        sum = ((uint64_t)x[-index] + y[-index]) + (sum >> 32);
        ret[-index] = (uint32_t)sum;
        index++;
    }
    if(index < yLen) {
        x = y;
        xLen = yLen;
    }
    len = MIN(retLen, xLen);
    uint8_t carry = (uint8_t)(sum >> 32);
    while((index < len) && carry) {
        ret[-index] = x[-index] + 1;
        carry = (ret[-index] == 0);
        index++;
    }
    if(index < retLen) {
        if(carry) {
            ret[-index] = 1;
            carry = 0;
            index++;
        }
        while(index < retLen) {
            ret[-index] = x[-index];
            index++;
        }
    }
    return carry;
}

static void subtractImpl(uint32_t *ret, uint32_t retLen, uint32_t *big, uint32_t bigLen, uint32_t *little, uint32_t littleLen) {
    if(ret == NULL || retLen == 0)
        return;
    if(big == NULL)
        bigLen = 0;
    if(little == NULL)
        little = 0;
    int32_t index = 0;
    int64_t difference = 0;
    uint32_t len = MIN(retLen, littleLen);
    ret = &ret[retLen - 1];
    big = &big[bigLen - 1];
    little = &little[littleLen - 1];
    while(index < len) {
        difference = ((uint64_t)big[-index] - little[-index]) + (difference >> 32);
        ret[-index] = (uint32_t)difference;
        index++;
    }
    len = MIN(retLen, bigLen);
    uint8_t borrow = (difference >> 32 != 0);
    while((index < len) && borrow) {
        ret[-index] = big[-index] - 1;
        borrow = (ret[-index] == 0xFFFFFFFF);
        index++;
    }
    while(index < len) {
        ret[-index] = big[-index];
        index++;
    }
    while(index < retLen) {
        ret[-index] = 0;
        index++;
    }
}

static void shiftLeftImpl(uint32_t *ret, uint32_t retLen, uint32_t *val, uint32_t valLen, uint32_t shift) {
    int32_t index = 0;
    uint32_t nInts = shift >> 5;
    uint32_t nBits = shift & 0x1F;
    if(ret == NULL || retLen == 0)
        return;
    if(val == NULL || valLen == 0) {
        memset(ret, 0, retLen * sizeof(uint32_t));
        return;
    }
    ret = &ret[retLen - 1];
    val = &val[valLen - 1];
    uint32_t len = MIN(nInts, retLen);
    for(; index < len; index++)
        ret[-index] = 0;
    if(nBits == 0) for(uint32_t i = 0; i < valLen && index < retLen; i++) {
        ret[-index] = val[-i];
        index++;
    }
    else if(index < retLen) {
        ret[-index] = val[0] << nBits;
        index++;
        for(uint32_t i = 1; i < valLen && index < retLen; i++) {
            ret[-index] = (val[-i] << nBits) | (val[-(i - 1)] >> (32 - nBits));
            index++;
        }
        if(index < retLen) {
            ret[-index] = val[-(valLen - 1)] >> (32 - nBits);
            index++;
        }
    }
    for(; index < retLen; index++)
        ret[-index] = 0;
}

static void shiftRightImpl(uint32_t *ret, uint32_t retLen, uint32_t *val, uint32_t valLen, uint32_t shift) {
    int32_t index = 0;
    uint32_t nInts = shift >> 5;
    uint32_t nBits = shift & 0x1F;
    if(ret == NULL || retLen == 0)
        return;
    if(val == NULL || valLen == 0) {
        memset(ret, 0, retLen * sizeof(uint32_t));
        return;
    }
    ret = &ret[retLen - 1];
    val = &val[valLen - 1];
    if(nBits == 0) for(uint32_t i = nInts; i < valLen && index < retLen; i++) {
        ret[-index] = val[-i];
        index++;
    }
    else {
        for(uint32_t i = nInts; i < (valLen - 1) && index < retLen; i++) {
            ret[-index] = (val[-i] >> nBits) | (val[-(i + 1)] << (32 - nBits));
            index++;
        }
        if(index < retLen) {
            ret[-index] = val[-(valLen - 1)] >> nBits;
            index++;
        }
    }
    for(; index < retLen; index++)
        ret[-index] = 0;
}

static void multiplyBasicImpl(uint32_t *ret, uint32_t retLen, uint32_t *x, uint32_t xLen, uint32_t *y, uint32_t yLen) {
    if(ret == NULL || retLen == 0)
        return;
    if(x == NULL || y == NULL || xLen == 0 || yLen == 0) {
        memset(ret, 0, retLen * sizeof(uint32_t));
        return;
    }
    uint32_t xBitLen = bitLength(x, xLen);
    if((xBitLen <= 32) || (bitLength(y, yLen) <= 32)) {
        if(xBitLen <= 32) {
            uint32_t tmp = x[xLen - 1];
            if((tmp & (tmp - 1)) == 0) {
                shiftLeftImpl(ret, retLen, y, yLen, getExponentOfTwo(tmp));
                return;
            }
        }
        else {
            uint32_t tmp = y[yLen - 1];
            if((tmp & (tmp - 1)) == 0) {
                shiftLeftImpl(ret, retLen, x, xLen, getExponentOfTwo(tmp));
                return;
            }
        }
    }
    memset(ret, 0, retLen * sizeof(uint32_t));
    x = &x[xLen - 1];
    y = &y[yLen - 1];
    ret = &ret[retLen - 1];
    uint32_t carry;
    for(int32_t i = 0; i < yLen; i++) {
        carry = 0;
        for(int32_t j = 0; (j < xLen) && ((i + j) < retLen); j++) {
            uint64_t product = (uint64_t)y[-i] * x[-j] + ret[-(i + j)] + carry;
            ret[-(i + j)] = (uint32_t)product;
            carry = (uint32_t)(product >> 32);
        }
        if((i + xLen) < retLen)
            ret[-(i + xLen)] = carry;
    }
    if((yLen + xLen) < retLen)
        ret[-(yLen + xLen)] = carry;
}

static uint32_t divideByIntImpl(uint32_t *ret, uint32_t retLen, uint32_t *x, uint32_t xLen, uint32_t y) {
    if(ret == NULL || retLen == 0)
        return 0;
    if(y == 0)
        throw "Divided by zero";
    if(x == 0 || xLen == 0) {
        memset(ret, 0, retLen * sizeof(uint32_t));
        return 0;
    }
    if(xLen <= 2) {
        uint64_t tmp = (xLen == 1) ? x[0] : (((uint64_t)x[0] << 32) | x[1]);
        uint32_t rem = (uint32_t)(tmp % y);
        tmp /= y;
        if(retLen > 1) ret[retLen - 2] = (uint32_t)(tmp >> 32);
        if(retLen > 0) ret[retLen - 1] = (uint32_t)tmp;
        for(int32_t i = retLen - 3; i >= 0; i--)
            ret[i] = 0;
        return rem;
    }
    else {
        uint64_t tmp;
        uint32_t i;
        int32_t retIdx = retLen - xLen;
        for(int32_t i = retIdx - 1; i >= 0; i--)
            ret[i] = 0;
        if(x[0] >= y) {
            i = 0;
            tmp = 0;
        }
        else {
            i = 1;
            tmp = x[0];
            if(retIdx >= 0)
                ret[retIdx] = 0;
            retIdx++;
        }
        for(; i < xLen; i++) {
            tmp <<= 32;
            tmp |= x[i];
            uint32_t q = (uint32_t)(tmp / y);
            tmp = tmp % y;
            if(retIdx >= 0)
                ret[retIdx] = q;
            retIdx++;
        }
        return (uint32_t)tmp;
    }
}

static void checkNullObject(FlintExecution &execution, FlintObject *obj) {
    if(obj == NULL) {
        FlintString &strObj = execution.flint.newString(STR_AND_SIZE("Cannot load from null array object"));
        throw &execution.flint.newNullPointerException(strObj);
    }
}

static void checkMakeMagnitudeParams(FlintExecution &execution, int32_t maxLen, int32_t off, int32_t len) {
    if(off < 0 || off >= maxLen || (off + len) > maxLen) {
        char indexStrBuff[11];
        char lengthStrBuff[11];
        sprintf(indexStrBuff, "%d", (int)((off < 0 || off >= maxLen) ? off : maxLen));
        sprintf(lengthStrBuff, "%d", (int)maxLen);
        const char *msg[] = {"Index ", indexStrBuff, " out of bounds for length ", lengthStrBuff};
        FlintString &strObj = execution.flint.newString(msg, LENGTH(msg));
        throw &execution.flint.newArrayIndexOutOfBoundsException(strObj);
    }
}

static FlintInt32Array *makeMagnitude(FlintExecution &execution, FlintInt8Array *val, int32_t off, uint32_t end) {
    int8_t *valData = val->getData();
    while((valData[off] == 0) && (off < end))
        off++;
    if(off == end)
        return NULL;
    uint32_t magLen = ((end - off) + 3) / sizeof(int32_t);
    FlintInt32Array &mag = execution.flint.newIntegerArray(magLen);
    uint32_t *magData = (uint32_t *)mag.getData();
    uint32_t magIndex = 0;
    mag.clearData();
    for(int32_t i = end - 1; i >= off; i--) {
        magData[magLen - magIndex / 4 - 1] |= ((uint8_t)valData[i]) << ((magIndex % 4) * 8);
        magIndex++;
    }
    return &mag;
}

static FlintInt32Array *makeMagnitude(FlintExecution &execution, FlintInt32Array *val, int32_t off, uint32_t end) {
    int32_t *valData = val->getData();
    while((valData[off] == 0) && (off < end))
        off++;
    if(off == end)
        return NULL;
    if(off == 0 && end == val->getLength())
        return val;
    uint32_t magLen = end - off;
    FlintInt32Array &mag = execution.flint.newIntegerArray(magLen);
    memcpy(mag.getData(), &val[off], magLen * sizeof(int32_t));
    return &mag;
}

static FlintInt32Array *makePositiveMagnitude(FlintExecution &execution, FlintInt8Array *val, int32_t off, uint32_t end) {
    int8_t *valData = val->getData();
    while((valData[off] == -1) && (off < end))
        off++;
    if(off == end) {
        FlintInt32Array &mag = execution.flint.newIntegerArray(1);
        mag.getData()[0] = 1;
        return &mag;
    }
    else {
        uint32_t k = off;
        while((valData[k] == 0) && (k < end))
            k++;
        uint8_t extraByte = (k == end) ? 1 : 0;
        uint32_t magLen = ((end - off) + 3 + extraByte) / sizeof(int32_t);
        FlintInt32Array &mag = execution.flint.newIntegerArray(magLen);
        uint32_t *magData = (uint32_t *)mag.getData();
        uint32_t magIndex = 0;
        mag.clearData();
        for(int32_t i = end - 1; i >= off; i--) {
            magData[magLen - magIndex / 4 - 1] |= ((uint8_t)~valData[i]) << ((magIndex % 4) * 8);
            magIndex++;
        }
        for(int32_t i = magLen - 1; i >= 0; i--) {
            magData[i] = magData[i] + 1;
            if(magData[i] != 0)
                break;
        }
        return &mag;
    }
}

static FlintInt32Array *makePositiveMagnitude(FlintExecution &execution, FlintInt32Array *val, int32_t off, uint32_t end) {
    int32_t *valData = val->getData();
    while((valData[off] == -1) && (off < end))
        off++;
    if(off == end) {
        FlintInt32Array &mag = execution.flint.newIntegerArray(1);
        mag.getData()[0] = 1;
        return &mag;
    }
    else {
        uint32_t k = off;
        while((valData[k] == 0) && (k < end))
            k++;
        uint32_t magLen = (end - off) + ((k == end) ? 1 : 0);
        FlintInt32Array &mag = execution.flint.newIntegerArray(magLen);
        uint32_t *magData = (uint32_t *)&mag.getData()[magLen - 1];
        uint8_t carry;
        int32_t index = 0;
        valData = &valData[end - 1];
        do {
            magData[-index] = -valData[-index];
            carry = !magData[-index];
            index++;
        } while((index < magLen) && carry);
        while(index < magLen) {
            magData[-index] = ~valData[-index];
            index++;
        }
        return &mag;
    }
}

static FlintInt32Array *getLower(FlintExecution &execution, FlintInt32Array *x, uint32_t n) {
    uint32_t len = x ? x->getLength() : 0;
    if(len <= n)
        return x;
    int32_t *xData = x->getData();
    while(n && (xData[len - n] == 0))
        n--;
    if(n == 0)
        return NULL;
    FlintInt32Array &lowerInts = execution.flint.newIntegerArray(n);
    memcpy(lowerInts.getData(), &xData[len - n], n * sizeof(int32_t));
    return &lowerInts;
}

static FlintInt32Array *getUpper(FlintExecution &execution, FlintInt32Array *x, uint32_t n) {
    uint32_t len = x ? x->getLength() : 0;
    if(len <= n)
        return NULL;
    if(n == 0)
        return x;
    uint32_t upperLen = len - n;
    FlintInt32Array &upperInts = execution.flint.newIntegerArray(upperLen);
    memcpy(upperInts.getData(), x->getData(), upperLen * sizeof(int32_t));
    return &upperInts;
}

static FlintInt32Array *trustedStripLeadingZeroInts(FlintExecution &execution, FlintInt32Array *value) {
    if(value == NULL)
        return NULL;
    int32_t *data = value->getData();
    if(data[0] != 0)
        return value;
    else {
        uint32_t length = value->getLength();
        uint32_t index = 1;
        for(; index < length; index++) {
            if(data[index] != 0)
                break;
        }
        if(index == length) {
            execution.flint.freeObject(*value);
            return NULL;
        }
        else {
            FlintInt32Array &newMag = execution.flint.newIntegerArray(length - index);
            memcpy(newMag.getData(), &value->getData()[index], newMag.getLength() * sizeof(int32_t));
            execution.flint.freeObject(*value);
            return &newMag;
        }
    }
}

static int32_t compareMagnitude(FlintInt32Array *x, FlintInt32Array *y) {
    if(x == y)
        return 0;
    return compareMagnitudeImpl(
        x ? (uint32_t *)x->getData() : 0, x ? x->getLength() : 0,
        y ? (uint32_t *)y->getData() : 0, y ? y->getLength() : 0
    );
}

static FlintInt32Array *add(FlintExecution &execution, FlintInt32Array *x, FlintInt32Array *y) {
    uint32_t xLen = x ? x->getLength() : 0;
    uint32_t yLen = y ? y->getLength() : 0;
    if(xLen == 0 && yLen == 0)
        return NULL;
    FlintInt32Array &ret = execution.flint.newIntegerArray(MAX(xLen, yLen));
    if(addImpl(
        (uint32_t *)ret.getData(), ret.getLength(),
        x ? (uint32_t *)x->getData() : 0, xLen,
        y ? (uint32_t *)y->getData() : 0, yLen
    )) {
        FlintInt32Array &bigger = execution.flint.newIntegerArray(ret.getLength() + 1);
        memcpy(&bigger.getData()[1], ret.getData(), ret.getLength() * sizeof(int32_t));
        bigger.getData()[0] = 1;
        execution.flint.freeObject(ret);
        return &bigger;
    }
    return &ret;
}

static FlintInt32Array *subtract(FlintExecution &execution, FlintInt32Array *big, FlintInt32Array *little) {
    if(big == NULL) {
        if(little == NULL)
            return NULL;
        FlintString &strObj = execution.flint.newString(STR_AND_SIZE("Cannot load from null array object"));
        throw &execution.flint.newNullPointerException(strObj);
    }
    uint32_t bigLen = big->getLength();
    uint32_t littleLen = little ? little->getLength() : 0;
    FlintInt32Array &ret = execution.flint.newIntegerArray(bigLen);
    subtractImpl(
        (uint32_t *)ret.getData(), ret.getLength(),
        (uint32_t *)big->getData(), bigLen,
        little ? (uint32_t *)little->getData() : 0, littleLen
    );
    return trustedStripLeadingZeroInts(execution, &ret);
}

static FlintInt32Array *shiftLeft(FlintExecution &execution, FlintInt32Array *x, uint32_t shift) {
    if(x == NULL)
        return NULL;
    uint32_t nInts = shift >> 5;
    uint32_t nBits = shift & 0x1F;
    uint32_t retLen = x->getLength() + nInts;
    retLen += nBits ? (((uint32_t)x->getData()[0] >> (32 - nBits)) ? 1 : 0) : 0;
    FlintInt32Array *ret = &execution.flint.newIntegerArray(retLen);
    shiftLeftImpl((uint32_t *)ret->getData(), retLen, (uint32_t *)x->getData(), x->getLength(), shift);
    return ret;
}

static FlintInt32Array *shiftRight(FlintExecution &execution, FlintInt32Array *x, uint32_t shift) {
    if(x == NULL)
        return NULL;
    uint32_t nInts = shift >> 5;
    uint32_t nBits = shift & 0x1F;
    uint32_t magLen = x->getLength();
    if(nInts >= magLen)
        return NULL;
    else {
        uint32_t retLen = magLen - nInts;
        retLen -= nBits ? (((uint32_t)x->getData()[0] >> nBits) ? 0 : 1) : 0;
        if(retLen == 0)
            return NULL;
        FlintInt32Array *ret = &execution.flint.newIntegerArray(retLen);
        shiftRightImpl((uint32_t *)ret->getData(), retLen, (uint32_t *)x->getData(), magLen, shift);
        return ret;
    }
}

static FlintInt32Array *multiplyKaratsuba(FlintExecution &execution, FlintInt32Array *x, FlintInt32Array *y) {
    uint32_t half = (MAX(x->getLength(), y->getLength()) + 1) / 2;

    FlintInt32Array *xl = getLower(execution, x, half);
    FlintInt32Array *xh = getUpper(execution, x, half);
    FlintInt32Array *yl = getLower(execution, y, half);
    FlintInt32Array *yh = getUpper(execution, y, half);

    FlintInt32Array *p1 = multiply(execution, xh, yh);
    FlintInt32Array *p2 = multiply(execution, xl, yl);

    FlintInt32Array *tmp1 = add(execution, xh, xl);
    if(xl && xl != x) execution.flint.freeObject(*xl);
    if(xh && xh != x) execution.flint.freeObject(*xh);

    FlintInt32Array *tmp2 = add(execution, yh, yl);
    if(yl && yl != y) execution.flint.freeObject(*yl);
    if(yh && yh != y) execution.flint.freeObject(*yh);

    FlintInt32Array *p3 = multiply(execution, tmp1, tmp2);
    if(tmp1) execution.flint.freeObject(*tmp1);
    if(tmp2) execution.flint.freeObject(*tmp2);

    tmp1 = subtract(execution, p3, p1);
    if(p3) execution.flint.freeObject(*p3);

    tmp2 = subtract(execution, tmp1, p2);                   /* p3 - p1 - p2 */
    if(tmp1) execution.flint.freeObject(*tmp1);

    tmp1 = shiftLeft(execution, p1, 32 * half);             /* p1 << (32 * half) */
    if(p1) execution.flint.freeObject(*p1);

    FlintInt32Array *result = add(execution, tmp1, tmp2);   /* p1 << (32 * half) + (p3 - p1 - p2) */
    if(tmp1) execution.flint.freeObject(*tmp1);
    if(tmp2) execution.flint.freeObject(*tmp2);

    tmp1 = shiftLeft(execution, result, 32 * half);         /* (p1 << (32 * half) + (p3 - p1 - p2)) << (32 * half) */
    if(result) execution.flint.freeObject(*result);

    result = add(execution, tmp1, p2);                      /* (p1 << (32 * half) + (p3 - p1 - p2)) << (32 * half) + p2 */
    if(p2) execution.flint.freeObject(*p2);
    if(tmp1) execution.flint.freeObject(*tmp1);

    return result;
}

static FlintInt32Array *multiply(FlintExecution &execution, FlintInt32Array *x, FlintInt32Array *y) {
    uint32_t xLen = x ? x->getLength() : 0;
    uint32_t yLen = y ? y->getLength() : 0;
    if(xLen == 0 || yLen == 0)
        return NULL;
    if(x == y)
        return square(execution, x);
    else if((xLen < KARATSUBA_THRESHOLD) || (yLen < KARATSUBA_THRESHOLD)) {
        uint32_t xBitLen = bitLength((uint32_t *)x->getData(), xLen);
        uint32_t yBitLen = bitLength((uint32_t *)y->getData(), yLen);
        FlintInt32Array &ret = execution.flint.newIntegerArray((xBitLen + yBitLen + 31) / 32);
        multiplyBasicImpl(
            (uint32_t *)ret.getData(), ret.getLength(),
            (uint32_t *)x->getData(), xLen,
            (uint32_t *)y->getData(), yLen
        );
        return trustedStripLeadingZeroInts(execution, &ret);
    }
    else
        return multiplyKaratsuba(execution, x, y);
}

static FlintInt32Array *divideByInt(FlintExecution &execution, FlintInt32Array *x, uint32_t y, uint32_t *rem) {
    uint32_t xLen = x->getLength();
    uint32_t retLen = (xLen == 1) ? 1 : (((uint32_t)x->getData()[0] < y) ? (xLen - 1) : xLen);
    FlintInt32Array &ret = execution.flint.newIntegerArray(retLen);
    uint32_t tmp = divideByIntImpl((uint32_t *)ret.getData(), retLen, (uint32_t *)x->getData(), xLen, y);
    if(rem)
        *rem = tmp;
    return trustedStripLeadingZeroInts(execution, &ret);
}

static FlintInt32Array *divideKnuth(FlintExecution &execution, FlintInt32Array *x, FlintInt32Array *y, FlintInt32Array **rem) {
    uint32_t xLen = x->getLength();
    uint32_t yLen = y->getLength();
    int32_t retLen = (xLen - yLen) + (((uint32_t)x->getData()[0] >= (uint32_t)y->getData()[0]) ? 1 : 0);
    if(retLen == 0)
        return NULL;

    uint32_t aLen, shift;
    FlintInt32Array *a;
    FlintInt32Array *b = &execution.flint.newIntegerArray(yLen);
    uint32_t *aData;
    uint32_t *bData = (uint32_t *)b->getData();
    if((uint32_t)y->getData()[0] < 0x80000000) {
        uint32_t *xData = (uint32_t *)x->getData();
        uint32_t *yData = (uint32_t *)y->getData();
        shift = 32 - bitLength(y->getData()[0]);
        aLen = ((xData[0] >> (32 - shift)) != 0) ? (xLen + 1) : xLen;
        a = &execution.flint.newIntegerArray(aLen);
        aData = (uint32_t *)a->getData();
        shiftLeftImpl(aData, aLen, xData, xLen, shift);
        shiftLeftImpl(bData, yLen, yData, yLen, shift);
    }
    else {
        shift = 0;
        aLen = xLen;
        a = &execution.flint.newIntegerArray(xLen);
        aData = (uint32_t *)a->getData();
        memcpy(aData, x->getData(), xLen * sizeof(int32_t));
        memcpy(bData, y->getData(), yLen * sizeof(int32_t));
    }

    uint64_t tmp = aData[0];
    int32_t retIdx = (aData[0] >= bData[0]) ? 0 : -1;
    FlintInt32Array &ret = execution.flint.newIntegerArray(retLen);
    FlintInt32Array &bq = execution.flint.newIntegerArray(MAX(aLen, yLen + 1));
    uint32_t *retData = (uint32_t *)ret.getData();
    uint32_t *bqData = (uint32_t *)bq.getData();
    memset(&bqData[yLen + 1], 0, (bq.getLength() - (yLen + 1)) * sizeof(uint32_t));
    while(retIdx < retLen) {
        uint32_t q = tmp / bData[0];

        if(q) {
            uint32_t bqLen = (tmp >> 32) ? (yLen + 1) : yLen;
            bqData[yLen] = 0;
            while(q) {
                multiplyBasicImpl(bqData, bqLen, bData, yLen, &q, 1);
                if(compareMagnitudeImpl(bqData, aLen, aData, aLen) <= 0)
                    break;
                q--;
            }
            if(q)
                subtractImpl(aData, aLen, aData, aLen, bqData, aLen);
        }

        if(aData[0] == 0) {
            aLen--;
            aData = &aData[1];
        }

        if(retIdx >= 0)
            retData[retIdx] = q;
        retIdx++;

        tmp = (aLen < 2) ? aData[0] : (((uint64_t)aData[0] << 32) | aData[1]);
    }

    if(rem) {
        if(aLen == 0) {
            *rem = NULL;
            execution.flint.freeObject(*a);
        }
        else if((aLen == a->getLength()) && (aData[0] >> shift)) {
            if(shift != 0)
                shiftRightImpl(aData, aLen, aData, aLen, shift);
            *rem = a;
        }
        else {
            uint32_t remLen = aLen - ((aData[0] >> shift) ? 0 : 1);
            *rem = &execution.flint.newIntegerArray(remLen);
            shiftRightImpl((uint32_t *)(*rem)->getData(), remLen, aData, aLen, shift);
            execution.flint.freeObject(*a);
        }
    }
    else
        execution.flint.freeObject(*a);

    execution.flint.freeObject(*b);
    execution.flint.freeObject(bq);

    return &ret;
}

static FlintInt32Array *divide(FlintExecution &execution, FlintInt32Array *x, FlintInt32Array *y) {
    if(y == NULL)
        throw &execution.flint.newArithmeticException(execution.flint.newString(STR_AND_SIZE("Divided by zero")));
    else if(x == NULL)
        return NULL;
    if(compareMagnitude(x, y) < 0)
        return NULL;
    if(y->getLength() == 1)
        return divideByInt(execution, x, y->getData()[0], NULL);
    else
        return divideKnuth(execution, x, y, NULL);
}

static FlintInt32Array *remainder(FlintExecution &execution, FlintInt32Array *x, FlintInt32Array *y) {
    if(y == NULL)
        throw &execution.flint.newArithmeticException(execution.flint.newString(STR_AND_SIZE("Divided by zero")));
    else if(x == NULL)
        return NULL;
    if(compareMagnitude(x, y) < 0)
        return x;
    if(y->getLength() == 1) {
        if(y->getData()[0] == 1)
            return NULL;
        uint32_t rem;
        divideByInt(execution, x, y->getData()[0], &rem);
        if(rem == 0)
            return NULL;
        FlintInt32Array &ret = execution.flint.newIntegerArray(1);
        ret.getData()[0] = (int32_t)rem;
        return &ret;
    }
    else {
        FlintInt32Array *rem;
        FlintInt32Array *ret = divideKnuth(execution, x, y, &rem);
        if(ret)
            execution.flint.freeObject(*ret);
        return rem;
    }
}

static FlintInt32Array *squareKaratsuba(FlintExecution &execution, FlintInt32Array *x) {
    uint32_t half = (x->getLength() + 1) / 2;

    FlintInt32Array *xl = getLower(execution, x, half);
    FlintInt32Array *xh = getUpper(execution, x, half);

    FlintInt32Array *xhs = square(execution, xh);
    FlintInt32Array *xls = square(execution, xl);

    FlintInt32Array *tmp1 = add(execution, xh, xl);             
    if(xl && xl != x) execution.flint.freeObject(*xl);
    if(xh && xh != x) execution.flint.freeObject(*xh);
    
    FlintInt32Array *tmp2 = square(execution, tmp1);            
    if(tmp1) execution.flint.freeObject(*tmp1);

    tmp1 = add(execution, xhs, xls);                            

    FlintInt32Array *result = subtract(execution, tmp2, tmp1);  
    if(tmp1) execution.flint.freeObject(*tmp1);
    if(tmp2) execution.flint.freeObject(*tmp2);

    tmp1 = shiftLeft(execution, xhs, 32 * half);                
    if(xhs) execution.flint.freeObject(*xhs);

    tmp2 = add(execution, tmp1, result);                        
    if(tmp1) execution.flint.freeObject(*tmp1);
    if(result) execution.flint.freeObject(*result);

    tmp1 = shiftLeft(execution, tmp2, 32 * half);               
    if(tmp2) execution.flint.freeObject(*tmp2);

    result = add(execution, tmp1, xls);                         
    if(tmp1) execution.flint.freeObject(*tmp1);
    if(xls) execution.flint.freeObject(*xls);

    return result;
}

static FlintInt32Array *square(FlintExecution &execution, FlintInt32Array *x) {
    if(x == NULL)
        return NULL;
    uint32_t xLen = x->getLength();
    if(xLen < KARATSUBA_THRESHOLD) {
        uint32_t xBitLen = bitLength((uint32_t *)x->getData(), xLen);
        FlintInt32Array &ret = execution.flint.newIntegerArray((xBitLen * 2 + 31) / 32);
        multiplyBasicImpl(
            (uint32_t *)ret.getData(), ret.getLength(),
            (uint32_t *)x->getData(), xLen,
            (uint32_t *)x->getData(), xLen
        );
        return trustedStripLeadingZeroInts(execution, &ret);
    }
    else
        return squareKaratsuba(execution, x);
}

static FlintInt32Array *pow1(FlintExecution &execution, FlintInt32Array *x, uint32_t exponent) {
    uint32_t xLen = x->getLength();
    uint32_t baseLen = xLen;
    uint32_t curRetLen = 1;
    uint32_t retLen = (bitLength((uint32_t *)x->getData(), xLen) * exponent + 31) / 32;
    FlintInt32Array *ret = &execution.flint.newIntegerArray(retLen);
    FlintInt32Array *base = &execution.flint.newIntegerArray(retLen);
    FlintInt32Array *buff = &execution.flint.newIntegerArray(retLen);

    setValueByIntImpl((uint32_t *)ret->getData(), curRetLen, 1);
    setValueImpl((uint32_t *)base->getData(), baseLen, (uint32_t *)x->getData(), xLen);

    while(exponent > 0) {
        FlintInt32Array *tmp;
        if(exponent % 2) {
            uint32_t newRetLen = MIN(curRetLen + baseLen, retLen);
            multiplyBasicImpl(
                (uint32_t *)buff->getData(), newRetLen,
                (uint32_t *)ret->getData(), curRetLen,
                (uint32_t *)base->getData(), baseLen
            );

            if(execution.hasTerminateRequest()) {
                execution.flint.freeObject(*ret);
                execution.flint.freeObject(*base);
                execution.flint.freeObject(*buff);
                throw &execution.flint.newInterruptedException(*(FlintString *)NULL);
            }

            tmp = ret;
            ret = buff;
            buff = tmp;
            curRetLen = newRetLen;
        }
        exponent /= 2;
        if(exponent) {
            uint32_t newBaseLen = MIN(baseLen * 2, retLen);
            multiplyBasicImpl(
                (uint32_t *)buff->getData(), newBaseLen,
                (uint32_t *)base->getData(), baseLen,
                (uint32_t *)base->getData(), baseLen
            );
            tmp = base;
            base = buff;
            buff = tmp;
            baseLen = newBaseLen;
        }
    }

    execution.flint.freeObject(*base);
    execution.flint.freeObject(*buff);

    return ret;
}

static FlintInt32Array *pow2(FlintExecution &execution, FlintInt32Array *x, uint32_t exponent) {
    FlintInt32Array *ret = 0;
    FlintInt32Array *base = x;
    while(exponent > 0) {
        if(exponent % 2) {
            if(ret == 0)
                ret = base;
            else {
                FlintInt32Array *tmp = multiply(execution, ret, base);
                if(ret != x)
                    execution.flint.freeObject(*ret);
                ret = tmp;
            }
            if(execution.hasTerminateRequest()) {
                if(ret != x) execution.flint.freeObject(*ret);
                if(base != x) execution.flint.freeObject(*base);
                throw &execution.flint.newInterruptedException(*(FlintString *)NULL);
            }
        }
        exponent /= 2;
        if(exponent) {
            FlintInt32Array *tmp = square(execution, base);
            if(base != x)
                execution.flint.freeObject(*base);
            base = tmp;
        }
    }
    if(base != x && base != ret)
        execution.flint.freeObject(*base);
    return ret;
}

static FlintInt32Array *pow(FlintExecution &execution, FlintInt32Array *x, uint32_t exponent) {
    uint32_t bitLen = x ? bitLength((uint32_t *)x->getData(), x->getLength()) : 0;
    uint32_t retLen = bitLen * exponent;
    if((retLen <= bitLen) || (retLen <= exponent) || ((uint32_t)retLen > (uint32_t)0xFFFFFFE0))
        throw "BigInteger would overflow supported range";
    if(retLen < (KARATSUBA_THRESHOLD * 2))
        return pow1(execution, x, exponent);
    else
        return pow2(execution, x, exponent);
}

static FlintInt32Array *sqrt(FlintExecution &execution, FlintInt32Array *x) {
    if(x == NULL)
        return NULL;
    else if(x->getLength() == 1) {
        FlintInt32Array *ret = &execution.flint.newIntegerArray(1);
        ((uint32_t *)ret->getData())[0] = (uint32_t)sqrt((uint32_t)x->getData()[0]);
        return ret;
    }
    else if(x->getLength() == 2) {
        uint64_t val = ((uint64_t)x->getData()[0] << 32) | ((uint32_t)x->getData()[1]);
        val = (uint64_t)sqrt(val);
        if(val > 0xFFFFFFFF) {
            FlintInt32Array *ret = &execution.flint.newIntegerArray(2);
            ((uint32_t *)ret->getData())[0] = (uint32_t)(val >> 32);
            ((uint32_t *)ret->getData())[1] = (uint32_t)val;
            return ret;
        }
        else {
            FlintInt32Array *ret = &execution.flint.newIntegerArray(1);
            ((uint32_t *)ret->getData())[1] = (uint32_t)val;
            return ret;
        }
    }
    else {
        FlintInt32Array *ret2 = 0;
        FlintInt32Array *ret1 = shiftRight(execution, x, 1);
        do {
            if(execution.hasTerminateRequest()) {
                execution.flint.freeObject(*ret1);
                if(ret2) execution.flint.freeObject(*ret2);
                throw &execution.flint.newInterruptedException(*(FlintString *)NULL);
            }
            else {
                FlintInt32Array *tmp1 = divide(execution, x, ret1);

                FlintInt32Array *tmp2 = add(execution, ret1, tmp1);
                execution.flint.freeObject(*tmp1);
                
                ret2 = shiftRight(execution, tmp2, 1);
                execution.flint.freeObject(*tmp2);

                if(compareMagnitude(ret1, ret2) == 0) {
                    execution.flint.freeObject(*ret1);
                    return ret2;
                }

                execution.flint.freeObject(*ret1);
                ret1 = ret2;
            }
        } while(true);
    }
}

static void nativeMakeMagnitudeWithLongInput(FlintExecution &execution) {
    int64_t val = execution.stackPopInt64();
    if(val < 0)
        val = -val;
    int32_t highWord = val >> 32;
    if(highWord) {
        FlintInt32Array &mag = execution.flint.newIntegerArray(2);
        mag.getData()[0] = highWord;
        mag.getData()[1] = (int32_t)val;
        execution.stackPushObject(&mag);
    }
    else {
        FlintInt32Array &mag = execution.flint.newIntegerArray(1);
        mag.getData()[0] = (int32_t)val;
        execution.stackPushObject(&mag);
    }
}

static void nativeMakeMagnitudeWithByteArrayInput(FlintExecution &execution) {
    int32_t len = execution.stackPopInt32();
    int32_t off = execution.stackPopInt32();
    FlintInt8Array *val = (FlintInt8Array *)execution.stackPopObject();
    if(val == NULL)
        return execution.stackPushObject(NULL);
    checkMakeMagnitudeParams(execution, val->getLength(), off, len);
    uint32_t end = off + len;
    if(val->getData()[off] >= 0)
        return execution.stackPushObject(makeMagnitude(execution, val, off, end));
    execution.stackPushObject(makePositiveMagnitude(execution, val, off, end));
}

static void nativeMakeMagnitudeWithSignumInput(FlintExecution &execution) {
    int32_t len = execution.stackPopInt32();
    int32_t off = execution.stackPopInt32();
    FlintInt8Array *val = (FlintInt8Array *)execution.stackPopObject();
    int32_t signum = execution.stackPopInt32();
    if(signum == 0 || val == NULL)
        return execution.stackPushObject(NULL);
    checkMakeMagnitudeParams(execution, val->getLength(), off, len);
    uint32_t end = off + len;
    if(signum > 0)
        return execution.stackPushObject(makeMagnitude(execution, val, off, end));
    execution.stackPushObject(makePositiveMagnitude(execution, val, off, end));
}

static void nativeMakeMagnitudeWithIntArrayInput(FlintExecution &execution) {
    int32_t len = execution.stackPopInt32();
    int32_t off = execution.stackPopInt32();
    FlintInt32Array *val = (FlintInt32Array *)execution.stackPopObject();
    if(val == NULL)
        return execution.stackPushObject(NULL);
    checkMakeMagnitudeParams(execution, val->getLength(), off, len);
    uint32_t end = off + len;
    if(val->getData()[off] >= 0)
        return execution.stackPushObject(makeMagnitude(execution, val, off, end));
    execution.stackPushObject(makePositiveMagnitude(execution, val, off, end));
}

static void nativeBitLength(FlintExecution &execution) {
    int32_t signum = execution.stackPopInt32();
    FlintInt32Array *mag = (FlintInt32Array *)execution.stackPopObject();
    if(signum == 0 || mag == NULL)
        return execution.stackPushInt32(0);
    uint32_t *magData = (uint32_t *)mag->getData();
    uint32_t magLen = mag->getLength();
    int32_t n = bitLength(magData, magLen);
    if(signum > 0)
        return execution.stackPushInt32(n);
    bool isPowOfTwo = isPowerOfTwo(magData[0]);
    for(uint32_t i = 1; i < magLen && isPowOfTwo; i++)
        isPowOfTwo = (magData[i] == 0);
    execution.stackPushInt32(isPowOfTwo ? (n - 1) : n);
}

static void nativeCompareMagnitude(FlintExecution &execution) {
    FlintInt32Array *y = (FlintInt32Array *)execution.stackPopObject();
    FlintInt32Array *x = (FlintInt32Array *)execution.stackPopObject();
    execution.stackPushInt32(compareMagnitude(x, y));
}

static void nativeAdd(FlintExecution &execution) {
    FlintInt32Array *y = (FlintInt32Array *)execution.stackPopObject();
    FlintInt32Array *x = (FlintInt32Array *)execution.stackPopObject();
    execution.stackPushObject(add(execution, x, y));
}

static void nativeSubtract(FlintExecution &execution) {
    FlintInt32Array *little = (FlintInt32Array *)execution.stackPopObject();
    FlintInt32Array *big = (FlintInt32Array *)execution.stackPopObject();
    execution.stackPushObject(subtract(execution, big, little));
}

static void nativeMultiply(FlintExecution &execution) {
    FlintInt32Array *y = (FlintInt32Array *)execution.stackPopObject();
    FlintInt32Array *x = (FlintInt32Array *)execution.stackPopObject();
    execution.stackPushObject(multiply(execution, x, y));
}

static void nativeDivide(FlintExecution &execution) {
    FlintInt32Array *y = (FlintInt32Array *)execution.stackPopObject();
    FlintInt32Array *x = (FlintInt32Array *)execution.stackPopObject();
    execution.stackPushObject(divide(execution, x, y));
}

static void nativeRemainder(FlintExecution &execution) {
    FlintInt32Array *y = (FlintInt32Array *)execution.stackPopObject();
    FlintInt32Array *x = (FlintInt32Array *)execution.stackPopObject();
    execution.stackPushObject(remainder(execution, x, y));
}

static void nativeShiftLeft(FlintExecution &execution) {
    uint32_t n = execution.stackPopInt32();
    FlintInt32Array *x = (FlintInt32Array *)execution.stackPopObject();
    execution.stackPushObject(shiftLeft(execution, x, n));
}

static void nativeShiftRight(FlintExecution &execution) {
    uint32_t n = execution.stackPopInt32();
    FlintInt32Array *x = (FlintInt32Array *)execution.stackPopObject();
    execution.stackPushObject(shiftRight(execution, x, n));
}

static void nativeSquare(FlintExecution &execution) {
    FlintInt32Array *x = (FlintInt32Array *)execution.stackPopObject();
    execution.stackPushObject(square(execution, x));
}

static void nativePow(FlintExecution &execution) {
    uint32_t exponent = execution.stackPopInt32();
    FlintInt32Array *x = (FlintInt32Array *)execution.stackPopObject();
    execution.stackPushObject(pow(execution, x, exponent));
}

static void nativeSqrt(FlintExecution &execution) {
    FlintInt32Array *x = (FlintInt32Array *)execution.stackPopObject();
    execution.stackPushObject(sqrt(execution, x));
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x0D\x00\xD7\x06""makeMagnitude",    "\x05\x00\x86\xEF""(J)[I",     nativeMakeMagnitudeWithLongInput),
    NATIVE_METHOD("\x0D\x00\xD7\x06""makeMagnitude",    "\x08\x00\xB9\x31""([BII)[I",  nativeMakeMagnitudeWithByteArrayInput),
    NATIVE_METHOD("\x0D\x00\xD7\x06""makeMagnitude",    "\x09\x00\xCA\x2F""(I[BII)[I", nativeMakeMagnitudeWithSignumInput),
    NATIVE_METHOD("\x0D\x00\xD7\x06""makeMagnitude",    "\x08\x00\xB8\x4A""([III)[I",  nativeMakeMagnitudeWithIntArrayInput),
    NATIVE_METHOD("\x09\x00\x7F\xF5""bitLength",        "\x06\x00\x85\xED""([II)I",    nativeBitLength),
    NATIVE_METHOD("\x10\x00\x16\xC4""compareMagnitude", "\x07\x00\x80\x8C""([I[I)I",   nativeCompareMagnitude),
    NATIVE_METHOD("\x03\x00\xF4\xCA""add",              "\x08\x00\x00\x49""([I[I)[I",  nativeAdd),
    NATIVE_METHOD("\x08\x00\x06\x3A""subtract",         "\x08\x00\x00\x49""([I[I)[I",  nativeSubtract),
    NATIVE_METHOD("\x08\x00\x15\x40""multiply",         "\x08\x00\x00\x49""([I[I)[I",  nativeMultiply),
    NATIVE_METHOD("\x06\x00\x8B\x76""divide",           "\x08\x00\x00\x49""([I[I)[I",  nativeDivide),
    NATIVE_METHOD("\x09\x00\x51\x46""remainder",        "\x08\x00\x00\x49""([I[I)[I",  nativeRemainder),
    NATIVE_METHOD("\x09\x00\xEE\x70""shiftLeft",        "\x07\x00\xA1\x4A""([II)[I",   nativeShiftLeft),
    NATIVE_METHOD("\x0A\x00\x42\x86""shiftRight",       "\x07\x00\xA1\x4A""([II)[I",   nativeShiftRight),
    NATIVE_METHOD("\x06\x00\x27\xB5""square",           "\x07\x00\xA1\x4A""([II)[I",   nativeSquare),
    NATIVE_METHOD("\x03\x00\xE2\x32""pow",              "\x07\x00\xA1\x4A""([II)[I",   nativePow),
    NATIVE_METHOD("\x04\x00\x91\xC3""sqrt",             "\x06\x00\xA1\x53""([I)[I",    nativeSqrt),
};

const FlintNativeClass BIGINTEGER_CLASS = NATIVE_CLASS(bigIntegerClassName, methods);
