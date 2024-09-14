
#error "Do not build this file. You need to create a new file based on this template file and implement the functions defined in this file."

#include "flint_system_api.h"

FlintFileResult FlintAPI::IO::finfo(const char *fileName, uint32_t *size, int64_t *time) {
    throw "FlintAPI::IO::finfo is not implemented in VM";
}

void *FlintAPI::IO::fopen(const char *fileName, FlintFileMode mode) {
    throw "FlintAPI::IO::fopen is not implemented in VM";
}

FlintFileResult FlintAPI::IO::fread(void *handle, void *buff, uint32_t btr, uint32_t *br) {
    throw "FlintAPI::IO::fread is not implemented in VM";
}

FlintFileResult FlintAPI::IO::fwrite(void *handle, void *buff, uint32_t btw, uint32_t *bw) {
    throw "FlintAPI::IO::fwrite is not implemented in VM";
}

uint32_t FlintAPI::IO::fsize(void *handle) {
    throw "FlintAPI::IO::fsize is not implemented in VM";
}

uint32_t FlintAPI::IO::ftell(void *handle) {
    throw "FlintAPI::IO::ftell is not implemented in VM";
}

FlintFileResult FlintAPI::IO::fseek(void *handle, uint32_t offset) {
    throw "FlintAPI::IO::fseek is not implemented in VM";
}

FlintFileResult FlintAPI::IO::fclose(void *handle) {
    throw "FlintAPI::IO::fclose is not implemented in VM";
}

FlintFileResult FlintAPI::IO::fremove(const char *fileName) {
    throw "FlintAPI::IO::fremove is not implemented in VM";
}

void *FlintAPI::IO::opendir(const char *dirName) {
    throw "FlintAPI::Directory::open is not implemented in VM";
}

FlintFileResult FlintAPI::IO::readdir(void *handle, uint8_t *attribute, char *nameBuff, uint32_t buffSize, uint32_t *size, int64_t *time) {
    throw "FlintAPI::Directory::read is not implemented in VM";
}

FlintFileResult FlintAPI::IO::closedir(void *handle) {
    throw "FlintAPI::Directory::close is not implemented in VM";
}

FlintFileResult FlintAPI::IO::mkdir(const char *path) {
    throw "FlintAPI::Directory::create is not implemented in VM";
}
