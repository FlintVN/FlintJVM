
#ifndef __FLINT_FILE_READER_H
#define __FLINT_FILE_READER_H

#include "flint_system_api.h"

class FileReader {
protected:
    FlintAPI::IO::FileHandle handle;
    class FExec *ctx;
    const char *filePath;
public:
    FileReader(void);
    FileReader(class FExec *ctx, const char *filePath);

    bool open(void);
    bool close(void);

    int32_t read(void *buff, uint32_t size);

    bool readUInt8(uint8_t &value);
    bool readUInt16(uint16_t &value);
    bool readUInt32(uint32_t &value);
    bool readUInt64(uint64_t &value);

    bool readSwapUInt16(uint16_t &value);
    bool readSwapUInt32(uint32_t &value);
    bool readSwapUInt64(uint64_t &value);

    int32_t readLine(char *buff, uint32_t size);

    uint32_t tell(void);
    bool seek(int32_t offset);
    bool offset(int32_t offset);
    uint32_t size(void);

    const char *getFilePath(void);
    class FExec *getContext(void);
private:
    FileReader(const FileReader &) = delete;
    void operator=(const FileReader &) = delete;
};

#endif /* __FLINT_FILE_READER_H */
