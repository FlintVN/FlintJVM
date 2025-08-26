
#ifndef __FLINT_JAVA_FLOAT_H
#define __FLINT_JAVA_FLOAT_H

#include "flint_java_object.h"

class FlintJavaFloat : public JObject {
public:
    float getValue(void) const;
    void setValue(float value);
protected:
    FlintJavaFloat(void) = delete;
    FlintJavaFloat(const FlintJavaFloat &) = delete;
    void operator=(const FlintJavaFloat &) = delete;
};

#endif /* __FLINT_JAVA_FLOAT_H */
