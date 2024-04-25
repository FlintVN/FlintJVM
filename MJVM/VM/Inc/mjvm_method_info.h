
#ifndef __MJVM_METHOD_INFO_H
#define __MJVM_METHOD_INFO_H

#include "mjvm_const_pool.h"
#include "mjvm_attribute_info.h"

class ClassLoader;

class MethodInfo {
public:
    const ClassLoader &classLoader;
    const MethodAccessFlag accessFlag;
    const ConstUtf8 &name;
    const ConstUtf8 &descriptor;
    const uint16_t attributesCount;
private:
    const AttributeInfo **attributes;

    MethodInfo(const ClassLoader &classLoader, MethodAccessFlag accessFlag, const ConstUtf8 &name, const ConstUtf8 &descriptor);

    MethodInfo(const MethodInfo &) = delete;
    void operator=(const MethodInfo &) = delete;

    void setAttributes(AttributeInfo **attributes, uint16_t length);

    friend class ClassLoader;
public:
    const AttributeInfo &getAttribute(uint16_t index) const;
    const AttributeInfo &getAttribute(AttributeType type) const;
    const AttributeCode &getAttributeCode(void) const;
    const AttributeNative &getAttributeNative(void) const;

    ParamInfo parseParamInfo(void) const;

    ~MethodInfo(void);
};

#endif /* __MJVM_METHOD_INFO_H */
