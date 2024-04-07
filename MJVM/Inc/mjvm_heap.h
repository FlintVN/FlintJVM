
#ifndef __MJVM_HEAP_H
#define __MJVM_HEAP_H

#include <stdint.h>
#include "mjvm_const_pool.h"

class MjvmObject {
public:
    const uint32_t size : 31;
private:
    uint32_t prot : 1;
public:
    const ConstUtf8 &type;
    const uint8_t dimensions;
    uint8_t data[];

    uint8_t parseTypeSize(void) const;

    void setProtected(void);
    bool isProtected(void) const;
private:
    MjvmObject(uint32_t size, const ConstUtf8 &type, uint8_t dimensions);
    MjvmObject(const MjvmObject &) = delete;
    void operator=(const MjvmObject &) = delete;

    friend class MjvmHeap;
};

class MjvmHeap {
public:
    static void *malloc(uint32_t size);
    static void free(void *p);
    static void freeObject(MjvmObject *obj);
    static void freeAllObject(uint32_t id);
    static void garbageCollection(uint32_t id);

    static MjvmObject *newObject(uint32_t id, uint32_t size, const ConstUtf8 &type, uint8_t dimensions = 0);
private:
    class MjvmObjectNode {
    public:
        uint32_t id;
        MjvmObjectNode *prev;
        MjvmObjectNode *next;

        MjvmObject *getMjvmObject(void) const;
    private:
        uint8_t data[];
    };

    static MjvmObjectNode *objectList;

    MjvmHeap(void) = delete;
    MjvmHeap(const MjvmHeap &) = delete;
    void operator=(const MjvmHeap &) = delete;

    static void freeObject(MjvmObjectNode *objNode);
    static void addToObjectList(MjvmObjectNode *objNode);
    static void removeFromObjectList(MjvmObjectNode *objNode);
};

#endif /* __MJVM_HEAP_H */
