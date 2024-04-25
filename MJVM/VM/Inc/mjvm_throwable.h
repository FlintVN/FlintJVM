
#ifndef __MJVM_THROWABLE_H
#define __MJVM_THROWABLE_H

#include "mjvm_string.h"

class MjvmThrowable : public MjvmObject {
public:
    MjvmString &getDetailMessage(void) const;
protected:
    MjvmThrowable(void) = delete;
    MjvmThrowable(const MjvmThrowable &) = delete;
    void operator=(const MjvmThrowable &) = delete;
};

#endif /* __MJVM_THROWABLE_H */
