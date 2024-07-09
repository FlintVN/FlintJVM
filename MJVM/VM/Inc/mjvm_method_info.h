
#ifndef __MJVM_METHOD_INFO_H
#define __MJVM_METHOD_INFO_H

#include "mjvm_const_pool.h"
#include "mjvm_attribute_info.h"

class MjvmClassLoader;

class MjvmMethodInfo {
public:
    const MjvmMethodAccessFlag accessFlag;
    MjvmClassLoader &classLoader;
    MjvmConstUtf8 &name;
    MjvmConstUtf8 &descriptor;
private:
    MjvmAttribute *attributes;

    MjvmMethodInfo(MjvmClassLoader &classLoader, MjvmMethodAccessFlag accessFlag, MjvmConstUtf8 &name, MjvmConstUtf8 &descriptor);

    MjvmMethodInfo(const MjvmMethodInfo &) = delete;
    void operator=(const MjvmMethodInfo &) = delete;

    void addAttribute(MjvmAttribute *attribute);

    friend class MjvmClassLoader;
public:
    MjvmAttribute &getAttribute(MjvmAttributeType type) const;
    MjvmCodeAttribute &getAttributeCode(void) const;
    MjvmNativeAttribute &getAttributeNative(void) const;

    ~MjvmMethodInfo(void);
};

#endif /* __MJVM_METHOD_INFO_H */
