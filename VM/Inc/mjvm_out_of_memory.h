
#ifndef __MJVM_OUT_OF_MEMORY_H
#define __MJVM_OUT_OF_MEMORY_H

class MjvmOutOfMemoryError {
public:
    const char *getMessage(void) const;
private:
    MjvmOutOfMemoryError(void) = delete;
    MjvmOutOfMemoryError(const MjvmOutOfMemoryError &) = delete;
    void operator=(const MjvmOutOfMemoryError &) = delete;
};

#endif /* __MJVM_OUT_OF_MEMORY_H */
