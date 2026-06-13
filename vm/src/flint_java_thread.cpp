
#include "flint_java_thread.h"
#include "flint_fields_data.h"

FlintAPI::Thread::ThreadHandle JThread::getHandle(void) const {
    return (FlintAPI::Thread::ThreadHandle)getFieldByIndex(0)->getInt32();
}

void JThread::setHandle(FlintAPI::Thread::ThreadHandle handle) {
    getFieldByIndex(0)->setInt64((int64_t)handle);
}

JString *JThread::getName(void) const {
    return (JString *)getFieldByIndex(1)->getObj();
}

void JThread::setName(JString *name) {
    getFieldByIndex(1)->setObj((JObject *)name);
}

bool JThread::getInterrupt(void) const {
    return !!getFieldByIndex(2)->getInt32();
}

void JThread::setInterrupt(void) {
    getFieldByIndex(2)->setInt32(1);
}

void JThread::clearInterrupt(void) {
    getFieldByIndex(2)->setInt32(0);
}

JObject *JThread::getTask(void) const {
    return getFieldByIndex(3)->getObj();
}

int32_t JThread::getStackSize(void) const {
    return getFieldByIndex(4)->getInt32();
}

void JThread::setStackSize(int32_t size) {
    getFieldByIndex(4)->setInt32(size);
}

int32_t JThread::getPriority(void) const {
    return getFieldByIndex(5)->getInt32();
}

void JThread::setPriority(int32_t priority) {
    getFieldByIndex(5)->setInt32(priority);
}

bool JThread::isDaemon(void) const {
    return getFieldByIndex(6)->getInt32() != 0;
}

void JThread::setDaemon(bool on) {
    getFieldByIndex(6)->setInt32((int32_t)on);
}
