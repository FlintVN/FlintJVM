
#ifndef __FLINT_FIELD_INFO_H
#define __FLINT_FIELD_INFO_H

#include "flint_std.h"
#include "flint_const_pool.h"

typedef enum : uint16_t {
    FIELD_PUBLIC = 0x0001,
    FIELD_PRIVATE = 0x0002,
    FIELD_PROTECTED = 0x0004,
    FIELD_STATIC = 0x0008,
    FIELD_FINAL = 0x0010,
    FIELD_VOLATILE = 0x0040,
    FIELD_TRANSIENT = 0x0080,
    FIELD_SYNTHETIC = 0x1000,
    FIELD_ENUM = 0x4000,
    FIELD_UNLOAD = 0x8000,
} FieldAccessFlag;

class FieldInfo {
public:
    const FieldAccessFlag accessFlag;
    const char * const name;
    const char * const desc;
    const uint32_t hash;

    void free(FieldInfo *fieldInfo);
private:
    FieldInfo(FieldAccessFlag accessFlag, const char *name, const char *desc);

    FieldInfo(const FieldInfo &) = delete;
    void operator=(const FieldInfo &) = delete;

    friend class ClassLoader;
};

#endif /* __FLINT_FIELD_INFO_H */
