
#ifndef __FLINT_STACK_INFO_H
#define __FLINT_STACK_INFO_H

typedef enum : uint8_t {
    STACK_TYPE_NON_OBJECT = 0,
    STACK_TYPE_OBJECT = 1,
} FlintStackType;

typedef struct {
    FlintStackType type;
    int32_t value;
} FlintStackValue;

#endif /* __FLINT_STACK_INFO_H */
