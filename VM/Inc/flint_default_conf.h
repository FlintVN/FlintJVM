
#ifndef __FLINT_DEFAULT_CONF_H
#define __FLINT_DEFAULT_CONF_H

#if __has_include("flint_conf.h")
#include "flint_conf.h"
#endif

#ifndef FLINT_VARIANT_NAME
    #define FLINT_VARIANT_NAME          "Common FlintJVM"
    #warning "FLINT_VARIANT_NAME is not defined. Default value will be used"
#endif /* FLINT_VARIANT_NAME */

#ifndef FILE_NAME_BUFF_SIZE
    #define FILE_NAME_BUFF_SIZE         256
    #warning "FILE_NAME_BUFF_SIZE is not defined. Default value will be used"
#endif /* FILE_NAME_BUFF_SIZE */

#ifndef DEFAULT_STACK_SIZE
    #define DEFAULT_STACK_SIZE          MEGA_BYTE(1)
    #warning "DEFAULT_STACK_SIZE is not defined. Default value will be used"
#endif /* DEFAULT_STACK_SIZE */

#ifndef OBJECT_SIZE_TO_GC
    #define OBJECT_SIZE_TO_GC           MEGA_BYTE(1)
    #warning "OBJECT_SIZE_TO_GC is not defined. Default value will be used"
#endif /* OBJECT_SIZE_TO_GC */

#ifndef MAX_OF_BREAK_POINT
    #define MAX_OF_BREAK_POINT          20
    #warning "MAX_OF_BREAK_POINT is not defined. Default value will be used"
#endif /* MAX_OF_BREAK_POINT */

#ifdef DBG_TX_BUFFER_SIZE
    #if(DBG_TX_BUFFER_SIZE < 16)
        #error "DBG_TX_BUFFER_SIZE is at least 16 bytes"
    #endif
#else
    #ifndef DBG_TX_BUFFER_SIZE
        #define DBG_TX_BUFFER_SIZE      KILO_BYTE(1)
        #warning "DBG_TX_BUFFER_SIZE is not defined. Default value will be used"
    #endif
#endif /* DBG_TX_BUFFER_SIZE */

#ifndef DBG_CONSOLE_BUFFER_SIZE
    #define DBG_CONSOLE_BUFFER_SIZE     KILO_BYTE(1)
    #warning "DBG_CONSOLE_BUFFER_SIZE is not defined. Default value will be used"
#endif /* DBG_CONSOLE_BUFFER_SIZE */

#endif /* __FLINT_DEFAULT_CONF_H */
