
#include "flint.h"
#include "flint_java_thread.h"
#include "flint_system_api.h"
#include "flint_native_thread.h"

jvoid NativeThread_Start0(FNIEnv *env, jthread thread) {
    static constexpr ConstNameAndType runName("run", "()V");
    Flint *flint = ((FExec *)env)->getFlint();
    jobject task = thread->getTask();
    FExec *exec = flint->newExecution(((FExec *)env), thread);
    if(exec == NULL) return;
    if(task == 0) task = thread;

    ClassLoader *loader = task->type->getClassLoader();
    jmethodId method = loader->getMethodInfo(((FExec *)env), (ConstNameAndType *)&runName);
    if(method == NULL) {
        env->throwNew(env->findClass("java/lang/NoSuchMethodError"), "%s.%s", task->getTypeName(), "run()");
        return;
    }

    if(!exec->run(method, 1, task)) {
        flint->freeExecution(exec);
        env->throwNew(env->findClass("java/lang/Exception"), "Thread start failed");
    }
}

jvoid NativeThread_Yield0(FNIEnv *env) {
    (void)env;
    FlintAPI::Thread::yield();
}

jvoid NativeThread_SetPriority0(FNIEnv *env, jthread thread, jint newPriority) {
    // TODO
    (void)env;
    (void)thread;
    (void)newPriority;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"));
}

jvoid NativeThread_Interrupt0(FNIEnv *env, jthread thread) {
    (void)env;
    FlintAPI::Thread::ThreadHandle handle = thread->getHandle();
    FlintAPI::Thread::notify(handle, (uint32_t)handle);
}

jthread NativeThread_CurrentThread(FNIEnv *env) {
    return ((FExec *)env)->getOwnerThread();
}

jvoid NativeThread_Sleep0(FNIEnv *env, jlong millis) {
    int64_t startTime = FlintAPI::System::getTimeMillis();
    jthread ownerThread = ((FExec *)env)->getOwnerThread();
    while((int64_t)(FlintAPI::System::getTimeMillis() - startTime) < millis) {
        int64_t remaining = millis - (FlintAPI::System::getTimeMillis() - startTime);
        if(remaining > 1000) remaining = 1000;
        else if(remaining < 0) return;
        if(env->hasTerminateRequest()) return;
        if(ownerThread->getInterrupt()) {
            env->throwNew(env->findClass("java/lang/InterruptedException"), "sleep interrupted");
            ownerThread->clearInterrupt();
            return;
        }
        FlintAPI::Thread::wait((uint32_t)remaining);
    }
}
