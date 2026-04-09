
#ifndef __FLINT_HOOK_H
#define __FLINT_HOOK_H

#include "flint_std.h"
#include "flint_list.h"

class Hook : public ListNode {
public:
    void * const handle;
    void (* const func)(void *);
public:
    Hook(void *handle, void (*func)(void *));

    void *getHandle(void) const;
    void (*getFunc(void))(void *) const;

    void invoke(void) const;
private:
    Hook(const Hook &) = delete;
    void operator=(const Hook &) = delete;
};

#endif /* __FLINT_HOOK_H */
