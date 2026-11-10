
#include "flint.h"
#include "flint_system_api.h"
#include "flint_native_runtime.h"

jvoid NativeRuntime_ExitInternal(FNIEnv *env, jobject obj, jint status) {
    (void)obj;
    Flint *flint = ((FExec *)env)->getFlint();
    flint->setExitCode(status);
    flint->terminateRequest();
}

jlong NativeRuntime_FreeMemory(FNIEnv *env, jobject obj) {
    (void)env;
    (void)obj;
    return FlintAPI::System::freeMemory();
}

jlong NativeRuntime_TotalMemory(FNIEnv *env, jobject obj) {
    (void)env;
    (void)obj;
    return FlintAPI::System::totalMemory();
}

jvoid NativeRuntime_Gc(FNIEnv *env, jobject obj) {
    (void)env;
    (void)obj;
    // TODO
}
