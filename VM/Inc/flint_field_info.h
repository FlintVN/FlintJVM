
#ifndef __FLINT_FIELD_INFO_H
#define __FLINT_FIELD_INFO_H

#include "flint_const_pool.h"

class FlintFieldInfo {
public:
    const FlintFieldAccessFlag accessFlag;
    class FlintClassLoader &classLoader;
    uint16_t nameIndex;
    uint16_t descIndex;
private:
    FlintFieldInfo(FlintClassLoader &classLoader, FlintFieldAccessFlag accessFlag, uint16_t nameIndex, uint16_t descIndex);

    FlintFieldInfo(const FlintFieldInfo &) = delete;
    void operator=(const FlintFieldInfo &) = delete;

public:
    FlintConstUtf8 &getName(void) const;
    FlintConstUtf8 &getDescriptor(void) const;

    friend class FlintClassLoader;
};

#endif /* __FLINT_FIELD_INFO_H */
