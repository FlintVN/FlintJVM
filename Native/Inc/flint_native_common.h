
#ifndef __FLINT_NATIVE_COMMON_H
#define __FLINT_NATIVE_COMMON_H

#include "flint_std.h"
#include "flint_native_interface.h"

bool CheckIndex(FNIEnv *env, jarray array, int32_t index);
bool CheckArrayIndexSize(FNIEnv *env, jarray arr, int32_t index, int32_t count);

#endif /* __FLINT_NATIVE_COMMON_H */
