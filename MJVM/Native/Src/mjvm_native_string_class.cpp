
#include "mjvm.h"
#include "mjvm_object.h"
#include "mjvm_const_name.h"
#include "mjvm_native_string_class.h"

static bool nativeIntern(MjvmExecution &execution) {
    MjvmString *obj = (MjvmString *)execution.stackPopObject();
    execution.stackPushObject(execution.getConstString(*obj));
    return true;
}

static const NativeMethod methods[] = {
    NATIVE_METHOD("\x06\x00\x90\x02""intern", "\x14\x00\xF1\x06""()Ljava/lang/String;", nativeIntern),
};

const NativeClass STRING_CLASS = NATIVE_CLASS(stringClassName, methods);
