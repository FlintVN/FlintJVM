
#ifndef __MJVM_STACK_INFO_H
#define __MJVM_STACK_INFO_H

typedef enum : uint8_t {
    STACK_TYPE_NON_OBJECT = 0,
    STACK_TYPE_OBJECT = 1,
} MjvmStackType;

typedef struct {
    MjvmStackType type;
    int32_t value;
} MjvmStackValue;

#endif /* __MJVM_STACK_INFO_H */
