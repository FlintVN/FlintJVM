
#include "mjvm_file_not_found_exception.h"

const char *FileNotFound::getFileName(void) const {
    return (const char *)this;
}
