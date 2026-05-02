#define _POSIX_C_SOURCE 200809L
#include "splay.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Splay Tree Implementation
 *
 * The splay operation brings a node to the root using three rotations:
 *   Zig:     Single rotation (node is child of root)
 *   Zig-Zig: Two rotations same direction (node and parent same side)
 *   Zig-Zag: Two rotations opposite direction (node and parent differ)
 *
 * Amortized O(log n) for all operations via the splay operation.
 */

/* ===================== Internal Node Helpers ===================== */

static SplayNode* _new_node(int key, const char *value) {
    SplayNode *n = (SplayNode *)malloc(sizeof(SplayNode));
    if (!n) return NULL;
    n->key   = key;
    n->value = value ? strdup(value) : NULL;
    n->left  = NULL;
    n->right = NULL;
    return n;
}

static void _free_node(SplayNode *n) {
    if (!n) return;
    free(n->value);
    free(n);
}

/* Right rotation around y */
static SplayNode* _rotate_right(SplayNode *y) {
    SplayNode *x = y->left;
    y->left  = x->right;
    x->right = y;
    return x;
}

/* Left rotation around x */
static SplayNode* _rotate_left(SplayNode *x) {
    SplayNode *y = x->right;
    x->right = y->left;
    y->left  = x;
    return y;
}

/* ===================== Splay Operation ===================== */

/*
 * Splay: bring the node with the given key to root.
 * If key is not present, the last accessed node becomes root.
 * Returns the new root.
 */
static SplayNode* _splay(SplayNode *root, int key, int *traversals) {
    if (!root) return NULL;
    if (traversals) (*traversals)++;

    /* Key already at root */
    if (root->key == key) return root;

    if (key < root->key) {
        /* Key is in left subtree */
        if (!root->left) return root;  /* Not found, root is closest */
        if (traversals) (*traversals)++;

        if (key < root->left->key) {
            /* Zig-Zig (Left-Left) */
            root->left->left = _splay(root->left->left, key, traversals);
            root = _rotate_right(root);
        } else if (key > root->left->key) {
            /* Zig-Zag (Left-Right) */
            root->left->right = _splay(root->left->right, key, traversals);
            if (root->left->right)
                root->left = _rotate_left(root->left);
        }
        return root->left ? _rotate_right(root) : root;
    } else {
        /* Key is in right subtree */
        if (!root->right) return root;
        if (traversals) (*traversals)++;

        if (key > root->right->key) {
            /* Zig-Zig (Right-Right) */
            root->right->right = _splay(root->right->right, key, traversals);
            root = _rotate_left(root);
        } else if (key < root->right->key) {
            /* Zig-Zag (Right-Left) */
            root->right->left = _splay(root->right->left, key, traversals);
            if (root->right->left)
                root->right = _rotate_right(root->right);
        }
        return root->right ? _rotate_left(root) : root;
    }
}

/* ===================== Core Operations ===================== */

SplayTree* splay_create(void) {
    SplayTree *t = (SplayTree *)calloc(1, sizeof(SplayTree));
    return t;
}

void splay_insert(SplayTree *tree, int key, const char *value) {
    if (!tree) return;

    if (!tree->root) {
        tree->root = _new_node(key, value);
        if (tree->root) tree->node_count++;
        return;
    }

    /* Splay the closest key to root */
    tree->root = _splay(tree->root, key, &tree->traversal_count);
    tree->splay_count++;

    if (tree->root->key == key) {
        /* Key exists: update value */
        free(tree->root->value);
        tree->root->value = value ? strdup(value) : NULL;
        return;
    }

    SplayNode *n = _new_node(key, value);
    if (!n) return;

    if (key < tree->root->key) {
        /* New node becomes root; root's left subtree goes left of new node */
        n->right      = tree->root;
        n->left       = tree->root->left;
        tree->root->left = NULL;
    } else {
        /* New node becomes root; root's right subtree goes right of new node */
        n->left        = tree->root;
        n->right       = tree->root->right;
        tree->root->right = NULL;
    }

    tree->root = n;
    tree->node_count++;
}

char* splay_search(SplayTree *tree, int key, int *traversals) {
    if (!tree || !tree->root) return NULL;

    tree->root = _splay(tree->root, key, traversals);
    tree->splay_count++;

    if (tree->root->key == key) return tree->root->value;
    return NULL;
}

int splay_delete(SplayTree *tree, int key) {
    if (!tree || !tree->root) return 0;

    tree->root = _splay(tree->root, key, &tree->traversal_count);
    tree->splay_count++;

    if (tree->root->key != key) return 0;  /* Not found */

    SplayNode *old_root = tree->root;

    if (!tree->root->left) {
        tree->root = tree->root->right;
    } else {
        /* Splay max of left subtree to root of left subtree */
        SplayNode *left = _splay(tree->root->left, key, &tree->traversal_count);
        left->right = tree->root->right;
        tree->root = left;
    }

    _free_node(old_root);
    tree->node_count--;
    return 1;
}

/* ===================== Height & Node Count ===================== */

static int _height(SplayNode *n) {
    if (!n) return 0;
    int l = _height(n->left);
    int r = _height(n->right);
    return 1 + (l > r ? l : r);
}

int splay_height(SplayTree *tree) {
    return tree ? _height(tree->root) : 0;
}

int splay_node_count(SplayTree *tree) {
    return tree ? tree->node_count : 0;
}

/* ===================== Range Query ===================== */

static void _range_collect(SplayNode *n, int start, int end,
                           SplayRangeResult *res, int *traversals) {
    if (!n) return;
    if (traversals) (*traversals)++;

    if (n->key > start) _range_collect(n->left,  start, end, res, traversals);
    if (n->key >= start && n->key <= end) {
        if (res->count == res->capacity) {
            res->capacity *= 2;
            res->keys   = (int *)  realloc(res->keys,   res->capacity * sizeof(int));
            res->values = (char **)realloc(res->values, res->capacity * sizeof(char *));
        }
        res->keys[res->count]   = n->key;
        res->values[res->count] = n->value;
        res->count++;
    }
    if (n->key < end) _range_collect(n->right, start, end, res, traversals);
}

SplayRangeResult* splay_range_query(SplayTree *tree, int start, int end, int *traversals) {
    SplayRangeResult *res = (SplayRangeResult *)malloc(sizeof(SplayRangeResult));
    if (!res) return NULL;

    res->capacity = 16;
    res->count    = 0;
    res->keys     = (int *)  malloc(res->capacity * sizeof(int));
    res->values   = (char **)malloc(res->capacity * sizeof(char *));

    if (!res->keys || !res->values) {
        free(res->keys); free(res->values); free(res);
        return NULL;
    }

    if (tree) _range_collect(tree->root, start, end, res, traversals);
    return res;
}

void splay_range_result_free(SplayRangeResult *result) {
    if (!result) return;
    free(result->keys);
    free(result->values);
    free(result);
}

/* ===================== Print ===================== */

static void _print_inorder(SplayNode *n, int depth) {
    if (!n) return;
    _print_inorder(n->left,  depth + 1);
    for (int i = 0; i < depth; i++) printf("  ");
    printf("[%d] => %s\n", n->key, n->value ? n->value : "(null)");
    _print_inorder(n->right, depth + 1);
}

void splay_print(SplayTree *tree) {
    if (!tree || !tree->root) {
        printf("(empty splay tree)\n");
        return;
    }
    printf("Splay Tree (in-order, root=[%d], nodes=%d, splays=%d):\n",
           tree->root->key, tree->node_count, tree->splay_count);
    _print_inorder(tree->root, 0);
}

/* ===================== Destroy ===================== */

static void _destroy_nodes(SplayNode *n) {
    if (!n) return;
    _destroy_nodes(n->left);
    _destroy_nodes(n->right);
    _free_node(n);
}

void splay_destroy(SplayTree *tree) {
    if (!tree) return;
    _destroy_nodes(tree->root);
    free(tree);
}
