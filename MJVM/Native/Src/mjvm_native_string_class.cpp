
#include "mjvm.h"
#include "mjvm_object.h"
#include "mjvm_const_name.h"
#include "mjvm_native_string_class.h"

static bool nativeIntern(Execution &execution) {
    MjvmString *obj = (MjvmString *)execution.stackPopObject();
    Mjvm::lock();
    execution.stackPushObject(execution.getConstString(*obj));
    Mjvm::unlock();
    return true;
}

static const NativeMethod methods[] = {
    NATIVE_METHOD("\x06\x00""intern", "\x14\x00""()Ljava/lang/String;", nativeIntern),
};

const NativeClass STRING_CLASS = NATIVE_CLASS(stringClassName, methods);
