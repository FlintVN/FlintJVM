
#include "flint.h"
#include "flint_java_object.h"
#include "flint_const_name_base.h"
#include "flint_native_string_class.h"
#include "flint_throw_support.h"

static FlintError nativeIntern(FlintExecution &execution) {
    FlintJavaString *obj = (FlintJavaString *)execution.stackPopObject();
    auto strIntern = execution.flint.getConstString(*obj);
    if(strIntern.err == ERR_OK)
        execution.stackPushObject(strIntern.value);
    return checkAndThrowForFlintError(execution, strIntern.err, strIntern.getErrorMsg(), strIntern.getErrorMsgLength());
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x06\x00\xB1\xB7""intern", "\x14\x00\xA7\xAF""()Ljava/lang/String;", nativeIntern),
};

const FlintNativeClass STRING_CLASS = NATIVE_CLASS(stringClassName, methods);
