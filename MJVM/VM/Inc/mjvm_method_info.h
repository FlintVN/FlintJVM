
#ifndef __MJVM_METHOD_INFO_H
#define __MJVM_METHOD_INFO_H

#include "mjvm_const_pool.h"
#include "mjvm_attribute_info.h"

class ClassLoader;

class MethodInfo {
public:
    const MethodAccessFlag accessFlag;
    ClassLoader &classLoader;
    ConstUtf8 &name;
    ConstUtf8 &descriptor;
private:
    AttributeInfo *attributes;

    MethodInfo(ClassLoader &classLoader, MethodAccessFlag accessFlag, ConstUtf8 &name, ConstUtf8 &descriptor);

    MethodInfo(const MethodInfo &) = delete;
    void operator=(const MethodInfo &) = delete;

    void addAttribute(AttributeInfo *attribute);

    friend class ClassLoader;
public:
    AttributeInfo &getAttribute(AttributeType type) const;
    AttributeCode &getAttributeCode(void) const;
    AttributeNative &getAttributeNative(void) const;

    ~MethodInfo(void);
};

#endif /* __MJVM_METHOD_INFO_H */
