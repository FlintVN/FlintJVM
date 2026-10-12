
#ifndef __FLINT_ZIP_FILE_READER_H
#define __FLINT_ZIP_FILE_READER_H

#include "flint_file_reader.h"

class ZipEntry {
private:
    const char *zipFile;
    uint32_t offset;
    uint16_t nameLen;
public:
    const char *getZipFilePath(void) const;
    int32_t getNameLength(void) const;
private:
    friend class ZipFileReader;
};

class ZipFileReader : public FileReader {
private:
    uint32_t entryOffset;   /* Do not change the position or order of this field. */
    uint16_t entryNameLen;  /* Do not change the position or order of this field. */

    int32_t entryCount;
    uint32_t entryIndex;
    uint32_t nextEntryOffset;

    bool gotoEOCD(void);
    int32_t gotoCDFH(void);
public:
    ZipFileReader(void);
    ZipFileReader(class FExec *ctx, const char *filePath);

    void reset(void);
    ZipEntry *nextEntry(void);
    bool readEntryName(char *name, uint16_t size);
    bool openEntry(void);
    bool openEntry(ZipEntry *entry);
    bool openEntry(const char *name, uint16_t length = 0xFFFF);
    bool openClassFileEntry(const char *name, uint16_t length = 0xFFFF);
private:
    ZipFileReader(const ZipFileReader &) = delete;
    void operator=(const ZipFileReader &) = delete;
};

#endif /* __FLINT_ZIP_FILE_READER_H */
