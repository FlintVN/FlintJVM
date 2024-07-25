
#ifndef __FLINT_CLASS_H
#define __FLINT_CLASS_H

#include "flint_string.h"

class FlintClass : public FlintObject {
public:
    FlintString &getName(void) const;
protected:
    FlintClass(void) = delete;
    FlintClass(const FlintClass &) = delete;
    void operator=(const FlintClass &) = delete;
};

class FlintConstClass {
private:
    FlintConstClass *next;
public:
    FlintClass &flintClass;
private:
    FlintConstClass(FlintClass &flintClass);
    FlintConstClass(const FlintConstClass &) = delete;
    void operator=(const FlintConstClass &) = delete;

    friend class Flint;
};

#endif /* __FLINT_CLASS_H */
