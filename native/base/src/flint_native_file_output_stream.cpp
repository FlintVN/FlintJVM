
#include "flint.h"
#include "flint_native_common.h"
#include "flint_native_file_output_stream.h"

static jint getFd(FNIEnv *env, jobject obj) {
    jobject fdObj = obj->getFieldByIndex(0)->getObj();
    jint fd = fdObj->getFieldByIndex(0)->getInt32();
    if(fd == -1)
        env->throwNew(env->findClass("java/io/IOException"), "Stream closed");
    return fd;
}

jvoid NativeFileOutputStream_Open(FNIEnv *env, jobject obj, jstring name, jbool append) {
    char buff[FILE_NAME_BUFF_SIZE];
    Flint *flint = ((FExec *)env)->getFlint();
    if(flint->resolvePath(name->getAscii(), name->getLength(), buff, sizeof(buff)) == -1) return;
    flint->lock();
    jobject fdObj = obj->getFieldByIndex(0)->getObj();
    jint fd = fdObj->getFieldByIndex(0)->getInt32();
    if(fd != -1)
        env->throwNew(env->findClass("java/io/IOException"), "Stream has been opened");
    else {
        FlintAPI::IO::FileMode fileMode;
        if(append)
            fileMode = (FlintAPI::IO::FileMode)(FlintAPI::IO::FILE_MODE_APPEND | FlintAPI::IO::FILE_MODE_WRITE);
        else
            fileMode = (FlintAPI::IO::FileMode)(FlintAPI::IO::FILE_MODE_CREATE_ALWAYS | FlintAPI::IO::FILE_MODE_WRITE);
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

jvoid NativeFileOutputStream_Write(FNIEnv *env, jobject obj, jint b) {
    jint fd = getFd(env, obj);
    if(fd == -1) return;
    else if(fd == 0) {  /* in */

    }
    else if(fd == 1) {  /* out */
        uint8_t buff = (uint8_t)b;
        ((FExec *)env)->getFlint()->consoleWrite(&buff, 1);
    }
    else if(fd == 2) {  /* err */

    }
    else {
        uint8_t data = b & 0xFF;
        uint32_t bw = 0;
        void *handle = ((Hook *)fd)->getHandle();
        auto result = FlintAPI::IO::fwrite((FlintAPI::IO::FileHandle)handle, &data, 1, &bw);
        if(result != FlintAPI::IO::FILE_RESULT_OK || bw != 1)
            env->throwNew(env->findClass("java/io/IOException"), "Error while writing");
    }
}

jvoid NativeFileOutputStream_WriteBytes(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len) {
    if(!CheckArrayIndexSize(env, b, off, len)) return;
    jint fd = getFd(env, obj);
    if(fd == -1) return;
    else if(fd == 0) {  /* in */

    }
    else if(fd == 1) {   /* out */
        int8_t *data = &b->getData()[off];
        ((FExec *)env)->getFlint()->consoleWrite((uint8_t *)data, len);
    }
    else if(fd == 2) {  /* err */

    }
    else {
        int8_t *data = &b->getData()[off];
        uint32_t bw = 0;
        void *handle = ((Hook *)fd)->getHandle();
        auto result = FlintAPI::IO::fwrite((FlintAPI::IO::FileHandle)handle, data, len, &bw);
        if(result != FlintAPI::IO::FILE_RESULT_OK || bw != len)
            env->throwNew(env->findClass("java/io/IOException"), "Error while writing");
    }
}

jvoid NativeFileOutputStream_Close(FNIEnv *env, jobject obj) {
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
