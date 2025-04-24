
#ifndef __FLINT_FIELD_INFO_H
#define __FLINT_FIELD_INFO_H

#include "flint_const_pool.h"

class FlintFieldInfo {
public:
    const FlintFieldAccessFlag accessFlag;
    class FlintClassLoader &classLoader;
    FlintConstUtf8 &name;
    FlintConstUtf8 &descriptor;
private:
    FlintFieldInfo(FlintClassLoader &classLoader, FlintFieldAccessFlag accessFlag, const FlintConstUtf8 &name, const FlintConstUtf8 &descriptor);

    FlintFieldInfo(const FlintFieldInfo &) = delete;
    void operator=(const FlintFieldInfo &) = delete;

    friend class FlintClassLoader;
};

#endif /* __FLINT_FIELD_INFO_H */
