
#ifndef __FLINT_DEFAULT_COMPILER_DEFINE_H
#define __FLINT_DEFAULT_COMPILER_DEFINE_H

#if __has_include("flint_compiler_define.h")
#include "flint_compiler_define.h"
#endif
    #if defined(__GNUC__)
        #define RO_DATA     __attribute__((section(".rodata")))
    #elif defined(__clang__)
        #error "Not supported yet"
    #elif defined(_MSC_VER)
        #error "Not supported yet"
    #else
        #error "Not supported yet"
#endif /* __has_include("flint_compiler_define.h") */

#endif /* __FLINT_DEFAULT_COMPILER_DEFINE_H */
