
#ifndef __FLINT_H
#define __FLINT_H

#include <cstdarg>
#include "flint_std.h"
#include "flint_list.h"
#include "flint_mutex.h"
#include "flint_execution.h"
#include "flint_dictionary.h"
#include "flint_java_object.h"
#include "flint_java_class.h"
#include "flint_java_string.h"
#include "flint_java_throwable.h"
#include "flint_java_method_handle.h"
#include "flint_class_loader.h"
#include "flint_utf8_dict_node.h"
#include "flint_java_class_dict_node.h"
#include "flint_java_string_dict_node.h"

class Flint {
private:
    static FMutex flintLock;
    static FDbg *dbg;
    static FDict<ClassLoader> loaders;
    static FDict<JClassDictNode> classes;
    static FDict<Utf8DictNode> utf8s;
    static FDict<JStringDictNode> constStr;
    static FList<FExec> execs;
    static FList<JObject> objs;
    static FList<JObject> globalObjs;

    static JClass *classOfClass;

    static uint32_t heapCount;
    static uint32_t objectCountToGc;
    static void *heapStart;
    static void *headEnd;
private:
    static void updateHeapRegion(void *p);
    static void resetHeapRegion(void);
    static bool isHeapPointer(void *p);
public:
    static void *malloc(FExec *ctx, uint32_t size);
    static void *realloc(FExec *ctx, void *p, uint32_t size);
    static void free(void *p);

    static void lock(void);
    static void unlock(void);

    static FDbg *getDebugger(void);
    static void setDebugger(FDbg *dbg);
private:
    static void print(const char *buff, uint32_t length, uint8_t coder);
public:
    static void print(int64_t num);
    static void print(const char *ascii);
    static void print(JString *str);
    static void println(int64_t num);
    static void println(const char *ascii);
    static void println(JString *str);

    static const char *getUtf8(FExec *ctx, const char *utf8, uint16_t length = 0xFFFF);
    static ClassLoader *findLoader(FExec *ctx, const char *clsName, uint16_t length = 0xFFFF);
    static JClass *findClass(FExec *ctx, const char *clsName, uint16_t length = 0xFFFF, bool verify = false);
    static JClass *findClassOfArray(FExec *ctx, const char *clsName, uint8_t dimensions);
    static JClass *getPrimitiveClass(FExec *ctx, const char *name, uint16_t length = 0xFFFF);
    static JClass *getClassOfClass(FExec *ctx);
    static MethodInfo *findMethod(FExec *ctx, JClass *cls, ConstNameAndType *nameAndType);
    static MethodInfo *findMethod(FExec *ctx, JClass *cls, const char *name, const char *desc);
    static MethodInfo *findMethod(FExec *ctx, JClass *cls, const char *name, uint16_t nameLen, const char *desc, uint16_t descLen);
    static JString *getConstString(FExec *ctx, const char *utf8);
    static JString *getConstString(FExec *ctx, JString *str);

    static bool isInstanceof(FExec *ctx, JObject *obj, JClass *type);
    static bool isAssignableFrom(FExec *ctx, JClass *fromType, JClass *toType);

    static FExec *newExecution(FExec *ctx, JThread *onwer = NULL, uint32_t stackSize = DEFAULT_STACK_SIZE);
    static void freeExecution(FExec *exec);

    static JObject *newObject(FExec *ctx, JClass *type);
    static JObject *newArray(FExec *ctx, JClass *type, uint32_t count);
    static JObject *newMultiArray(FExec *ctx, JClass *type, int32_t *counts, uint8_t depth);
    static JString *newString(FExec *ctx, const char *utf8);
    static JString *newAscii(FExec *ctx, const char *format, ...);
    static JString *newAscii(FExec *ctx, const char *format, va_list args);

    static JObject *newMethodType(FExec *ctx, const char *desc);
    static JMethodHandle *newMethodHandle(FExec *ctx, ConstMethod *constMethod, RefKind refKind);

    static void makeToGlobal(JObject *obj);
    static void clearProtLv2(JObject *obj);
    static bool isObject(void *p);
    static void gc(void);

    static bool runToMain(const char *cls);

    static bool isRunning(void);
    static void stopRequest(void);
    static void terminateRequest(void);
    static void terminate(void);
    static void freeObject(JObject *obj);
    static void clearAllStaticFields(void);
    static void freeAllExecution(void);
    static void freeAll(void);
    static void reset(void);
private:
    static void freeAllObject(void);
    static void freeAllClassLoader(void);
    static void freeAllConstUtf8(void);
    static void clearMarkRecursion(JObject *obj);
    static void markObjectRecursion(JObject *obj);
    static void clearProtLv2Recursion(JObject *obj);
private:
    static const char *getArrayClassName(FExec *ctx, const char *clsName, uint8_t dimensions);
    static JClass *newClass(FExec *ctx, const char *clsName, uint16_t length = 0xFFFF, uint8_t flag = 0x00);
    static JClass *newClassOfArray(FExec *ctx, const char *clsName, uint8_t dimensions);
    static JClass *newClassOfClass(FExec *ctx);
    static JMethodHandle *newMethodHandle(FExec *ctx);
private:
    Flint(void) = delete;
    Flint(const Flint &) = delete;
    void operator=(const Flint &) = delete;
};

#endif /* __FLINT_H */
