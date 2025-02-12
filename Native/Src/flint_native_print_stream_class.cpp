
#include "flint.h"
#include "flint_system_api.h"
#include "flint_native_print_stream_class.h"

static void nativeWrite(FlintExecution &execution) {
    FlintJavaString *str = (FlintJavaString *)execution.stackPopObject();
    if(str == 0)
        execution.flint.print("null", 4, 0);
    else
        execution.flint.print(str->getText(), str->getLength(), str->getCoder());
}

static void nativeWriteln(FlintExecution &execution) {
    nativeWrite(execution);
    execution.flint.print("\n", 1, 0);
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x05\x00\x03\xBB""write",   "\x15\x00\xA0\xD0""(Ljava/lang/String;)V", nativeWrite),
    NATIVE_METHOD("\x07\x00\x12\x20""writeln", "\x15\x00\xA0\xD0""(Ljava/lang/String;)V", nativeWriteln),
};

const FlintNativeClass PRINT_STREAM_CLASS = NATIVE_CLASS(printStreamClassName, methods);
