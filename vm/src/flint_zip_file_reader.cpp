
#include <string.h>
#include <algorithm>
#include "flint.h"
#include "flint_zip_file_reader.h"

const char *ZipEntry::getZipFilePath(void) const {
    return zipFile;
}

int32_t ZipEntry::getNameLength(void) const {
    return nameLen;
}

ZipFileReader::ZipFileReader(void) : FileReader() {
    entryCount = -1;
}

ZipFileReader::ZipFileReader(FExec *ctx, const char *filePath) : FileReader(ctx, filePath) {
    entryCount = -1;
}

bool ZipFileReader::gotoEOCD(void) {
    return seek(size() - 22);
}

int32_t ZipFileReader::gotoCDFH(void) {
    if (!gotoEOCD()) return -1;
    uint16_t fileCount;
    uint32_t magicNum, CDFHPos;
    uint32_t pos = tell();

    if (!readUInt32(magicNum)) return -1;
    if (!seek(pos + 10)) return -1;
    if (!readUInt16(fileCount)) return -1;
    if (!seek(pos + 16)) return -1;
    if (!readUInt32(CDFHPos)) return -1;
    if (!seek(CDFHPos)) return -1;

    return fileCount;
}

void ZipFileReader::reset(void) {
    entryCount = -1;
    entryOffset = -1;
}

ZipEntry *ZipFileReader::nextEntry(void) {
    if (entryCount < 0) {
        entryCount = gotoCDFH();
        if (entryCount < 0) return NULL;
        entryIndex = 0;
        nextEntryOffset = tell();
        entryOffset = (uint32_t)-1;
    }

    if (entryIndex < entryCount) {
        if (!seek(nextEntryOffset + 28)) return NULL;
        uint16_t nameLen, fieldLen, cmtLen;
        if (!readUInt16(nameLen)) return NULL;
        if (!readUInt16(fieldLen)) return NULL;
        if (!readUInt16(cmtLen)) return NULL;
        entryIndex++;
        entryOffset = nextEntryOffset;
        entryNameLen = nameLen;
        nextEntryOffset += 46 + nameLen + fieldLen + cmtLen;
        return (ZipEntry *)&filePath;
    }
    else
        entryOffset = (uint32_t)-1;
    return NULL;
}

bool ZipFileReader::readEntryName(char *name, uint16_t size) {
    if (entryOffset == (uint32_t)-1)
        return false;
    if (!seek(entryOffset + 46)) return false;
    size = std::min<uint16_t>(size, entryNameLen);
    if (read(name, size) != size) return false;
    return true;
}

bool ZipFileReader::openEntry(void) {
    uint16_t nameLen, fieldLen;
    uint32_t fileOff;

    if (entryOffset == (uint32_t)-1)
        return false;
    if (!seek(entryOffset + 42)) return false;
    if (!readUInt32(fileOff)) return false;

    if (!seek(fileOff + 26)) return false;
    if (!readUInt16(nameLen)) return false;
    if (!readUInt16(fieldLen)) return false;
    if (!seek(fileOff + 30 + nameLen + fieldLen)) return false;

    return true;
}

bool ZipFileReader::openEntry(ZipEntry *entry) {
    uint16_t nameLen, fieldLen;
    uint32_t fileOff;

    if (entry == NULL || entry->offset == (uint32_t)-1)
        return false;
    if (!seek(entry->offset + 42)) return false;
    if (!readUInt32(fileOff)) return false;

    if (!seek(fileOff + 26)) return false;
    if (!readUInt16(nameLen)) return false;
    if (!readUInt16(fieldLen)) return false;
    if (!seek(fileOff + 30 + nameLen + fieldLen)) return false;

    return true;
}

bool ZipFileReader::openEntry(const char *name, uint16_t length) {
    if (length == 0xFFFF) length = strlen(name);
    entryCount = -1;
    while (true) {
        ZipEntry *e = nextEntry();
        if (e == NULL) return false;
        if (e->nameLen == length) {
            bool isOk = true;
            char buff[32];
            uint32_t N = length / sizeof(buff);
            if (!seek(entryOffset + 46)) return false;
            for (uint32_t i = 0; isOk && (i < N); i++) {
                if (read(buff, sizeof(buff)) != sizeof(buff)) return false;
                if (strncmp(&name[i * sizeof(buff)], buff, sizeof(buff)) != 0) {
                    isOk = false;
                    break;
                }
            }
            N = length % sizeof(buff);
            if (isOk && N > 0) {
                if (read(buff, N) != N) return false;
                isOk = strncmp(&name[length - N], buff, N) == 0;
            }
            if (isOk)
                return openEntry();
        }
    }

    return false;
}

bool ZipFileReader::openClassFileEntry(const char *name, uint16_t length) {
    if (length == 0xFFFF) length = strlen(name);
    entryCount = -1;
    uint32_t len = length + 6;
    while (true) {
        ZipEntry *e = nextEntry();
        if (e == NULL) return false;
        if (e->nameLen == len) {
            bool isOk = true;
            char buff[32];
            uint32_t N = length / sizeof(buff);
            if (!seek(entryOffset + 46)) return false;
            for (uint32_t i = 0; isOk && (i < N); i++) {
                if (read(buff, sizeof(buff)) != sizeof(buff)) return false;
                if (strncmp(&name[i * sizeof(buff)], buff, sizeof(buff)) != 0) {
                    isOk = false;
                    break;
                }
            }
            N = length % sizeof(buff);
            if (isOk && N > 0) {
                if (read(buff, N) != N) return false;
                isOk = strncmp(&name[length - N], buff, N) == 0;
            }
            if (isOk) {
                if (read(buff, 6) != 6) return false;
                isOk = strncmp(".class", buff, 6) == 0;
            }
            if (isOk)
                return openEntry();
        }
    }
    return false;
}
