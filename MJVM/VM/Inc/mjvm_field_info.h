
#ifndef __MJVM_FIELD_INFO_H
#define __MJVM_FIELD_INFO_H

#include "mjvm_const_pool.h"
#include "mjvm_attribute_info.h"

class MjvmClassLoader;

class MjvmFieldInfo {
public:
    const MjvmFieldAccessFlag accessFlag;
    MjvmClassLoader &classLoader;
    MjvmConstUtf8 &name;
    MjvmConstUtf8 &descriptor;
private:
    MjvmFieldInfo(MjvmClassLoader &classLoader, MjvmFieldAccessFlag accessFlag, MjvmConstUtf8 &name, MjvmConstUtf8 &descriptor);

    MjvmFieldInfo(const MjvmFieldInfo &) = delete;
    void operator=(const MjvmFieldInfo &) = delete;

    friend class MjvmClassLoader;
public:
    MjvmAttribute &getAttribute(MjvmAttributeType type) const;
};

#endif /* __MJVM_FIELD_INFO_H */
