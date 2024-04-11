
#ifndef __MJVM_CLASS_FILE_H
#define __MJVM_CLASS_FILE_H

#include <stdint.h>
#include <iostream>

class ClassFile {
private:
    FILE *fptr = 0;
public:
    ClassFile(const char *fileName);
    ClassFile(const char *fileName, uint16_t length);
    void read(void *buff, uint32_t size);
    uint8_t readUInt8(void);
    uint16_t readUInt16(void);
    uint32_t readUInt32(void);
    uint64_t readUInt64(void);
    void lseek(uint32_t offset);
    void close(void);
    ~ClassFile();
};

#endif /* __MJVM_CLASS_FILE_H */
