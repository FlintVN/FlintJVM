
#ifndef __MJVM_HEAP_H
#define __MJVM_HEAP_H

#include <stdint.h>

void *MJVM_Malloc(uint32_t size);
void MJVM_Free(void *p);

#endif /* __MJVM_HEAP_H */
