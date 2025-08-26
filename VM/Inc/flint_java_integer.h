
#ifndef __FLINT_JAVA_INTEGER_H
#define __FLINT_JAVA_INTEGER_H

#include "flint_java_object.h"

class FlintJavaInteger : public JObject {
public:
    int32_t getValue(void) const;
    void setValue(int32_t value);
protected:
    FlintJavaInteger(void) = delete;
    FlintJavaInteger(const FlintJavaInteger &) = delete;
    void operator=(const FlintJavaInteger &) = delete;
};

#endif /* __FLINT_JAVA_INTEGER_H */
