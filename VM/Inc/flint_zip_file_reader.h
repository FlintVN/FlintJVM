
#ifndef __FLINT_ZIP_FILE_READER_H
#define __FLINT_ZIP_FILE_READER_H

#include "flint_file_reader.h"

class ZipFileReader : public FileReader {
private:
    bool gotoEOCD(void);
    int32_t gotoCDFH(void);
public:
    ZipFileReader(void);
    ZipFileReader(class FExec *ctx, const char *filePath);

    bool gotoFile(const char *name, uint16_t length = 0xFFFF);
    bool gotoClassFile(const char *name, uint16_t length = 0xFFFF);
private:
    ZipFileReader(const ZipFileReader &) = delete;
    void operator=(const ZipFileReader &) = delete;
};

#endif /* __FLINT_ZIP_FILE_READER_H */
