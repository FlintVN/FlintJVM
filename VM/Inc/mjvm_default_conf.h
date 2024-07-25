
#ifndef __MJVM_DEFAULT_CONF_H
#define __MJVM_DEFAULT_CONF_H

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

#ifdef MAX_OF_DBG_BUFFER
    #if(MAX_OF_DBG_BUFFER < 16)
        #error "MAX_OF_DBG_BUFFER is at least 16 bytes"
    #endif
#else
    #ifndef MAX_OF_DBG_BUFFER
        #define MAX_OF_DBG_BUFFER          KILO_BYTE(1)
        #warning "MAX_OF_DBG_BUFFER is not defined. Default value will be used"
    #endif
#endif /* MAX_OF_DBG_BUFFER */

#endif /* __MJVM_DEFAULT_CONF_H */
