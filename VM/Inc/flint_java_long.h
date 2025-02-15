
#ifndef __FLINT_JAVA_LONG_H
#define __FLINT_JAVA_LONG_H

#include "flint_java_object.h"

class FlintJavaLong : public FlintJavaObject {
public:
    int64_t getValue(void) const;
    void setValue(int64_t value);
protected:
    FlintJavaLong(void) = delete;
    FlintJavaLong(const FlintJavaLong &) = delete;
    void operator=(const FlintJavaLong &) = delete;
};

#endif /* __FLINT_JAVA_LONG_H */
