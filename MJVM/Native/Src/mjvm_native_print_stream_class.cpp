
#include "mjvm_string.h"
#include "mjvm_const_name.h"
#include "mjvm_system_api.h"
#include "mjvm_native_print_stream_class.h"

static bool nativeWrite(MjvmExecution &execution) {
    MjvmString *str = (MjvmString *)execution.stackPopObject();
    if(str == 0)
        MjvmSystem_Write("null", 4, 0);
    else
        MjvmSystem_Write(str->getText(), str->getLength(), str->getCoder());
    return true;
}

static bool nativeWriteln(MjvmExecution &execution) {
    if(!nativeWrite(execution))
        return false;
    MjvmSystem_Write("\n", 1, 0);
    return true;
}

static const NativeMethod methods[] = {
    NATIVE_METHOD("\x05\x00\x2B\x02""write",   "\x15\x00\x47\x07""(Ljava/lang/String;)V", nativeWrite),
    NATIVE_METHOD("\x07\x00\x05\x03""writeln", "\x15\x00\x47\x07""(Ljava/lang/String;)V", nativeWriteln),
};

const NativeClass PRINT_STREAM_CLASS = NATIVE_CLASS(printStreamClassName, methods);
