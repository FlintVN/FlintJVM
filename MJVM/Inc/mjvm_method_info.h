
#ifndef __MJVM_METHOD_INFO_H
#define __MJVM_METHOD_INFO_H

#include "mjvm_const_pool.h"
#include "mjvm_attribute_info.h"

class MethodInfo {
public:
    const MethodAccessFlag accessFlag;
    const ConstUtf8 &name;
    const ConstUtf8 &descriptor;
    const uint16_t attributesCount;
private:
    const AttributeInfo **attributes;

    MethodInfo(const MethodInfo &) = delete;
    void operator=(const MethodInfo &) = delete;

    void setAttributes(AttributeInfo **attributes, uint16_t length);

    friend class ClassLoader;
public:
    MethodInfo(MethodAccessFlag accessFlag, const ConstUtf8 &name, const ConstUtf8 &descriptor);

    const AttributeInfo &getAttribute(uint16_t index) const;
    const AttributeInfo &getAttribute(AttributeType type) const;
    const AttributeCode &getAttributeCode(void) const;

    ~MethodInfo(void);
};

#endif /* __MJVM_METHOD_INFO_H */
