
#ifndef __FLINT_HOOK_H
#define __FLINT_HOOK_H

#include "flint_std.h"
#include "flint_list.h"

class Hook : public ListNode {
private:
    void * const handle;
    void (* const func)(void *);

    Hook(void *handle, void (*func)(void *));
    void invoke(void) const;
public:
    void *getHandle(void) const;
    void (*getFunc(void))(void *) const;
private:
    Hook(const Hook &) = delete;
    void operator=(const Hook &) = delete;

    friend class Flint;
};

#endif /* __FLINT_HOOK_H */
