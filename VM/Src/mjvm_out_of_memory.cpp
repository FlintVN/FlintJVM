
#include "mjvm_out_of_memory.h"

const char *MjvmOutOfMemoryError::getMessage(void) const {
    return (const char *)this;
}
