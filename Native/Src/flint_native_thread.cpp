

#include "flint.h"
#include "flint_java_thread.h"
#include "flint_system_api.h"
#include "flint_native_thread.h"

jvoid nativeStart0(FNIEnv *env, jthread thread) {
    static constexpr ConstNameAndType runName("run", "()V");
    jobject task = thread->getTask();
    FExec *exec = Flint::newExecution(env->exec, thread);
    if(exec == NULL) return;
    if(task == 0) task = thread;

    ClassLoader *loader = task->type->getClassLoader();
    jmethodId method = loader->getMethodInfo(env->exec, (ConstNameAndType *)&runName);
    if(method == NULL) {
        env->throwNew(env->findClass("java/lang/NoSuchMethodError"), "%s.%s", task->getTypeName(), "run()");
        return;
    }

    if(!exec->run(method, 1, task)) {
        Flint::freeExecution(exec);
        env->throwNew(env->findClass("java/lang/Exception"), "Thread start failed");
    }
}

jvoid nativeYield0(FNIEnv *env) {
    (void)env;
    FlintAPI::Thread::yield();
}

jvoid nativeInterrupt0(FNIEnv *env, jthread thread) {
    // TODO
    (void)env;
    (void)thread;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "interrupt0 is not implemented in VM");
}

jthread nativeCurrentThread(FNIEnv *env) {
    (void)env;
    return env->exec->getOnwerThread();
}

jvoid nativeSleep0(FNIEnv *env, jlong millis) {
    uint64_t startTime = FlintAPI::System::getNanoTime() / 1000000;
    while((int64_t)((FlintAPI::System::getNanoTime() / 1000000) - startTime) < (millis - 10)) {
        FlintAPI::Thread::sleep(10);
        if(env->exec->hasTerminateRequest()) return;
    }
    int64_t remaining = millis - ((FlintAPI::System::getNanoTime() / 1000000) - startTime);
    if(remaining > 0) FlintAPI::Thread::sleep((uint32_t)remaining);
}
