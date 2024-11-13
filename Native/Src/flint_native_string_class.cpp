
#include "flint.h"
#include "flint_object.h"
#include "flint_const_name.h"
#include "flint_native_string_class.h"

static void nativeIntern(FlintExecution &execution) {
    FlintString *obj = (FlintString *)execution.stackPopObject();
    Flint::lock();
    execution.stackPushObject(&execution.flint.getConstString(*obj));
    Flint::unlock();
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x06\x00\xB1\xB7""intern", "\x14\x00\xA7\xAF""()Ljava/lang/String;", nativeIntern),
};

const FlintNativeClass STRING_CLASS = NATIVE_CLASS(stringClassName, methods);
