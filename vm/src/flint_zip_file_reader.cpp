
#include <string.h>
#include "flint.h"
#include "flint_zip_file_reader.h"

ZipFileReader::ZipFileReader(void) : FileReader(), compressionMethod(0), compressedSize(0), uncompressedSize(0) {

}

ZipFileReader::ZipFileReader(FExec *ctx, const char *filePath) : FileReader(ctx, filePath), compressionMethod(0), compressedSize(0), uncompressedSize(0) {

}

bool ZipFileReader::gotoEOCD(void) {
    return seek(size() - 22);
}

int32_t ZipFileReader::gotoCDFH(void) {
    if(!gotoEOCD()) return -1;
    uint16_t fileCount;
    uint32_t magicNum, CDFHPos;
    uint32_t pos = tell();

    if(!readUInt32(magicNum)) return -1;
    if(!seek(pos + 10)) return -1;
    if(!readUInt16(fileCount)) return -1;
    if(!seek(pos + 16)) return -1;
    if(!readUInt32(CDFHPos)) return -1;
    if(!seek(CDFHPos)) return -1;

    return fileCount;
}

bool ZipFileReader::gotoFile(const char *name, uint16_t length) {
    if(length == 0xFFFF) length = strlen(name);

    int32_t fileCount = gotoCDFH();
    if(fileCount < 0) return false;

    for(uint32_t i = 0; i < fileCount; i++) {
        uint32_t off = tell();

        if(!seek(off + 28)) return false;

        uint16_t nameLen, fieldLen, cmtLen;
        if(!readUInt16(nameLen)) return false;
        if(!readUInt16(fieldLen)) return false;
        if(!readUInt16(cmtLen)) return false;

        uint32_t fileOff;
        if(!seek(off + 42)) return false;
        if(!readUInt32(fileOff)) return false;

        if(nameLen == length) {
            bool isOk = true;
            char buff[16];
            uint32_t N = length / sizeof(buff);
            for(uint32_t i = 0; isOk && (i < N); i++) {
                if(read(buff, sizeof(buff)) != sizeof(buff)) return false;
                if(strncmp(&name[i * sizeof(buff)], buff, sizeof(buff)) != 0)
                    isOk = false;
            }
            N = length % sizeof(buff);
            if(isOk && N > 0) {
                if(read(buff, N) != N) return false;
                isOk = strncmp(&name[length - N], buff, N) == 0;
            }
            if(isOk) {
                uint16_t method;
                uint32_t packedSize, unpackedSize;
                if(!seek(off + 10)) return false;
                if(!readUInt16(method)) return false;
                if(!seek(off + 20)) return false;
                if(!readUInt32(packedSize)) return false;
                if(!readUInt32(unpackedSize)) return false;
                if(!seek(fileOff + 26)) return false;
                if(!readUInt16(nameLen)) return false;
                if(!readUInt16(fieldLen)) return false;
                if(!seek(fileOff + 30 + nameLen + fieldLen)) return false;
                compressionMethod = method;
                compressedSize = packedSize;
                uncompressedSize = unpackedSize;
                return true;
            }
        }
        if(!seek(off + 46 + nameLen + fieldLen + cmtLen)) return false;
    }
    return false;
}

bool ZipFileReader::gotoClassFile(const char *name, uint16_t length) {
    if(length == 0xFFFF) length = strlen(name);

    int32_t fileCount = gotoCDFH();
    if(fileCount < 0) return false;

    for(uint32_t i = 0; i < fileCount; i++) {
        uint32_t off = tell();

        if(!seek(off + 28)) return false;

        uint16_t nameLen, fieldLen, cmtLen;
        if(!readUInt16(nameLen)) return false;
        if(!readUInt16(fieldLen)) return false;
        if(!readUInt16(cmtLen)) return false;

        uint32_t fileOff;
        if(!seek(off + 42)) return false;
        if(!readUInt32(fileOff)) return false;

        if(nameLen == (length + 6)) {
            bool isOk = true;
            char buff[16];
            uint32_t N = length / sizeof(buff);
            for(uint32_t i = 0; isOk && (i < N); i++) {
                if(read(buff, sizeof(buff)) != sizeof(buff)) return false;
                if(strncmp(&name[i * sizeof(buff)], buff, sizeof(buff)) != 0)
                    isOk = false;
            }
            N = length % sizeof(buff);
            if(isOk && N > 0) {
                if(read(buff, N) != N) return false;
                isOk = strncmp(&name[length - N], buff, N) == 0;
            }
            if(isOk) {
                if(read(buff, 6) != 6) return false;
                isOk = strncmp(".class", buff, 6) == 0;
            }
            if(isOk) {
                if(!seek(fileOff + 26)) return false;
                if(!readUInt16(nameLen)) return false;
                if(!readUInt16(fieldLen)) return false;
                if(!seek(fileOff + 30 + nameLen + fieldLen)) return false;
                return true;
            }
        }
        if(!seek(off + 46 + nameLen + fieldLen + cmtLen)) return false;
    }
    return false;
}

uint16_t ZipFileReader::getCompressionMethod(void) const {
    return compressionMethod;
}

uint32_t ZipFileReader::getCompressedSize(void) const {
    return compressedSize;
}

uint32_t ZipFileReader::getUncompressedSize(void) const {
    return uncompressedSize;
}
