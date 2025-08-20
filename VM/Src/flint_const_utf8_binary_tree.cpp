
#include <string.h>
#include "flint.h"
#include "flint_const_utf8_binary_tree.h"

FlintConstUtf8BinaryTree::FlintConstUtf8BinaryTree(void) : root(NULL_PTR) {

}

uint32_t FlintConstUtf8BinaryTree::getHeight(FlintConstUtf8Node *node) {
    return node ? node->height : 0;
}

int32_t FlintConstUtf8BinaryTree::getBalance(FlintConstUtf8Node *node) {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

void FlintConstUtf8BinaryTree::updateHeight(FlintConstUtf8Node *node) {
    if(node) {
        uint32_t leftHeight = getHeight(node->left);
        uint32_t rightHeight = getHeight(node->right);
        node->height = 1 + FLINT_MAX(leftHeight, rightHeight);
    }
}

FlintConstUtf8BinaryTree::FlintConstUtf8Node *FlintConstUtf8BinaryTree::rotateRight(FlintConstUtf8Node *y) {
    FlintConstUtf8Node *x = y->left;
    FlintConstUtf8Node *T = x->right;

    x->right = y;
    y->left = T;

    updateHeight(y);
    updateHeight(x);

    return x;
}

FlintConstUtf8BinaryTree::FlintConstUtf8Node *FlintConstUtf8BinaryTree::rotateLeft(FlintConstUtf8Node *x) {
    FlintConstUtf8Node *y = x->right;
    FlintConstUtf8Node *T = y->left;

    y->left = x;
    x->right = T;

    updateHeight(x);
    updateHeight(y);

    return y;
}

FlintConstUtf8BinaryTree::FlintConstUtf8Node *FlintConstUtf8BinaryTree::balance(FlintConstUtf8Node *node) {
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

int32_t FlintConstUtf8BinaryTree::compareConstUtf8(const char *text, uint32_t hash, FlintConstUtf8 &uft8, bool isTypeName) {
    uint32_t utf8Hash = CONST_UTF8_HASH(uft8);
    if(hash < utf8Hash)
        return -1;
    if(hash > utf8Hash)
        return 1;
    if(text == uft8.text)
        return 0;
    if(isTypeName) {
        const char *text2 = uft8.text;
        uint16_t length = ((uint16_t *)&hash)[0];
        for(uint16_t i = 0; i < length; i++) {
            if((text[i] == text2[i]) || (text[i] == '.' && text2[i] == '/'))
                continue;
            return text[i] - text2[i];
        }
        return 0;
    }
    else
        return strncmp(text, uft8.text, uft8.length);
}

FlintConstUtf8BinaryTree::FlintConstUtf8Node *FlintConstUtf8BinaryTree::createFlintConstUtf8Node(const char *text, uint32_t hash, bool isTypeName) {
    uint16_t length = ((uint16_t *)&hash)[0];
    FlintConstUtf8Node *newNode = (FlintConstUtf8Node *)Flint::malloc(sizeof(FlintConstUtf8Node) + length + 1);
    newNode->left = NULL_PTR;
    newNode->right = NULL_PTR;
    newNode->height = 1;
    *(uint16_t *)&newNode->value.length = length;
    *(uint16_t *)&newNode->value.crc = ((uint16_t *)&hash)[1];
    char *textBuff = (char *)newNode->value.text;
    if(isTypeName) {
        char *textBuff = (char *)newNode->value.text;
        for(uint16_t i = 0; i < length; i++)
            textBuff[i] = (text[i] == '.') ? '/' : text[i];
    }
    else
        strncpy(textBuff, text, length);
    textBuff[length] = 0;
    return newNode;
}

FlintConstUtf8BinaryTree::FlintConstUtf8Node *FlintConstUtf8BinaryTree::insert(FlintConstUtf8Node *rootNode, const char *text, uint32_t hash, bool isTypeName, FlintConstUtf8Node **node) {
    if(!rootNode) {
        FlintConstUtf8Node *constUtf8Node = createFlintConstUtf8Node(text, hash, isTypeName);
        if(node)
            *node = constUtf8Node;
        return constUtf8Node;
    }
    int32_t compareResult = compareConstUtf8(text, hash, rootNode->value, isTypeName);
    if(compareResult < 0)
        rootNode->left = insert(rootNode->left, text, hash, isTypeName, node);
    else if(compareResult > 0)
        rootNode->right = insert(rootNode->right, text, hash, isTypeName, node);
    else
        return rootNode;
    return balance(rootNode);
}

FlintConstUtf8 &FlintConstUtf8BinaryTree::add(const char *text, uint32_t hash, bool isTypeName) {
    FlintConstUtf8Node *newNode;
    root = insert(root, text, hash, isTypeName, &newNode);
    return newNode->value;
}

FlintConstUtf8 *FlintConstUtf8BinaryTree::find(const char *text, uint32_t hash, bool isTypeName) const {
    FlintConstUtf8Node *node = root;
    while(node) {
        int32_t compareResult = compareConstUtf8(text, hash, node->value, isTypeName);
        if(compareResult == 0)
            return &node->value;
        else if(compareResult > 0)
            node = node->right;
        else
            node = node->left;
    }
    return NULL_PTR;
}

void FlintConstUtf8BinaryTree::freeNode(FlintConstUtf8Node *node) {
    if(node) {
        freeNode(node->left);
        freeNode(node->right);
        Flint::free(node);
    }
}

void FlintConstUtf8BinaryTree::clear(void) {
    freeNode(root);
    root = NULL_PTR;
}
