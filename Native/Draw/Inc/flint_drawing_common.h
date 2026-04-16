
#ifndef __FLINT_DRAWING_COMMON_H
#define __FLINT_DRAWING_COMMON_H

#include <stdint.h>
#include "flint_default_conf.h"

#if FLINT_API_DRAW_ENABLED

typedef struct {
    int32_t w;
    int32_t h;
    int32_t clipX1;
    int32_t clipY1;
    int32_t clipX2;
    int32_t clipY2;
    uint8_t *data;
} FGfx;

#define F_MAX(_a, _b)   ((_a) > (_b) ? (_a) : (_b))
#define F_MIN(_a, _b)   ((_a) < (_b) ? (_a) : (_b))
#define F_ABS(_a)       ((_a) < 0 ? -(_a) : (_a)) 
#define F_SWAP(_a, _b)  do { int tmp = (_a); (_a) = (_b); (_b) = tmp; } while(0)

#endif /* FLINT_API_DRAW_ENABLED */

#endif /* __FLINT_DRAWING_COMMON_H */
