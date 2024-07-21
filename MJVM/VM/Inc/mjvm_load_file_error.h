
#ifndef __MJVM_LOAD_FILE_ERROR_H
#define __MJVM_LOAD_FILE_ERROR_H

class MjvmLoadFileError {
public:
    const char *getFileName(void) const;
private:
    MjvmLoadFileError(void) = delete;
    MjvmLoadFileError(const MjvmLoadFileError &) = delete;
    void operator=(const MjvmLoadFileError &) = delete;
};

#endif /* __MJVM_LOAD_FILE_ERROR_H */
