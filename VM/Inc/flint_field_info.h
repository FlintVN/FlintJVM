
#ifndef __FLINT_FIELD_INFO_H
#define __FLINT_FIELD_INFO_H

#include "flint_const_pool.h"
#include "flint_attribute_info.h"

class FlintClassLoader;

class FlintFieldInfo {
public:
    const FlintFieldAccessFlag accessFlag;
    FlintClassLoader &classLoader;
    FlintConstUtf8 &name;
    FlintConstUtf8 &descriptor;
private:
    FlintFieldInfo(FlintClassLoader &classLoader, FlintFieldAccessFlag accessFlag, FlintConstUtf8 &name, FlintConstUtf8 &descriptor);

    FlintFieldInfo(const FlintFieldInfo &) = delete;
    void operator=(const FlintFieldInfo &) = delete;

    friend class FlintClassLoader;
public:
    FlintAttribute &getAttribute(FlintAttributeType type) const;
};

#endif /* __FLINT_FIELD_INFO_H */
