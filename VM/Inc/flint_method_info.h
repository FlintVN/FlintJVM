
#ifndef __FLINT_METHOD_INFO_H
#define __FLINT_METHOD_INFO_H

#include "flint_const_pool.h"
#include "flint_attribute_info.h"

class FlintClassLoader;

class FlintMethodInfo {
public:
    const FlintMethodAccessFlag accessFlag;
    FlintClassLoader &classLoader;
    FlintConstUtf8 &name;
    FlintConstUtf8 &descriptor;
private:
    FlintAttribute *attributes;

    FlintMethodInfo(FlintClassLoader &classLoader, FlintMethodAccessFlag accessFlag, FlintConstUtf8 &name, FlintConstUtf8 &descriptor);

    FlintMethodInfo(const FlintMethodInfo &) = delete;
    void operator=(const FlintMethodInfo &) = delete;

    void addAttribute(FlintAttribute *attribute);

    friend class FlintClassLoader;
public:
    FlintAttribute &getAttribute(FlintAttributeType type) const;
    FlintCodeAttribute &getAttributeCode(void) const;
    FlintNativeAttribute &getAttributeNative(void) const;

    ~FlintMethodInfo(void);
};

#endif /* __FLINT_METHOD_INFO_H */
