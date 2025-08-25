#ifndef __FLINT_CLASS_DATA_BINARY_TREE_H
#define __FLINT_CLASS_DATA_BINARY_TREE_H

#include "flint_const_pool.h"
#include "flint_fields_data.h"
#include "flint_class_loader.h"

typedef enum {
    UNINITIALIZED = 0x00,
    INITIALIZING = 0x01,
    INITIALIZED = 0x02,
} FlintInitStatus;

class FlintClassData : public FlintClassLoader {
private:
    FlintClassData *left;
    FlintClassData *right;
    uint32_t height;

    uint32_t ownId;
    uint32_t monitorCount;
    uint32_t staticInitOwnId;
    FlintFieldsData *staticFieldsData;

    FlintClassData(class Flint &flint);

    FlintClassData(const FlintClassData &) = delete;
    void operator=(const FlintClassData &) = delete;

    FlintInitStatus getInitStatus(void) const;

    void clearStaticFields(void);

    ~FlintClassData(void);

    friend class Flint;
    friend class FlintExecution;
    friend class FlintClassDataBinaryTree;
};

class FlintClassDataBinaryTree {
private:
    FlintClassData *root;

    FlintClassDataBinaryTree(const FlintClassDataBinaryTree &) = delete;
    void operator=(const FlintClassDataBinaryTree &) = delete;

    static uint32_t getHeight(FlintClassData *node);
    static int32_t getBalance(FlintClassData *node);
    static void updateHeight(FlintClassData *node);
    static FlintClassData *rotateRight(FlintClassData *y);
    static FlintClassData *rotateLeft(FlintClassData *x);
    static FlintClassData *balance(FlintClassData *node);
    static FlintClassData *insert(FlintClassData *rootNode, FlintClassData &classData);

    void forEach(FlintClassData *node, void (*func)(FlintClassData &item));
    void freeNode(FlintClassData *node);
public:
    FlintClassDataBinaryTree(void);

    FlintResult<void> add(FlintClassData &classData);

    FlintClassData *find(const char *className, uint16_t length) const;
    FlintClassData *find(FlintConstUtf8 &utf8) const;

    void forEach(void (*func)(FlintClassData &));
    void clear(void);
};

#endif /* __FLINT_CLASS_DATA_BINARY_TREE_H */
