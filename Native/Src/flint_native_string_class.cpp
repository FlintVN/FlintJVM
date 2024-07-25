
#include "flint.h"
#include "flint_object.h"
#include "flint_const_name.h"
#include "flint_native_string_class.h"

static bool nativeIntern(FlintExecution &execution) {
    FlintString *obj = (FlintString *)execution.stackPopObject();
    execution.stackPushObject(execution.flint.getConstString(*obj));
    return true;
}

static const NativeMethod methods[] = {
    NATIVE_METHOD("\x06\x00\x90\x02""intern", "\x14\x00\xF1\x06""()Ljava/lang/String;", nativeIntern),
};

const NativeClass STRING_CLASS = NATIVE_CLASS(stringClassName, methods);
