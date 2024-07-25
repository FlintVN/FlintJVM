
#error "Do not build this file. You need to create a new file based on this template file and implement the functions defined in this file."

#include "flint_system_api.h"

void *FlintSystem_FileOpen(const char *fileName, FlintSys_FileMode mode) {
    throw "FlintSystem_FileOpen is not implemented in VM";
}

FlintSys_FileResult FlintSystem_FileRead(void *fileHandle, void *buff, uint32_t btr, uint32_t *br) {
    throw "FlintSystem_FileRead is not implemented in VM";
}

FlintSys_FileResult FlintSystem_FileWrite(void *fileHandle, void *buff, uint32_t btw, uint32_t *bw) {
    throw "FlintSystem_FileWrite is not implemented in VM";
}

uint32_t FlintSystem_FileSize(void *fileHandle) {
    throw "FlintSystem_FileSize is not implemented in VM";
}

uint32_t FlintSystem_FileTell(void *fileHandle) {
    throw "FlintSystem_FileTell is not implemented in VM";
}

FlintSys_FileResult FlintSystem_FileSeek(void *fileHandle, uint32_t offset) {
    throw "FlintSystem_FileSeek is not implemented in VM";
}

FlintSys_FileResult FlintSystem_FileClose(void *fileHandle) {
    throw "FlintSystem_FileClose is not implemented in VM";
}
