
#include "flint.h"
#include "flint_file_reader.h"

static uint16_t Swap16(uint16_t value) {
    return ((value) << 8) | ((value) >> 8);
}

static uint32_t Swap32(uint32_t value) {
    uint32_t ret;
    ((uint8_t *)&ret)[0] = ((uint8_t *)&value)[3];
    ((uint8_t *)&ret)[1] = ((uint8_t *)&value)[2];
    ((uint8_t *)&ret)[2] = ((uint8_t *)&value)[1];
    ((uint8_t *)&ret)[3] = ((uint8_t *)&value)[0];
    return ret;
}

static uint64_t Swap64(uint64_t value) {
    uint64_t ret;
    ((uint8_t *)&ret)[0] = ((uint8_t *)&value)[7];
    ((uint8_t *)&ret)[1] = ((uint8_t *)&value)[6];
    ((uint8_t *)&ret)[2] = ((uint8_t *)&value)[5];
    ((uint8_t *)&ret)[3] = ((uint8_t *)&value)[4];
    ((uint8_t *)&ret)[4] = ((uint8_t *)&value)[3];
    ((uint8_t *)&ret)[5] = ((uint8_t *)&value)[2];
    ((uint8_t *)&ret)[6] = ((uint8_t *)&value)[1];
    ((uint8_t *)&ret)[7] = ((uint8_t *)&value)[0];
    return ret;
}

static const char *GetName(const char *filePath) {
    const char *name = filePath;
    const char *path = filePath;
    const char separatorChar = Flint::getPathSeparator();
    while(*path) {
        if(*path == separatorChar)
            name = path;
        path++;
    }
    return name;
}

FileReader::FileReader(FExec *ctx) : handle(NULL), ctx(ctx), filePath(NULL) {

}

FileReader::FileReader(FExec *ctx, const char *filePath) :
handle(NULL), ctx(ctx), filePath(filePath) {

}

bool FileReader::open(void) {
    if(handle == NULL) {
        handle = FlintAPI::IO::fopen(filePath, FlintAPI::IO::FILE_MODE_READ);
        if(handle == NULL && ctx != NULL)
            ctx->throwNew(Flint::findClass(ctx, "java/io/IOException"), "Failed to open file %s", GetName(filePath));
        return handle != NULL;
    }
    return true;
}

bool FileReader::close(void) {
    if(FlintAPI::IO::fclose(handle) != FlintAPI::IO::FILE_RESULT_OK) {
        if(ctx != NULL)
            ctx->throwNew(Flint::findClass(ctx, "java/io/IOException"), "FlintAPI::IO::fclose failed");
        return false;
    }
    handle = NULL;
    return true;
}

bool FileReader::read(void *buff, uint32_t size) {
    uint32_t temp;
    FlintAPI::IO::FileResult ret = FlintAPI::IO::fread(handle, buff, size, &temp);
    if((ret != FlintAPI::IO::FILE_RESULT_OK) || (temp != size)) {
        if(ctx != NULL)
            ctx->throwNew(Flint::findClass(ctx, "java/io/IOException"), "FlintAPI::IO::fread failed");
        return false;
    }
    return true;
}

bool FileReader::readUInt8(uint8_t &value) {
    return read(&value, sizeof(uint8_t));
}

bool FileReader::readUInt16(uint16_t &value) {
    if(!read(&value, sizeof(uint16_t))) return false;
    return true;
}

bool FileReader::readUInt32(uint32_t &value) {
    if(!read(&value, sizeof(uint32_t))) return false;
    return true;
}

bool FileReader::readUInt64(uint64_t &value) {
    if(!read(&value, sizeof(uint64_t))) return false;
    return true;
}

bool FileReader::readSwapUInt16(uint16_t &value) {
    if(!read(&value, sizeof(uint16_t))) return false;
    value = Swap16(value);
    return true;
}

bool FileReader::readSwapUInt32(uint32_t &value) {
    if(!read(&value, sizeof(uint32_t))) return false;
    value = Swap32(value);
    return true;
}

bool FileReader::readSwapUInt64(uint64_t &value) {
    if(!read(&value, sizeof(uint64_t))) return false;
    value = Swap64(value);
    return true;
}

uint32_t FileReader::tell(void) {
    return FlintAPI::IO::ftell(handle);
}

bool FileReader::seek(int32_t offset) {
    if(FlintAPI::IO::fseek(handle, offset) != FlintAPI::IO::FILE_RESULT_OK) {
        if(ctx != NULL)
            ctx->throwNew(Flint::findClass(ctx, "java/io/IOException"), "FlintAPI::IO::fseek failed");
        return false;
    }
    return true;
}

bool FileReader::offset(int32_t offset) {
    return seek(tell() + offset);
}

uint32_t FileReader::size(void) {
    return FlintAPI::IO::fsize(handle);
}

const char *FileReader::getFilePath(void) {
    return filePath;
}

FExec *FileReader::getContext(void) {
    return ctx;
}
