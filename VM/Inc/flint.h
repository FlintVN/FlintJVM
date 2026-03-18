
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
#include "flint_class_loader.h"
#include "flint_utf8_dict_node.h"
#include "flint_java_class_dict_node.h"
#include "flint_java_string_dict_node.h"

class Flint {
private:
    FMutex flintLock;
    FDbg *dbg;
    const char *cwd;
    const char *program;
    FDict<ClassLoader> loaders;
    FDict<JClassDictNode> classes;
    FDict<Utf8DictNode> utf8s;
    FDict<JStringDictNode> constStr;
    FList<FExec> execs;
    FList<JObject> objs;
    FList<JObject> globalObjs;

    JClass *classOfClass;

    uint32_t heapCount;
    uint32_t objectCountToGc;
    void *heapStart;
    void *headEnd;
private:
    void updateHeapRegion(void *p);
    void resetHeapRegion(void);
    bool isHeapPointer(void *p);
public:
    Flint(void);
    void *malloc(FExec *ctx, uint32_t size);
    void *realloc(FExec *ctx, void *p, uint32_t size);
    void free(void *p);

    void lock(void);
    void unlock(void);

    FDbg *getDebugger(void);
    void setDebugger(FDbg *dbg);
public:
    void consoleWrite(uint8_t *utf8, uint32_t length);
    void print(int64_t num);
    void print(const char *ascii);
    void print(JString *str);
    void println(void);
    void println(int64_t num);
    void println(const char *ascii);
    void println(JString *str);

     const char *getCwd(void);
     void setCwd(const char *path);
     const char *getClassPath(uint32_t index);

    bool setProgram(const char *jarPath, uint16_t length = 0xFFFF);
    const char *getProgram(void);

    static char getPathSeparator(void);
    static uint16_t isAbsolutePath(const char *path, uint16_t length);
    int16_t resolvePath(const char *path, uint16_t length, char *buff, uint16_t buffSize);

    const char *getUtf8(FExec *ctx, const char *utf8, uint16_t length = 0xFFFF);
    ClassLoader *findLoader(FExec *ctx, const char *clsName, uint16_t length = 0xFFFF);
    JClass *findClass(FExec *ctx, const char *clsName, uint16_t length = 0xFFFF, bool verify = false);
    JClass *findClassOfArray(FExec *ctx, const char *clsName, uint8_t dimensions);
    JClass *getPrimitiveClass(FExec *ctx, const char *name, uint16_t length = 0xFFFF);
    JClass *getClassOfClass(FExec *ctx);
    MethodInfo *findMethod(FExec *ctx, JClass *cls, ConstNameAndType *nameAndType);
    JString *getConstString(FExec *ctx, const char *utf8);
    JString *getConstString(FExec *ctx, JString *str);

    bool isInstanceof(FExec *ctx, JObject *obj, JClass *type);
    bool isAssignableFrom(FExec *ctx, JClass *fromType, JClass *toType);

    FExec *newExecution(FExec *ctx, JThread *onwer = NULL, uint32_t stackSize = DEFAULT_STACK_SIZE);
    void freeExecution(FExec *exec);

    JObject *newObject(FExec *ctx, JClass *type);
    JObject *newArray(FExec *ctx, JClass *type, uint32_t count);
    JObject *newMultiArray(FExec *ctx, JClass *type, int32_t *counts, uint8_t depth);
    JString *newString(FExec *ctx, const char *utf8);
    JString *newAscii(FExec *ctx, const char *format, ...);
    JString *newAscii(FExec *ctx, const char *format, va_list args);

    void makeToGlobal(JObject *obj);
    void clearProtLv2(JObject *obj);
    bool isObject(void *p);
    void gc(void);

    bool start(void);

    bool isRunning(void);
    void stopRequest(void);
    void terminateRequest(void);
    void terminate(void);
    void freeObject(JObject *obj);
    void clearAllStaticFields(void);
    void freeAllExecution(void);
    void freeAll(void);
    void reset(void);
private:
    void freeAllObject(void);
    void freeAllClassLoader(void);
    void freeAllConstUtf8(void);
    void clearMarkRecursion(JObject *obj);
    void markObjectRecursion(JObject *obj);
    void clearProtLv2Recursion(JObject *obj);
private:
    const char *getArrayClassName(FExec *ctx, const char *clsName, uint8_t dimensions);
    JClass *newClass(FExec *ctx, const char *clsName, uint16_t length = 0xFFFF, uint8_t flag = 0x00);
    JClass *newClassOfArray(FExec *ctx, const char *clsName, uint8_t dimensions);
    JClass *newClassOfClass(FExec *ctx);
private:
    Flint(const Flint &) = delete;
    void operator=(const Flint &) = delete;
};

#endif /* __FLINT_H */
