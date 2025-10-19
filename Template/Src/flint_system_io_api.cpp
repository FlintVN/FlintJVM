
#error "Do not build this file. You need to create a new file based on this template file and implement the functions defined in this file."

#include "flint_system_api.h"

using namespace FlintAPI::IO;

FileResult FlintAPI::IO::finfo(const char *fileName, FileInfo *fileInfo) {
    #error "FlintAPI::IO::finfo is not implemented in VM";
}

FileHandle FlintAPI::IO::fopen(const char *fileName, FileMode mode) {
    #error "FlintAPI::IO::fopen is not implemented in VM";
}

FileResult FlintAPI::IO::fread(FileHandle handle, void *buff, uint32_t btr, uint32_t *br) {
    #error "FlintAPI::IO::fread is not implemented in VM";
}

FileResult FlintAPI::IO::fwrite(FileHandle handle, void *buff, uint32_t btw, uint32_t *bw) {
    #error "FlintAPI::IO::fwrite is not implemented in VM";
}

uint32_t FlintAPI::IO::ftell(FileHandle handle) {
    #error "FlintAPI::IO::ftell is not implemented in VM";
}

FileResult FlintAPI::IO::fseek(FileHandle handle, uint32_t offset) {
    #error "FlintAPI::IO::fseek is not implemented in VM";
}

FileResult FlintAPI::IO::fclose(FileHandle handle) {
    #error "FlintAPI::IO::fclose is not implemented in VM";
}

FileResult FlintAPI::IO::fremove(const char *fileName) {
    #error "FlintAPI::IO::fremove is not implemented in VM";
}

DirHandle FlintAPI::IO::opendir(const char *dirName) {
    #error "FlintAPI::Directory::open is not implemented in VM";
}

FileResult FlintAPI::IO::readdir(DirHandle handle, FileInfo *fileInfo) {
    #error "FlintAPI::Directory::read is not implemented in VM";
}

FileResult FlintAPI::IO::closedir(DirHandle handle) {
    #error "FlintAPI::Directory::close is not implemented in VM";
}

FileResult FlintAPI::IO::mkdir(const char *path) {
    #error "FlintAPI::Directory::create is not implemented in VM";
}
