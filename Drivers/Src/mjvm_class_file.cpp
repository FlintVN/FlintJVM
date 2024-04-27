
#include <string.h>
#include "mjvm_common.h"
#include "mjvm_class_file.h"
#include "mjvm_file_not_found_exception.h"

ClassFile::ClassFile(const char *fileName) {
    char buff[256];
    strcpy(buff, fileName);
    strcat(buff, ".class");
    fptr = fopen(buff, "rb");
    if(fptr == 0)
        throw (FileNotFound *)fileName;
}

ClassFile::ClassFile(const char *fileName, uint16_t length) {
    char buff[256];
    strncpy(buff, fileName, length);
    strcpy(&buff[length], ".class");
    fptr = fopen(buff, "rb");
    if(fptr == 0)
        throw (FileNotFound *)fileName;
}

void ClassFile::read(void *buff, uint32_t size) {
    uint8_t *u8Buff = (uint8_t *)buff;
    while(size) {
        uint32_t temp = fread(buff, 1, size, fptr);
        if(temp > 0) {
            size -= temp;
            u8Buff += temp;
        }
        else
            throw "read file error";
    }
}

uint8_t ClassFile::readUInt8(void) {
    uint8_t temp;
    read(&temp, sizeof(temp));
    return temp;
}

uint16_t ClassFile::readUInt16(void) {
    uint16_t temp;
    read(&temp, sizeof(temp));
    return swap16(temp);
}

uint32_t ClassFile::readUInt32(void) {
    uint32_t temp;
    read(&temp, sizeof(temp));
    return swap32(temp);
}

uint64_t ClassFile::readUInt64(void) {
    uint64_t temp;
    read(&temp, sizeof(temp));
    return swap64(temp);
}

void ClassFile::lseek(uint32_t offset) {
    fseek(fptr, offset, SEEK_SET);
}

void ClassFile::close(void) {
    fclose(fptr);
}

ClassFile::~ClassFile() {
    if(fptr != 0)
        fclose(fptr);
}
