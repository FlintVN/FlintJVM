
#error "Do not build this file. You need to create a new file based on this template file and implement the functions defined in this file."

#include "flint_system_api.h"

FlintFileResult FlintAPI::File::exists(const char *fileName) {
    throw "FlintAPI::File::exists is not implemented in VM";
}

void *FlintAPI::File::open(const char *fileName, FlintFileMode mode) {
    throw "FlintAPI::File::open is not implemented in VM";
}

FlintFileResult FlintAPI::File::read(void *fileHandle, void *buff, uint32_t btr, uint32_t *br) {
    throw "FlintAPI::File::read is not implemented in VM";
}

FlintFileResult FlintAPI::File::write(void *fileHandle, void *buff, uint32_t btw, uint32_t *bw) {
    throw "FlintAPI::File::write is not implemented in VM";
}

uint32_t FlintAPI::File::size(void *fileHandle) {
    throw "FlintAPI::File::size is not implemented in VM";
}

uint32_t FlintAPI::File::tell(void *fileHandle) {
    throw "FlintAPI::File::tell is not implemented in VM";
}

FlintFileResult FlintAPI::File::seek(void *fileHandle, uint32_t offset) {
    throw "FlintAPI::File::seek is not implemented in VM";
}

FlintFileResult FlintAPI::File::close(void *fileHandle) {
    throw "FlintAPI::File::close is not implemented in VM";
}

FlintFileResult FlintAPI::File::remove(const char *fileName) {
    throw "FlintAPI::File::remove is not implemented in VM";
}
