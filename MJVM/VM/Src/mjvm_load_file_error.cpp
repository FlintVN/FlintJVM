
#include "mjvm_load_file_error.h"

const char *MjvmLoadFileError::getFileName(void) const {
    return (const char *)this;
}
