
#error "Do not build this file. You need to create a new file based on this template file and implement the functions defined in this file."

#include "flint_system_api.h"

void *FlintAPI::Directory::open(const char *dirName) {
    throw "FlintAPI::Directory::open is not implemented in VM";
}

FlintFileResult FlintAPI::Directory::read(void *handle, uint8_t *attribute, char *nameBuff, uint32_t buffSize, uint32_t *size, int64_t *time) {
    throw "FlintAPI::Directory::read is not implemented in VM";
}

FlintFileResult FlintAPI::Directory::close(void *handle) {
    throw "FlintAPI::Directory::close is not implemented in VM";
}

FlintFileResult FlintAPI::Directory::exists(const char *path) {
    throw "FlintAPI::Directory::exists is not implemented in VM";
}

FlintFileResult FlintAPI::Directory::create(const char *path) {
    throw "FlintAPI::Directory::create is not implemented in VM";
}

FlintFileResult FlintAPI::Directory::remove(const char *path) {
    throw "FlintAPI::Directory::remove is not implemented in VM";
}
