#ifndef __FLINT_CLASS_DATA_BINARY_TREE_H
#define __FLINT_CLASS_DATA_BINARY_TREE_H

#include "flint_const_pool.h"
#include "flint_fields_data.h"
#include "flint_class_loader.h"

class FlintClassData : public FlintClassLoader {
private:
    FlintClassData *left;
    FlintClassData *right;
    uint32_t height;
public:
    uint32_t ownId;
    uint32_t monitorCount : 31;
    uint32_t isInitializing : 1;
    FlintFieldsData *staticFieldsData;
private:
    FlintClassData(class Flint &flint, const char *fileName, uint16_t length);

    FlintClassData(const FlintClassData &) = delete;
    void operator=(const FlintClassData &) = delete;

    void clearStaticFields(void);

    ~FlintClassData(void);

    friend class Flint;
    friend class FlintClassDataBinaryTree;
};

class FlintClassDataBinaryTree {
private:
    FlintClassData *root;
    class Flint &flint;

    FlintClassDataBinaryTree(const FlintClassDataBinaryTree &) = delete;
    void operator=(const FlintClassDataBinaryTree &) = delete;

    static uint32_t getHeight(FlintClassData *node);
    static int32_t getBalance(FlintClassData *node);
    static void updateHeight(FlintClassData *node);
    static FlintClassData *rotateRight(FlintClassData *y);
    static FlintClassData *rotateLeft(FlintClassData *x);
    static FlintClassData *balance(FlintClassData *node);
    FlintClassData *insert(FlintClassData *rootNode, const char *text, uint32_t hash, FlintClassData **node);

    void forEach(FlintClassData *node, void (*func)(FlintClassData &item));
    void freeNode(FlintClassData *node);
public:
    FlintClassDataBinaryTree(class Flint *flint);

    FlintClassData &add(const char *className, uint16_t length);
    FlintClassData &add(const FlintConstUtf8 &utf8);

    FlintClassData *find(const char *className, uint16_t length) const;
    FlintClassData *find(const FlintConstUtf8 &utf8) const;

    void forEach(void (*func)(FlintClassData &));
    void clear(void);
};

#endif /* __FLINT_CLASS_DATA_BINARY_TREE_H */
