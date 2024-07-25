
#include "flint_out_of_memory.h"

const char *FlintOutOfMemoryError::getMessage(void) const {
    return (const char *)this;
}
