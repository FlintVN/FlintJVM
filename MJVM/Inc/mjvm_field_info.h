
#ifndef __MJVM_FIELD_INFO_H
#define __MJVM_FIELD_INFO_H

#include "mjvm_const_pool.h"
#include "mjvm_attribute_info.h"

class FieldInfo {
public:
    const FieldAccessFlag accessFlag;
    const ConstUtf8 &name;
    const ConstUtf8 &descriptor;
    const uint16_t attributesCount;
private:
    const AttributeInfo **attributes;

    FieldInfo(const FieldInfo &) = delete;
    void operator=(const FieldInfo &) = delete;

    void setAttributes(AttributeInfo **attributes, uint16_t length);

    friend class ClassLoader;
public:
    FieldInfo(FieldAccessFlag accessFlag, const ConstUtf8 &name, const ConstUtf8 &descriptor);

    const AttributeInfo &getAttribute(uint16_t index) const;
    const AttributeInfo &getAttribute(AttributeType type) const;

    ~FieldInfo(void);
};

#endif /* __MJVM_FIELD_INFO_H */
