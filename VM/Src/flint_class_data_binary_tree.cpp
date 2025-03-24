
#include <new>
#include <string.h>
#include "flint.h"
#include "flint_class_data_binary_tree.h"

void FlintClassData::clearStaticFields(void) {
    if(staticFieldsData) {
        staticFieldsData->~FlintFieldsData();
        Flint::free(staticFieldsData);
        staticFieldsData = 0;
    }
}

FlintClassData::~FlintClassData() {
    clearStaticFields();
}

FlintClassData::FlintClassData(Flint &flint, const char *fileName, uint16_t length) : FlintClassLoader(flint, fileName, length) {
    ownId = 0;
    monitorCount = 0;
    isInitializing = 0;
    staticFieldsData = 0;
    left = 0;
    right = 0;
    height = 1;
}

FlintClassDataBinaryTree::FlintClassDataBinaryTree(void) : root(0) {

}

uint32_t FlintClassDataBinaryTree::getHeight(FlintClassData *node) {
    return node ? node->height : 0;
}

int32_t FlintClassDataBinaryTree::getBalance(FlintClassData *node) {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

void FlintClassDataBinaryTree::updateHeight(FlintClassData *node) {
    if(node) {
        uint32_t leftHeight = getHeight(node->left);
        uint32_t rightHeight = getHeight(node->right);
        node->height = 1 + FLINT_MAX(leftHeight, rightHeight);
    }
}

FlintClassData *FlintClassDataBinaryTree::rotateRight(FlintClassData *y) {
    FlintClassData *x = y->left;
    FlintClassData *T = x->right;

    x->right = y;
    y->left = T;

    updateHeight(y);
    updateHeight(x);

    return x;
}

FlintClassData *FlintClassDataBinaryTree::rotateLeft(FlintClassData *x) {
    FlintClassData *y = x->right;
    FlintClassData *T = y->left;

    y->left = x;
    x->right = T;

    updateHeight(x);
    updateHeight(y);

    return y;
}

FlintClassData *FlintClassDataBinaryTree::balance(FlintClassData *node) {
    if(!node)
        return node;
    updateHeight(node);
    int balanceFactor = getBalance(node);
    if(balanceFactor > 1) {
        if(getBalance(node->left) >= 0)
            return rotateRight(node);
        else {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
    }
    else if(balanceFactor < -1) {
        if(getBalance(node->right) <= 0)
            return rotateLeft(node);
        else {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
    }
    return node;
}

static int32_t compareConstUtf8(const char *text, uint32_t hash, FlintConstUtf8 &uft8) {
    uint32_t utf8Hash = CONST_UTF8_HASH(uft8);
    if(hash < utf8Hash)
        return -1;
    if(hash > utf8Hash)
        return 1;
    if(text == uft8.text)
        return 0;
    return strncmp(text, uft8.text, uft8.length);
}

FlintClassData *FlintClassDataBinaryTree::insert(Flint *flint, FlintClassData *rootNode, const char *text, uint32_t hash, FlintClassData **node) {
    if(!rootNode) {
        FlintClassData *classData = 0;
        try {
            uint16_t length = ((uint16_t *)&hash)[0];
            classData = (FlintClassData *)Flint::malloc(sizeof(FlintClassData));
            classData->staticFieldsData = 0;
            new (classData)FlintClassData(*flint, text, length);
            if(node)
                *node = classData;
            return classData;
        }
        catch(...) {
            if(classData) {
                classData->~FlintClassData();
                Flint::free(classData);
            }
            throw;
        }
    }
    int8_t compareResult = compareConstUtf8(text, hash, rootNode->getThisClass());
    if(compareResult < 0)
        rootNode->left = insert(flint, rootNode->left, text, hash, node);
    else if(compareResult > 0)
        rootNode->right = insert(flint, rootNode->right, text, hash, node);
    else
        return rootNode;
    return balance(rootNode);
}

FlintClassData &FlintClassDataBinaryTree::add(Flint *flint, const char *text, uint16_t length) {
    FlintClassData *newNode;
    uint32_t hash = Flint_CalcHash(text, length, false);
    root = insert(flint, root, text, hash, &newNode);
    return *newNode;
}

FlintClassData &FlintClassDataBinaryTree::add(Flint *flint, const FlintConstUtf8 &utf8) {
    FlintClassData *newNode;
    uint32_t hash = CONST_UTF8_HASH(utf8);
    root = insert(flint, root, utf8.text, hash, &newNode);
    return *newNode;
}

FlintClassData *FlintClassDataBinaryTree::find(const char *text, uint16_t length) const {
    uint32_t hash = Flint_CalcHash(text, length, false);
    FlintClassData *node = root;
    while(node) {
        int8_t compareResult = compareConstUtf8(text, hash, node->getThisClass());
        if(compareResult == 0)
            return node;
        else if(compareResult > 0)
            node = node->right;
        else
            node = node->left;
    }
    return NULL;
}

FlintClassData *FlintClassDataBinaryTree::find(const FlintConstUtf8 &utf8) const {
    uint32_t hash = CONST_UTF8_HASH(utf8);
    FlintClassData *node = root;
    while(node) {
        int8_t compareResult = compareConstUtf8(utf8.text, hash, node->getThisClass());
        if(compareResult == 0)
            return node;
        else if(compareResult > 0)
            node = node->right;
        else
            node = node->left;
    }
    return NULL;
}

void FlintClassDataBinaryTree::forEach(FlintClassData *node, void (*func)(FlintClassData &item)) {
    if(node) {
        forEach(node->left, func);
        forEach(node->right, func);
        func(*node);
    }
}

void FlintClassDataBinaryTree::forEach(void (*func)(FlintClassData &)) {
    forEach(root, func);
}

void FlintClassDataBinaryTree::freeNode(FlintClassData *node) {
    if(node) {
        freeNode(node->left);
        freeNode(node->right);
        Flint::free(node);
    }
}

void FlintClassDataBinaryTree::clear(void) {
    freeNode(root);
    root = 0;
}
