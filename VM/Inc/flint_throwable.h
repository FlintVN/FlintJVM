
#ifndef __FLINT_THROWABLE_H
#define __FLINT_THROWABLE_H

#include "flint_string.h"

class FlintThrowable : public FlintObject {
public:
    FlintString &getDetailMessage(void) const;
protected:
    FlintThrowable(void) = delete;
    FlintThrowable(const FlintThrowable &) = delete;
    void operator=(const FlintThrowable &) = delete;
};

#endif /* __FLINT_THROWABLE_H */
