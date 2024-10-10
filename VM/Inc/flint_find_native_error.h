
#ifndef __FLINT_FIND_NATIVE_ERROR_H
#define __FLINT_FIND_NATIVE_ERROR_H

class FlintFindNativeError {
public:
    const char *getMessage(void) const;
private:
    FlintFindNativeError(void) = delete;
    FlintFindNativeError(const FlintFindNativeError &) = delete;
    void operator=(const FlintFindNativeError &) = delete;
};

#endif /* __FLINT_FIND_NATIVE_ERROR_H */
