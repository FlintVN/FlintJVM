
#ifndef __FLINT_LOAD_FILE_ERROR_H
#define __FLINT_LOAD_FILE_ERROR_H

class FlintLoadFileError {
public:
    const char *getFileName(void) const;
private:
    FlintLoadFileError(void) = delete;
    FlintLoadFileError(const FlintLoadFileError &) = delete;
    void operator=(const FlintLoadFileError &) = delete;
};

#endif /* __FLINT_LOAD_FILE_ERROR_H */
