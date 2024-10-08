

#include "flint.h"
#include "flint_thread.h"
#include "flint_system_api.h"
#include "flint_const_name.h"
#include "flint_native_thread_class.h"

static const uint32_t runnableRunFieldName[] = {
    (uint32_t)"\x03\x00\x89\x58""run",              /* field name */
    (uint32_t)"\x03\x00\x91\x99""()V"               /* field type */
};

static bool nativeStart0(FlintExecution &execution) {
    Flint &flint = execution.flint;
    FlintThread *threadObj = (FlintThread *)execution.stackPopObject();
    FlintObject *task = threadObj->getTask();
    FlintExecution &threadExecution = flint.newExecution();
    if(task == 0)
        task = threadObj;
    threadExecution.stackPushObject(task);
    threadExecution.run(flint.load(task->type).getMethodInfo(*(FlintConstNameAndType *)runnableRunFieldName));
    return true;
}

static bool nativeYield0(FlintExecution &execution) {
    FlintAPI::Thread::yield();
    return true;
}

static bool nativeInterrupt0(FlintExecution &execution) {
    // TODO
    return true;
}

static bool nativeCurrentThread(FlintExecution &execution) {
    // TODO
    return true;
}

static bool nativeSleep0(FlintExecution &execution) {
    int64_t millis = execution.stackPopInt64();
    FlintAPI::Thread::sleep(millis);
    return true;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x06\x00\xDC\x1F""start0",        "\x03\x00\x91\x99""()V",                 nativeStart0),
    NATIVE_METHOD("\x06\x00\x5C\x41""yield0",        "\x03\x00\x91\x99""()V",                  nativeYield0),
    NATIVE_METHOD("\x0A\x00\x85\xC0""interrupt0",    "\x03\x00\x91\x99""()V",                  nativeInterrupt0),
    NATIVE_METHOD("\x0D\x00\x5C\x6C""currentThread", "\x14\x00\xE0\x90""()Ljava/lang/Thread;", nativeCurrentThread),
    NATIVE_METHOD("\x06\x00\x4F\xE9""sleep0",        "\x04\x00\x48\x03""(J)V",                 nativeSleep0),
};

const FlintNativeClass THREAD_CLASS = NATIVE_CLASS(threadClassName, methods);
