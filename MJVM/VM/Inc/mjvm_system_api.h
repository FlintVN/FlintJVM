
#ifndef __MJVM_SYSTEM_API_H
#define __MJVM_SYSTEM_API_H

#include <stdint.h>
#include "mjvm_system_type.h"

void MjvmSystem_Write(const char *text, uint32_t length, uint8_t coder);

int64_t MjvmSystem_GetNanoTime(void);

void *MjvmSystem_FileOpen(const char *fileName, MjvmSys_FileMode mode);
MjvmSys_FileResult MjvmSystem_FileRead(void *fileHandle, void *buff, uint32_t btr, uint32_t *br);
MjvmSys_FileResult MjvmSystem_FileWrite(void *fileHandle, void *buff, uint32_t btw, uint32_t *bw);
uint32_t MjvmSystem_FileSize(void *fileHandle);
uint32_t MjvmSystem_FileTell(void *fileHandle);
MjvmSys_FileResult MjvmSystem_FileSeek(void *fileHandle, uint32_t offset);
MjvmSys_FileResult MjvmSystem_FileClose(void *fileHandle);

void *MjvmSystem_ThreadCreate(void (*task)(void *), void *param, uint32_t stackSize = 0);
void MjvmSystem_ThreadTerminate(void *threadHandle);
void MjvmSystem_ThreadSleep(uint32_t ms);

void *MjvmSystem_Malloc(uint32_t size);
void *MjvmSystem_Realloc(void *p, uint32_t size);
void MjvmSystem_Free(void *p);

#endif /* __MJVM_SYSTEM_API_H */
