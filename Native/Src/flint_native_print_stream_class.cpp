
#include "flint.h"
#include "flint_system_api.h"
#include "flint_native_print_stream_class.h"

static FlintError nativeWrite(FlintExecution *exec) {
    FlintJavaString *str = (FlintJavaString *)exec->stackPopObject();
    if(str == 0)
        exec->flint.print("null", 4, 0);
    else
        exec->flint.print(str->getText(), str->getLength(), str->getCoder());
    return ERR_OK;
}

static FlintError nativeWriteln(FlintExecution *exec) {
    nativeWrite(exec);
    exec->flint.print("\n", 1, 0);
    return ERR_OK;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x05\x00\x03\xBB""write",   "\x15\x00\xA0\xD0""(Ljava/lang/String;)V", nativeWrite),
    NATIVE_METHOD("\x07\x00\x12\x20""writeln", "\x15\x00\xA0\xD0""(Ljava/lang/String;)V", nativeWriteln),
};

const FlintNativeClass PRINT_STREAM_CLASS = NATIVE_CLASS(printStreamClassName, methods);
