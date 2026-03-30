
#include <string.h>
#include "flint.h"
#include "flint_native_interface.h"

extern uint8_t parseArgc(const char *desc);

jlong::operator int64_t() const {
    uint64_t ret;
    ((uint32_t *)&ret)[0] = low;
    ((uint32_t *)&ret)[1] = high;
    return ret;
}

void jlong::operator=(int64_t value) {
    low = ((uint32_t *)&value)[0];
    high = ((uint32_t *)&value)[1];
}

jdouble::operator double() const {
    double ret;
    ((uint32_t *)&ret)[0] = low;
    ((uint32_t *)&ret)[1] = high;
    return ret;
}

void jdouble::operator=(double value) {
    low = ((uint32_t *)&value)[0];
    high = ((uint32_t *)&value)[1];
}

FNIEnv::FNIEnv(void) {

}
