

#include "flint.h"
#include "flint_java_thread.h"
#include "flint_system_api.h"
#include "flint_const_name_base.h"
#include "flint_native_thread_class.h"
#include "flint_throw_support.h"

static const uint32_t runnableRunFieldName[] = {
    (uint32_t)"\x03\x00\x89\x58""run",              /* field name */
    (uint32_t)"\x03\x00\x91\x99""()V"               /* field type */
};

static FlintError nativeStart0(FlintExecution *exec) {
    Flint &flint = exec->flint;
    JThread *threadObj = (JThread *)exec->stackPopObject();
    JObject *task = threadObj->getTask();
    auto threadExecution = flint.newExecution(threadObj);
    RETURN_IF_ERR(threadExecution.err);
    if(task == 0)
        task = threadObj;
    threadExecution.value->stackPushObject(task);

    auto loader = flint.load(task->type.text);
    if(loader.err != ERR_OK)
        return checkAndThrowForFlintError(exec, loader.err, &task->type);
    auto method = loader.value->getMethodInfo(*(FlintConstNameAndType *)runnableRunFieldName);
    if(method.err != ERR_OK) {
        if(method.err == ERR_METHOD_NOT_FOUND)
            return throwNoSuchMethodError(exec, task->type.text, "run");
        return checkAndThrowForFlintError(exec, method.err, method.getErrorMsg(), method.getErrorMsgLength());
    }

    if(!threadExecution.value->run(method.value)) {
        flint.freeExecution(threadExecution.value);
        return throwException(exec, "Thread start failed");
    }
    return ERR_OK;
}

static FlintError nativeYield0(FlintExecution *exec) {
    FlintAPI::Thread::yield();
    return ERR_OK;
}

static FlintError nativeInterrupt0(FlintExecution *exec) {
    JThread *threadObj = (JThread *)exec->stackPopObject();
    // TODO
    return throwUnsupportedOperationException(exec, "interrupt0 is not implemented in VM");
}

static FlintError nativeCurrentThread(FlintExecution *exec) {
    auto thread = exec->getOnwerThread();
    RETURN_IF_ERR(thread.err);
    exec->stackPushObject(thread.value);
    return ERR_OK;
}

static FlintError nativeSleep0(FlintExecution *exec) {
    uint64_t startTime = FlintAPI::System::getNanoTime() / 1000000;
    int64_t millis = exec->stackPopInt64();
    while((int64_t)((FlintAPI::System::getNanoTime() / 1000000) - startTime) < (millis - 10)) {
        FlintAPI::Thread::sleep(10);
        if(exec->hasTerminateRequest())
            return ERR_TERMINATE_REQUEST;
    }
    int64_t remaining = millis - ((FlintAPI::System::getNanoTime() / 1000000) - startTime);
    if(remaining > 0)
        FlintAPI::Thread::sleep((uint32_t)remaining);
    return ERR_OK;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x06\x00\xDC\x1F""start0",        "\x03\x00\x91\x99""()V",                  nativeStart0),
    NATIVE_METHOD("\x06\x00\x5C\x41""yield0",        "\x03\x00\x91\x99""()V",                  nativeYield0),
    NATIVE_METHOD("\x0A\x00\x85\xC0""interrupt0",    "\x03\x00\x91\x99""()V",                  nativeInterrupt0),
    NATIVE_METHOD("\x0D\x00\x5C\x6C""currentThread", "\x14\x00\xE0\x90""()Ljava/lang/Thread;", nativeCurrentThread),
    NATIVE_METHOD("\x06\x00\x4F\xE9""sleep0",        "\x04\x00\x48\x03""(J)V",                 nativeSleep0),
};

const FlintNativeClass THREAD_CLASS = NATIVE_CLASS(threadClassName, methods);
