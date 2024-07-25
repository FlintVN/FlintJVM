
#ifndef __FLINT_OUT_OF_MEMORY_H
#define __FLINT_OUT_OF_MEMORY_H

class FlintOutOfMemoryError {
public:
    const char *getMessage(void) const;
private:
    FlintOutOfMemoryError(void) = delete;
    FlintOutOfMemoryError(const FlintOutOfMemoryError &) = delete;
    void operator=(const FlintOutOfMemoryError &) = delete;
};

#endif /* __FLINT_OUT_OF_MEMORY_H */
