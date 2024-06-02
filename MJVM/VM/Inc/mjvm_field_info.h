
#ifndef __MJVM_FIELD_INFO_H
#define __MJVM_FIELD_INFO_H

#include "mjvm_const_pool.h"
#include "mjvm_attribute_info.h"

class ClassLoader;

class FieldInfo {
public:
    const FieldAccessFlag accessFlag;
    ClassLoader &classLoader;
    ConstUtf8 &name;
    ConstUtf8 &descriptor;
private:
    FieldInfo(ClassLoader &classLoader, FieldAccessFlag accessFlag, ConstUtf8 &name, ConstUtf8 &descriptor);

    FieldInfo(const FieldInfo &) = delete;
    void operator=(const FieldInfo &) = delete;

    friend class ClassLoader;
public:
    AttributeInfo &getAttribute(AttributeType type) const;
};

#endif /* __MJVM_FIELD_INFO_H */
