
#include "flint_hook.h"

Hook::Hook(void *handle, void (*func)(void *)) : handle(handle), func(func) {

}

void *Hook::getHandle(void) const {
    return handle;
}

void (*Hook::getFunc(void))(void *) {
    return func;
}

void Hook::invoke(void) const {
    func(handle);
}
