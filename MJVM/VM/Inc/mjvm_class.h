
#ifndef __MJVM_CLASS_H
#define __MJVM_CLASS_H

#include "mjvm_string.h"

class MjvmClass : public MjvmObject {
public:
    MjvmString &getName(void) const;
protected:
    MjvmClass(void) = delete;
    MjvmClass(const MjvmClass &) = delete;
    void operator=(const MjvmClass &) = delete;
};

class MjvmConstClass {
private:
    MjvmConstClass *next;
public:
    MjvmClass &mjvmClass;
private:
    MjvmConstClass(MjvmClass &mjvmClass);
    MjvmConstClass(const MjvmConstClass &) = delete;
    void operator=(const MjvmConstClass &) = delete;

    friend class Execution;
};

#endif /* __MJVM_CLASS_H */
