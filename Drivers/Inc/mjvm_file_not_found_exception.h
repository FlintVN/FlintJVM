
#ifndef __MJVM_FILE_NOT_FOUND_EXCEPTION_H
#define __MJVM_FILE_NOT_FOUND_EXCEPTION_H

class FileNotFound {
public:
    const char *getFileName(void) const;
private:
    FileNotFound(void) = delete;
    FileNotFound(const FileNotFound &) = delete;
    void operator=(const FileNotFound &) = delete;
};

#endif /* __MJVM_FILE_NOT_FOUND_EXCEPTION_H */
