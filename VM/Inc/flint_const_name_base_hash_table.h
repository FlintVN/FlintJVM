
#ifndef __FLINT_CONST_NAME_BASE_HASH_TABLE_H
#define __FLINT_CONST_NAME_BASE_HASH_TABLE_H

#include "flint_const_pool.h"

typedef struct {
    uint32_t count;
    const FlintConstUtf8 *values[];
} ConstNameBaseList;

extern const ConstNameBaseList * const baseConstUtf8HashTable[56];

#endif /* __FLINT_CONST_NAME_BASE_HASH_TABLE_H */
