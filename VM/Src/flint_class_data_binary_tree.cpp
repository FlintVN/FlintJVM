
#include <new>
#include <string.h>
#include "flint.h"
#include "flint_class_data_binary_tree.h"

FlintInitStatus FlintClassData::getInitStatus(void) const {
    if(!staticFieldsData)
        return UNINITIALIZED;
    else if(staticInitOwnId)
        return INITIALIZING;
    return INITIALIZED;
}

void FlintClassData::clearStaticFields(void) {
    if(staticFieldsData) {
        staticFieldsData->~FlintFieldsData();
        Flint::free(staticFieldsData);
        staticFieldsData = NULL_PTR;
    }
}

FlintClassData::~FlintClassData() {
    clearStaticFields();
}

FlintClassData::FlintClassData(Flint &flint) : FlintClassLoader(flint) {
    ownId = 0;
    monitorCount = 0;
    staticInitOwnId = 0;
    staticFieldsData = NULL_PTR;
    left = NULL_PTR;
    right = NULL_PTR;
    height = 1;
}

FlintClassDataBinaryTree::FlintClassDataBinaryTree(void) : root(NULL_PTR) {

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

FlintClassData *FlintClassDataBinaryTree::insert(FlintClassData *rootNode, FlintClassData &classData) {
    if(!rootNode)
        return &classData;
    uint32_t hash = CONST_UTF8_HASH(*classData.thisClass);
    int8_t compareResult = compareConstUtf8(classData.thisClass->text, hash, *rootNode->thisClass);
    if(compareResult < 0)
        rootNode->left = insert(rootNode->left, classData);
    else if(compareResult > 0)
        rootNode->right = insert(rootNode->right, classData);
    else
        return rootNode;
    return balance(rootNode);
}

FlintResult<void> FlintClassDataBinaryTree::add(FlintClassData &classData) {
    root = insert(root, classData);
    return ERR_OK;
}

FlintClassData *FlintClassDataBinaryTree::find(const char *text, uint16_t length) const {
    uint32_t hash = Flint_CalcHash(text, length, false);
    FlintClassData *node = root;
    while(node) {
        int8_t compareResult = compareConstUtf8(text, hash, *node->thisClass);
        if(compareResult == 0)
            return node;
        else if(compareResult > 0)
            node = node->right;
        else
            node = node->left;
    }
    return NULL_PTR;
}

FlintClassData *FlintClassDataBinaryTree::find(const FlintConstUtf8 &utf8) const {
    uint32_t hash = CONST_UTF8_HASH(utf8);
    FlintClassData *node = root;
    while(node) {
        int8_t compareResult = compareConstUtf8(utf8.text, hash, *node->thisClass);
        if(compareResult == 0)
            return node;
        else if(compareResult > 0)
            node = node->right;
        else
            node = node->left;
    }
    return NULL_PTR;
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
        node->~FlintClassData();
        Flint::free(node);
    }
}

void FlintClassDataBinaryTree::clear(void) {
    freeNode(root);
    root = NULL_PTR;
}
