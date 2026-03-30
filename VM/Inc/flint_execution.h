
#ifndef __FLINT_EXECUTION_H
#define __FLINT_EXECUTION_H

#include <cstdarg>
#include "flint_common.h"
#include "flint_debugger.h"
#include "flint_list.h"
#include "flint_const_pool.h"
#include "flint_method_info.h"
#include "flint_java_thread.h"
#include "flint_java_throwable.h"
#include "flint_native_interface.h"
#include "flint_default_conf.h"

class FExec : public ListNode, public FNIEnv {
private:
    class Flint * const flint;
    const void ** volatile opcodes;
    const uint32_t stackLength;
    MethodInfo *method;
    const uint8_t *code;
    uint32_t pc;
    uint32_t lr;
    int32_t sp;
    int32_t startSp;
    int32_t peakSp;
    int32_t *locals;
    JThread *onwerThread;
    JThrowable *excp;
    int32_t stack[];

    void stackPushInt32(int32_t value);
    void stackPushInt64(int64_t value);
    void stackPushFloat(float value);
    void stackPushDouble(double value);
    void stackPushObject(JObject *obj);

    int32_t stackPopInt32(void);
    int64_t stackPopInt64(void);
    float stackPopFloat(void);
    double stackPopDouble(void);
    JObject *stackPopObject(void);

    void stackPushArgs(uint32_t argc, va_list args);
    void stackSaveContext(void);
    void stackRestoreContext(void);
    void restoreContext(void);
    void initNewContext(MethodInfo *methodInfo, uint16_t argc);
    void initExitPoint(MethodInfo *methodInfo);

    bool lockClass(ClassLoader *cls);
    void unlockClass(ClassLoader *cls);
    bool lockObject(JObject *obj);
    void unlockObject(JObject *obj);

    bool checkInvokeArgs(JObject *obj, MethodInfo *methodInfo);
    uint64_t callMethod(jmethodId mtid, uint8_t argc);
    uint64_t vCallMethod(jmethodId mtid, va_list args);
    void invokeNativeMethod(MethodInfo *methodInfo, uint8_t argc);
    void invoke(MethodInfo *methodInfo, uint8_t argc);
    void invokeStatic(ConstMethod *constMethod);
    void invokeSpecial(ConstMethod *constMethod);
    void invokeVirtual(ConstMethod *constMethod);
    void invokeInterface(ConstInterfaceMethod *interfaceMethod, uint8_t argc);
    void invokeStaticCtor(ClassLoader *cls);

    void exec(bool initOpcodeLabels);
    void stopRequest(void);
    void terminateRequest(void);
    int32_t getStackTrace(StackFrame *stackTrace, int32_t traceSp) const;
    bool getStackTrace(uint32_t index, StackFrame *stackTrace, bool *isEndStack) const;
    bool readLocal(uint32_t stackIndex, uint32_t localIndex, uint32_t *value, bool *isObject) const;
    bool readLocal(uint32_t stackIndex, uint32_t localIndex, uint64_t *value) const;

    static void runTask(FExec *execution);
public:
    Flint *getFlint(void) const;

    bool run(MethodInfo *method, uint32_t argc = 0, ...);

    JClass *getCallerClass(void);

    void throwNew(JClass *cls, const char *msg = NULL, ...);
    void vThrowNew(JClass *cls, const char *msg, va_list args);
    bool hasException(void) const;

    JThread *getOnwerThread(void);
public:
    jclass findClass(const char *name, uint16_t length = 0xFFFF) __attribute__((used));

    jbool isInstanceof(jobject obj, jclass type) __attribute__((used));
    jbool isAssignableFrom(jclass fromType, jclass toType) __attribute__((used));

    jobject newObject(jclass type) __attribute__((used));
    jobject newObject(jclass type, jmethodId ctor, ...) __attribute__((used));
    jstring newString(const char *format, ...) __attribute__((used));
    jboolArray newBoolArray(uint32_t count) __attribute__((used));
    jbyteArray newByteArray(uint32_t count) __attribute__((used));
    jcharArray newCharArray(uint32_t count) __attribute__((used));
    jshortArray newShortArray(uint32_t count) __attribute__((used));
    jintArray newIntArray(uint32_t count) __attribute__((used));
    jlongArray newLongArray(uint32_t count) __attribute__((used));
    jfloatArray newFloatArray(uint32_t count) __attribute__((used));
    jdoubleArray newDoubleArray(uint32_t count) __attribute__((used));
    jobjectArray newObjectArray(jclass type, uint32_t count) __attribute__((used));

    jfieldId getFieldId(jobject obj, const char *name) __attribute__((used));
    jbool getBoolField(jfieldId fid) __attribute__((used));
    jbyte getByteField(jfieldId fid) __attribute__((used));
    jchar getCharField(jfieldId fid) __attribute__((used));
    jshort getShortField(jfieldId fid) __attribute__((used));
    jint getIntField(jfieldId fid) __attribute__((used));
    jfloat getFloatField(jfieldId fid) __attribute__((used));
    jlong getLongField(jfieldId fid) __attribute__((used));
    jdouble getDoubleField(jfieldId fid) __attribute__((used));
    jobject getObjField(jfieldId fid) __attribute__((used));

    jvoid setBoolField(jfieldId fid, jbool val) __attribute__((used));
    jvoid setByteField(jfieldId fid, jbyte val) __attribute__((used));
    jvoid setCharField(jfieldId fid, jchar val) __attribute__((used));
    jvoid setShortField(jfieldId fid, jshort val) __attribute__((used));
    jvoid setIntField(jfieldId fid, jint val) __attribute__((used));
    jvoid setFloatField(jfieldId fid, jfloat val) __attribute__((used));
    jvoid setLongField(jfieldId fid, jlong val) __attribute__((used));
    jvoid setDoubleField(jfieldId fid, jdouble val) __attribute__((used));
    jvoid setObjField(jfieldId fid, jobject obj) __attribute__((used));

    jmethodId getMethodId(jclass cls, const char *name, const char *sig) __attribute__((used));
    jmethodId getConstructorId(jclass cls, const char *sig) __attribute__((used));

    jvoid callVoidMethod(jmethodId mtid, ...) __attribute__((used));
    jbool callBoolMethod(jmethodId mtid, ...) __attribute__((used));
    jbyte callByteMethod(jmethodId mtid, ...) __attribute__((used));
    jchar callCharMethod(jmethodId mtid, ...) __attribute__((used));
    jshort callShortMethod(jmethodId mtid, ...) __attribute__((used));
    jint callIntMethod(jmethodId mtid, ...) __attribute__((used));
    jlong callLongMethod(jmethodId mtid, ...) __attribute__((used));
    jfloat callFloatMethod(jmethodId mtid, ...) __attribute__((used));
    jdouble callDoubleMethod(jmethodId mtid, ...) __attribute__((used));
    jobject callObjectMethod(jmethodId mtid, ...) __attribute__((used));

    jbool hasTerminateRequest(void) __attribute__((used));
    jvoid freeObject(jobject obj) __attribute__((used));
private:
    FExec(Flint *flint, JThread *onwer, uint32_t stackSize);
    FExec(const FExec &) = delete;
    void operator=(const FExec &) = delete;

    friend class Flint;
    friend class FDbg;
    friend class FNIEnv;

    friend jobject NativeMethod_Invoke0(FNIEnv *, jobject, jobject, jobjectArray);
    friend jobject NativeConstructor_NewInstance0(FNIEnv *, jobject, jobjectArray);
};

#endif /* __FLINT_EXECUTION_H */
