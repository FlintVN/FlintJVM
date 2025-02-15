
#ifndef __FLINT_JAVA_BYTE_H
#define __FLINT_JAVA_BYTE_H

#include "flint_java_object.h"

class FlintJavaByte : public FlintJavaObject {
public:
    int8_t getValue(void) const;
    void setValue(int8_t value);
protected:
    FlintJavaByte(void) = delete;
    FlintJavaByte(const FlintJavaByte &) = delete;
    void operator=(const FlintJavaByte &) = delete;
};

#endif /* __FLINT_JAVA_BYTE_H */
