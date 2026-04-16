
#ifndef __FLINT_GFX_H
#define __FLINT_GFX_H

#include <stddef.h>
#include <stdint.h>

#define GFX_F_BITS              4

#define GFX_MAX(_a, _b)         ((_a) > (_b) ? (_a) : (_b))
#define GFX_MIN(_a, _b)         ((_a) < (_b) ? (_a) : (_b))
#define GFX_ABS(_a)             ((_a) < 0 ? -(_a) : (_a)) 
#define GFX_SWAP(_a, _b)        do { decltype(_a) tmp = (_a); (_a) = (_b); (_b) = tmp; } while(0)

#endif /* __FLINT_GFX_H */
