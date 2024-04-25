
#ifndef __MJVM_STRING_H
#define __MJVM_STRING_H

#include "mjvm_object.h"

class MjvmString : public MjvmObject {
public:
    const char *getText(void) const;
    uint32_t getLength(void) const;
protected:
    MjvmString(void) = delete;
    MjvmString(const MjvmString &) = delete;
    void operator=(const MjvmString &) = delete;
};

#endif /* __MJVM_STRING_H */
