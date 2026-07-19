
#include "flint.h"
#include "flint_native_common.h"
#include "flint_native_file_output_stream.h"
#include "flint_native_random_access_file.h"

#define J_RDONLY        1
#define J_RDWR          2
#define J_SYNC          4
#define J_DSYNC         8

static jint getFd(FNIEnv *env, jobject obj) {
    jobject fdObj = obj->getFieldByIndex(0)->getObj();
    jint fd = fdObj->getFieldByIndex(0)->getInt32();
    if(fd == -1)
        env->throwNew(env->findClass("java/io/IOException"), "Stream closed");
    return fd;
}

static jint getMode(jobject obj) {
    return obj->getFieldByIndex(2)->getInt32();
}

static bool readBytes(FNIEnv *env, jobject obj, void *buf, uint32_t size, uint32_t *br) {
    jint fd = getFd(env, obj);
    if(fd == -1) return false;
    void *handle = ((Hook *)fd)->getHandle();
    auto result = FlintAPI::IO::fread((FlintAPI::IO::FileHandle)handle, buf, size, br);
    if(result != FlintAPI::IO::FILE_RESULT_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "Error while reading");
        return false;
    }
    return true;
}

static bool writeBytes(FNIEnv *env, jobject obj, void *buf, uint32_t size, uint32_t *bw) {
    jint fd = getFd(env, obj);
    if(fd == -1) return false;
    void *handle = ((Hook *)fd)->getHandle();
    auto result = FlintAPI::IO::fwrite((FlintAPI::IO::FileHandle)handle, buf, size, bw);
    if(result != FlintAPI::IO::FILE_RESULT_OK || *bw != size) {
        env->throwNew(env->findClass("java/io/IOException"), "Error while writing");
        return false;
    }
    uint32_t mode = getMode(obj);
    if(mode & J_SYNC) {
        result = FlintAPI::IO::fsync((FlintAPI::IO::FileHandle)handle);
        if(result != FlintAPI::IO::FILE_RESULT_OK) {
            env->throwNew(env->findClass("java/io/IOException"), "Error while writing: fsync error");
            return false;
        }
    }
    return true;
}

jvoid NativeRandomAccessFile_Open(FNIEnv *env, jobject obj, jstring name, jint mode) {
    char buff[FILE_NAME_BUFF_SIZE];
    jobject fdObj = obj->getFieldByIndex(0)->getObj();
    jint fd = fdObj->getFieldByIndex(0)->getInt32();
    Flint *flint = ((FExec *)env)->getFlint();
    if(flint->resolvePath(name->getAscii(), name->getLength(), buff, sizeof(buff)) == -1) return;
    flint->lock();
    if(fd != -1)
        env->throwNew(env->findClass("java/io/IOException"), "Stream has been opened");
    else {
        FlintAPI::IO::FileMode fileMode = FlintAPI::IO::FILE_MODE_READ;
        if(mode & J_RDWR)
            fileMode = (FlintAPI::IO::FileMode)(fileMode | FlintAPI::IO::FILE_MODE_WRITE | FlintAPI::IO::FILE_MODE_OPEN_ALWAYS);
        FExec *exec = (FExec *)env;
        auto handle = FlintAPI::IO::fopen(buff, fileMode);
        Hook *hook = (handle == NULL) ? NULL : exec->getFlint()->addShutdownHook(exec, handle, (void (*)(void*))FlintAPI::IO::fclose);
        if(hook == NULL) {
            if(handle != NULL) FlintAPI::IO::fclose(handle);
            env->throwNew(env->findClass("java/io/FileNotFoundException"), "Stream opening failed");
        }
        else
            fdObj->getFieldByIndex(0)->setInt32((int32_t)hook);
    }
    flint->unlock();
}

jint NativeRandomAccessFile_Read(FNIEnv *env, jobject obj) {
    uint8_t data;
    uint32_t br = 0;
    readBytes(env, obj, &data, 1, &br);
    return (br == 1) ? data : -1;
}

jint NativeRandomAccessFile_ReadBytes(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len) {
    if(!CheckArrayIndexSize(env, b, off, len)) return 0;
    uint32_t br = 0;
    readBytes(env, obj, &b->getData()[off], len, &br);
    return br;
}

jvoid NativeRandomAccessFile_Write(FNIEnv *env, jobject obj, jint b) {
    uint8_t data = b & 0xFF;
    uint32_t bw = 0;
    writeBytes(env, obj, &data, 1, &bw);
}

jvoid NativeRandomAccessFile_WriteBytes(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len) {
    if(!CheckArrayIndexSize(env, b, off, len)) return;
    uint32_t bw = 0;
    writeBytes(env, obj, &b->getData()[off], len, &bw);
}

jlong NativeRandomAccessFile_GetFilePointer(FNIEnv *env, jobject obj) {
    jint fd = getFd(env, obj);
    if(fd == -1) return 0;
    void *handle = ((Hook *)fd)->getHandle();
    return FlintAPI::IO::ftell((FlintAPI::IO::FileHandle)handle);
}

jvoid NativeRandomAccessFile_Seek(FNIEnv *env, jobject obj, jlong pos) {
    if(pos < 0)
        return env->throwNew(env->findClass("java/io/IOException"), "Negative seek offset");
    else if(pos > 0xFFFFFFFFUL)
        return env->throwNew(env->findClass("java/io/IOException"), "The seek offset is too large");
    jint fd = getFd(env, obj);
    if(fd == -1) return;
    void *handle = ((Hook *)fd)->getHandle();
    auto result = FlintAPI::IO::fseek((FlintAPI::IO::FileHandle)handle, (uint32_t)pos);
    if(result != FlintAPI::IO::FILE_RESULT_OK)
        env->throwNew(env->findClass("java/io/IOException"), "seek error");
}

jlong NativeRandomAccessFile_Length(FNIEnv *env, jobject obj) {
    jint fd = getFd(env, obj);
    if(fd == -1) return 0;
    void *handle = ((Hook *)fd)->getHandle();
    return FlintAPI::IO::fsize((FlintAPI::IO::FileHandle)handle);
}

jvoid NativeRandomAccessFile_SetLength(FNIEnv *env, jobject obj, jlong newLength) {
    if(newLength < 0)
        return env->throwNew(env->findClass("java/io/IOException"), "Negative newLength");
    else if(newLength > 0xFFFFFFFFUL)
        return env->throwNew(env->findClass("java/io/IOException"), "The newLength is too large");
    jint fd = getFd(env, obj);
    if(fd == -1) return;
    void *handle = ((Hook *)fd)->getHandle();
    auto result = FlintAPI::IO::ftruncate((FlintAPI::IO::FileHandle)handle, (uint32_t)newLength);
    if(result != FlintAPI::IO::FILE_RESULT_OK)
        env->throwNew(env->findClass("java/io/IOException"), "setLength error %d", (uint32_t)result);
}

jvoid NativeRandomAccessFile_Close(FNIEnv *env, jobject obj) {
    Flint *flint = ((FExec *)env)->getFlint();
    flint->lock();
    jobject fdObj = obj->getFieldByIndex(0)->getObj();
    jint fd = fdObj->getFieldByIndex(0)->getInt32();
    if(fd != -1) {
        if(!(0 <= fd && fd <= 2)) {
            void *handle = ((Hook *)fd)->getHandle();
            if(FlintAPI::IO::fclose((FlintAPI::IO::FileHandle)handle) == FlintAPI::IO::FILE_RESULT_OK) {
                flint->removeShutdownHook((Hook *)fd);
                fdObj->getFieldByIndex(0)->setInt32(-1);
            }
            else
                env->throwNew(env->findClass("java/io/IOException"), "Stream closing failed");
        }
    }
    flint->unlock();
}
