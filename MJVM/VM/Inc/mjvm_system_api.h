
#ifndef __MJVM_SYSTEM_API_H
#define __MJVM_SYSTEM_API_H

#include <stdint.h>
#include "mjvm_system_type.h"

void MjvmSystem_WriteChar(uint16_t ch);

int64_t MjvmSystem_GetNanoTime(void);

void *MjvmSystem_FileOpen(const char *fileName, MjvmSys_FileMode mode);
MjvmSys_FileResult MjvmSystem_FileRead(void *fileHandle, void *buff, uint32_t btr, uint32_t *br);
MjvmSys_FileResult MjvmSystem_FileWrite(void *fileHandle, void *buff, uint32_t btw, uint32_t *bw);
uint32_t MjvmSystem_FileSize(void *fileHandle);
uint32_t MjvmSystem_FileTell(void *fileHandle);
MjvmSys_FileResult MjvmSystem_FileSeek(void *fileHandle, uint32_t offset);
MjvmSys_FileResult MjvmSystem_FileClose(void *fileHandle);

#endif /* __MJVM_SYSTEM_API_H */
