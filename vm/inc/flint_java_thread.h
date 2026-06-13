
#ifndef __FLINT_JAVA_THREAD_H
#define __FLINT_JAVA_THREAD_H

#include "flint_system_api.h"
#include "flint_java_object.h"

class JThread : public JObject {
public:
    FlintAPI::Thread::ThreadHandle getHandle(void) const;
    void setHandle(FlintAPI::Thread::ThreadHandle handle);

    JString *getName(void) const;
    void setName(JString *name);

    bool getInterrupt(void) const;
    void setInterrupt(void);
    void clearInterrupt(void);

    JObject *getTask(void) const;

    int32_t getStackSize(void) const;
    void setStackSize(int32_t size);

    int32_t getPriority(void) const;
    void setPriority(int32_t priority);

    bool isDaemon(void) const;
    void setDaemon(bool on);
protected:
    JThread(void) = delete;
    JThread(const JThread &) = delete;
    void operator=(const JThread &) = delete;
};

#endif /* __FLINT_JAVA_THREAD_H */
