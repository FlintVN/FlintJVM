
#include "flint_load_file_error.h"

const char *FlintLoadFileError::getFileName(void) const {
    return (const char *)this;
}
