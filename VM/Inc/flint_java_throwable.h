
#ifndef __FLINT_JAVA_THROWABLE_H
#define __FLINT_JAVA_THROWABLE_H

#include "flint_java_string.h"

class JThrowable : public JObject {
public:
    JString *getDetailMessage(void) const;
    void setDetailMessage(JString *strObj);
protected:
    JThrowable(void) = delete;
    JThrowable(const JThrowable &) = delete;
    void operator=(const JThrowable &) = delete;
};

#endif /* __FLINT_JAVA_THROWABLE_H */
