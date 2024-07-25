
#ifndef __FLINT_SYSTEM_API_H
#define __FLINT_SYSTEM_API_H

#include <stdint.h>
#include "flint_system_type.h"

void FlintSystem_Write(const char *text, uint32_t length, uint8_t coder);

int64_t FlintSystem_GetNanoTime(void);

void *FlintSystem_FileOpen(const char *fileName, FlintSys_FileMode mode);
FlintSys_FileResult FlintSystem_FileRead(void *fileHandle, void *buff, uint32_t btr, uint32_t *br);
FlintSys_FileResult FlintSystem_FileWrite(void *fileHandle, void *buff, uint32_t btw, uint32_t *bw);
uint32_t FlintSystem_FileSize(void *fileHandle);
uint32_t FlintSystem_FileTell(void *fileHandle);
FlintSys_FileResult FlintSystem_FileSeek(void *fileHandle, uint32_t offset);
FlintSys_FileResult FlintSystem_FileClose(void *fileHandle);

void *FlintSystem_ThreadCreate(void (*task)(void *), void *param, uint32_t stackSize = 0);
void FlintSystem_ThreadTerminate(void *threadHandle);
void FlintSystem_ThreadSleep(uint32_t ms);

void *FlintSystem_Malloc(uint32_t size);
void *FlintSystem_Realloc(void *p, uint32_t size);
void FlintSystem_Free(void *p);

#endif /* __FLINT_SYSTEM_API_H */
