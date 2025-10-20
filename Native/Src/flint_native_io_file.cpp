
#include <string.h>
#include "flint.h"
#include "flint_common.h"
#include "flint_native_io_file.h"

static const char *resolvePath(FNIEnv *env, jobject file, char *buff, uint32_t buffSize) {
    jstring path = (jstring)file->getFieldByIndex(0)->getObj();
    const char *ptxt = path->getAscii();
    uint32_t len = path->getLength();
    if(resolvePath(ptxt, len, buff, buffSize) == 0) {
        jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
        env->throwNew(excpCls, "Class name cannot exceed %d characters", buffSize - 1);
        return NULL;
    }
    return buff;
}

static FlintAPI::IO::DirHandle openDir(FNIEnv *env, jobject file) {
    char buff[FILE_NAME_BUFF_SIZE];
    if(resolvePath(env, file, buff, sizeof(buff)) == NULL) return NULL;
    return FlintAPI::IO::opendir(buff);
}

jchar nativeIoFileGetSeparatorChar0(FNIEnv *env) {
    (void)env;
    return getPathSeparatorChar();
}

jchar nativeIoFileGetPathSeparatorChar0(FNIEnv *env) {
    (void)env;
    return ';';
}

jbool nativeIoFileIsAbsolute(FNIEnv *env, jobject file) {
    (void)env;
    jstring path = (jstring)file->getFieldByIndex(0)->getObj();
    const char *txt = path->getAscii();
    uint32_t len = path->getLength();
    return isAbsolutePath(txt, len) ? true : false;
}

jstring nativeIoFileGetAbsolutePath(FNIEnv *env, jobject file) {
    jstring path = (jstring)file->getFieldByIndex(0)->getObj();
    const char *txt = path->getAscii();
    uint32_t len = path->getLength();
    if(isAbsolutePath(txt, len))
        return path;
    else {
        char buff[FILE_NAME_BUFF_SIZE];
        if(resolvePath(env, file, buff, sizeof(buff)) == NULL) return NULL;
        return Flint::newString(env->exec, buff);
    }
}

jbool nativeIoFileExists(FNIEnv *env, jobject file) {
    (void)env;
    char buff[FILE_NAME_BUFF_SIZE];
    if(resolvePath(env, file, buff, sizeof(buff)) == NULL) return false;
    return FlintAPI::IO::finfo(buff, NULL) == FlintAPI::IO::FILE_RESULT_OK;
}

jbool nativeIoFileCanWrite(FNIEnv *env, jobject file) {
    FlintAPI::IO::FileInfo fileInfo;
    char buff[FILE_NAME_BUFF_SIZE];
    if(resolvePath(env, file, buff, sizeof(buff)) == NULL) return false;
    if(FlintAPI::IO::finfo(buff, &fileInfo) != FlintAPI::IO::FILE_RESULT_OK)
        return false;
    return (fileInfo.system | fileInfo.readOnly) ? false : true;
}

jbool nativeIoFileCanRead(FNIEnv *env, jobject file) {
    FlintAPI::IO::FileInfo fileInfo;
    char buff[FILE_NAME_BUFF_SIZE];
    if(resolvePath(env, file, buff, sizeof(buff)) == NULL) return false;
    if(FlintAPI::IO::finfo(buff, &fileInfo) != FlintAPI::IO::FILE_RESULT_OK)
        return false;
    return fileInfo.system ? false : true;
}

jbool nativeIoFileIsFile(FNIEnv *env, jobject file) {
    FlintAPI::IO::FileInfo fileInfo;
    char buff[FILE_NAME_BUFF_SIZE];
    if(resolvePath(env, file, buff, sizeof(buff)) == NULL) return false;
    if(FlintAPI::IO::finfo(buff, &fileInfo) != FlintAPI::IO::FILE_RESULT_OK)
        return false;
    return fileInfo.directory ? false : true;
}

jbool nativeIoFileIsDirectory(FNIEnv *env, jobject file) {
    return !nativeIoFileIsFile(env, file);
}

jbool nativeIoFileIsHidden(FNIEnv *env, jobject file) {
    FlintAPI::IO::FileInfo fileInfo;
    char buff[FILE_NAME_BUFF_SIZE];
    if(resolvePath(env, file, buff, sizeof(buff)) == NULL) return false;
    if(FlintAPI::IO::finfo(buff, &fileInfo) != FlintAPI::IO::FILE_RESULT_OK)
        return false;
    return fileInfo.hidden ? true : false;
}

jlong nativeIoFileLastModified(FNIEnv *env, jobject file) {
    FlintAPI::IO::FileInfo fileInfo;
    char buff[FILE_NAME_BUFF_SIZE];
    if(resolvePath(env, file, buff, sizeof(buff)) == NULL) return false;
    if(FlintAPI::IO::finfo(buff, &fileInfo) != FlintAPI::IO::FILE_RESULT_OK)
        return 0;
    return fileInfo.time;
}

jlong nativeIoFileLength(FNIEnv *env, jobject file) {
    FlintAPI::IO::FileInfo fileInfo;
    char buff[FILE_NAME_BUFF_SIZE];
    if(resolvePath(env, file, buff, sizeof(buff)) == NULL) return false;
    if(FlintAPI::IO::finfo(buff, &fileInfo) != FlintAPI::IO::FILE_RESULT_OK)
        return 0;
    return fileInfo.size;
}

jbool nativeIoFileMkdir(FNIEnv *env, jobject file) {
    char buff[FILE_NAME_BUFF_SIZE];
    if(resolvePath(env, file, buff, sizeof(buff)) == NULL) return false;
    return FlintAPI::IO::mkdir(buff) == FlintAPI::IO::FILE_RESULT_OK;
}

jbool nativeIoFileRenameTo(FNIEnv *env, jobject file, jobject dest) {
    if(dest == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return false;
    }
    char buff1[FILE_NAME_BUFF_SIZE];
    char buff2[FILE_NAME_BUFF_SIZE];
    if(resolvePath(env, file, buff1, sizeof(buff1)) == NULL) return false;
    if(resolvePath(env, dest, buff2, sizeof(buff2)) == NULL) return false;
    return FlintAPI::IO::frename(buff1, buff2) == FlintAPI::IO::FILE_RESULT_OK;
}

jbool nativeIoFileDelete0(FNIEnv *env, jobject file) {
    char buff[FILE_NAME_BUFF_SIZE];
    if(resolvePath(env, file, buff, sizeof(buff)) == NULL) return false;
    return FlintAPI::IO::fremove(buff) == FlintAPI::IO::FILE_RESULT_OK;
}

jbool nativeIoFileRmdir0(FNIEnv *env, jobject file) {
    return nativeIoFileDelete0(env, file);
}

jobjectArray nativeIoFileList(FNIEnv *env, jobject file) {
    auto handle = openDir(env, file);
    if(handle == NULL) return NULL;

    jclass strArrCls = Flint::findClassOfArray(env->exec, "java/lang/String", 1);
    jobjectArray arr = (jobjectArray)Flint::newArray(env->exec, strArrCls, 16);
    if(strArrCls == NULL || arr == NULL) return NULL;
    arr->clearArray();
    uint32_t count = 0;
    FlintAPI::IO::FileInfo fileInfo;
    while(1) {
        if(FlintAPI::IO::readdir(handle, &fileInfo) != FlintAPI::IO::FILE_RESULT_OK) {
            env->throwNew(env->findClass("java/io/IOException"), "Error while reading directory");
            break;
        }
        if(fileInfo.name[0] == 0) {
            FlintAPI::IO::closedir(handle);
            if(count != arr->getLength()) {
                jobjectArray newArr = (jobjectArray)Flint::newArray(env->exec, strArrCls, count);
                if(newArr == NULL) break;
                newArr->clearArray();
                memcpy(newArr->getData(), arr->getData(), newArr->getLength() * sizeof(jobject));
                env->freeObject(arr);
                arr = newArr;
            }
            return arr;
        }
        if(count >= arr->getLength()) {
            jobjectArray newArr = (jobjectArray)Flint::newArray(env->exec, strArrCls, arr->getLength() + 16);
            if(newArr == NULL) break;
            newArr->clearArray();
            memcpy(newArr->getData(), arr->getData(), arr->getLength() * sizeof(jobject));
            env->freeObject(arr);
            arr = newArr;
        }
        arr->getData()[count] = Flint::newString(env->exec, fileInfo.name);
        if(arr->getData()[count] == NULL) break;
        count++;
    }
    for(uint32_t i = 0; i < count; i++)
        env->freeObject(arr->getData()[i]);
    env->freeObject(arr);
    FlintAPI::IO::closedir(handle);
    return NULL;
}

jstring nativeIoFileGetCanonicalPath(FNIEnv *env, jobject file, jstring path) {
    char buff[FILE_NAME_BUFF_SIZE];
    if(resolvePath(env, file, buff, sizeof(buff)) == NULL) return NULL;
    char sep = getPathSeparatorChar();
    char *src = buff;
    char *dst = buff;
    while(*src) {
        if(*src == sep && src[1] == sep) {
            src++;
            continue;
        }
        if(*src == '.' && (src[1] == sep || src[1] == 0)) {
            src += (src[1] == sep) ? 2 : 1;
            continue;
        }
        if(*src == '.' && src[1] == '.' && (src[2] == sep || src[2] == 0)) {
            src += (src[2] == sep) ? 3 : 2;
            if(dst > buff) {
                dst--;
                while(dst > buff && *(dst - 1) != sep) dst--;
            }
            continue;
        }
        *dst++ = *src++;
    }
    *dst = 0;
    return Flint::newString(env->exec, buff);
}

jbool nativeIoFileCreateNewFile(FNIEnv *env, jobject file) {
    char buff[FILE_NAME_BUFF_SIZE];
    if(resolvePath(env, file, buff, sizeof(buff)) == NULL) return false;
    auto f = FlintAPI::IO::fopen(buff, FlintAPI::IO::FILE_MODE_CREATE_NEW);
    if(f == NULL) return false;
    FlintAPI::IO::fclose(f);
    return true;
}
