
#ifndef __FLINT_CONF_H
#define __FLINT_CONF_H

#error "Do not build this file. You need to create a new file based on this template file and define your configuration."

#include "flint_common.h"

#define FILE_NAME_BUFF_SIZE     256

#define DEFAULT_STACK_SIZE      MEGA_BYTE(1)
#define OBJECT_SIZE_TO_GC       MEGA_BYTE(1)

#define MAX_OF_BREAK_POINT      20
#define MAX_OF_DBG_BUFFER       KILO_BYTE(1)

#endif /* __FLINT_CONF_H */
