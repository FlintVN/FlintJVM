
#ifndef __FLINT_JAVA_SHORT_H
#define __FLINT_JAVA_SHORT_H

#include "flint_java_object.h"

class FlintJavaShort : public FlintJavaObject {
public:
    int16_t getValue(void) const;
    void setValue(int16_t value);
protected:
    FlintJavaShort(void) = delete;
    FlintJavaShort(const FlintJavaShort &) = delete;
    void operator=(const FlintJavaShort &) = delete;
};

#endif /* __FLINT_JAVA_SHORT_H */
