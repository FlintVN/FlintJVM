
#error "Do not build this file. You need to create a new file based on this template file and implement the functions defined in this file."

#include "flint_system_api.h"

FlintFileResult FlintAPI::IO::finfo(const char *fileName, uint32_t *size, int64_t *time) {
    #error "FlintAPI::IO::finfo is not implemented in VM";
}

void *FlintAPI::IO::fopen(const char *fileName, FlintFileMode mode) {
    #error "FlintAPI::IO::fopen is not implemented in VM";
}

FlintFileResult FlintAPI::IO::fread(void *handle, void *buff, uint32_t btr, uint32_t *br) {
    #error "FlintAPI::IO::fread is not implemented in VM";
}

FlintFileResult FlintAPI::IO::fwrite(void *handle, void *buff, uint32_t btw, uint32_t *bw) {
    #error "FlintAPI::IO::fwrite is not implemented in VM";
}

uint32_t FlintAPI::IO::fsize(void *handle) {
    #error "FlintAPI::IO::fsize is not implemented in VM";
}

uint32_t FlintAPI::IO::ftell(void *handle) {
    #error "FlintAPI::IO::ftell is not implemented in VM";
}

FlintFileResult FlintAPI::IO::fseek(void *handle, uint32_t offset) {
    #error "FlintAPI::IO::fseek is not implemented in VM";
}

FlintFileResult FlintAPI::IO::fclose(void *handle) {
    #error "FlintAPI::IO::fclose is not implemented in VM";
}

FlintFileResult FlintAPI::IO::fremove(const char *fileName) {
    #error "FlintAPI::IO::fremove is not implemented in VM";
}

void *FlintAPI::IO::opendir(const char *dirName) {
    #error "FlintAPI::Directory::open is not implemented in VM";
}

FlintFileResult FlintAPI::IO::readdir(void *handle, uint8_t *attribute, char *nameBuff, uint32_t buffSize, uint32_t *size, int64_t *time) {
    #error "FlintAPI::Directory::read is not implemented in VM";
}

FlintFileResult FlintAPI::IO::closedir(void *handle) {
    #error "FlintAPI::Directory::close is not implemented in VM";
}

FlintFileResult FlintAPI::IO::mkdir(const char *path) {
    #error "FlintAPI::Directory::create is not implemented in VM";
}
