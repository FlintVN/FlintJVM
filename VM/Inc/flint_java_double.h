
#ifndef __FLINT_JAVA_DOUBLE_H
#define __FLINT_JAVA_DOUBLE_H

#include "flint_java_object.h"

class FlintJavaDouble : public FlintJavaObject {
public:
    double getValue(void) const;
    void setValue(double value);
protected:
    FlintJavaDouble(void) = delete;
    FlintJavaDouble(const FlintJavaDouble &) = delete;
    void operator=(const FlintJavaDouble &) = delete;
};

#endif /* __FLINT_JAVA_DOUBLE_H */
