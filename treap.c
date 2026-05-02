#define _POSIX_C_SOURCE 200809L
#include "treap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/*
 * Treap Implementation
 *
 * Each node has:
 *   key      — BST ordering property
 *   priority — randomly assigned; Max-Heap property maintained
 *
 * Rotations restore heap property after insert/delete.
 * The randomness of priorities gives expected O(log n) height.
 */

/* ===================== Internal Helpers ===================== */

static int _rand_priority(void) {
    return rand();
}

static TreapNode* _new_node(int key, const char *value) {
    TreapNode *n = (TreapNode *)malloc(sizeof(TreapNode));
    if (!n) return NULL;
    n->key      = key;
    n->priority = _rand_priority();
    n->value    = value ? strdup(value) : NULL;
    n->left     = NULL;
    n->right    = NULL;
    return n;
}

static void _free_node(TreapNode *n) {
    if (!n) return;
    free(n->value);
    free(n);
}

/* Right rotation: y's left child x becomes new root */
static TreapNode* _rotate_right(TreapNode *y) {
    TreapNode *x = y->left;
    y->left  = x->right;
    x->right = y;
    return x;
}

/* Left rotation: x's right child y becomes new root */
static TreapNode* _rotate_left(TreapNode *x) {
    TreapNode *y = x->right;
    x->right = y->left;
    y->left  = x;
    return y;
}

/* ===================== Insert ===================== */

static TreapNode* _insert(TreapNode *root, int key, const char *value,
                          int *count, int *traversals) {
    if (!root) {
        (*count)++;
        return _new_node(key, value);
    }

    if (traversals) (*traversals)++;

    if (key < root->key) {
        root->left = _insert(root->left, key, value, count, traversals);
        /* Fix heap property */
        if (root->left && root->left->priority > root->priority)
            root = _rotate_right(root);
    } else if (key > root->key) {
        root->right = _insert(root->right, key, value, count, traversals);
        if (root->right && root->right->priority > root->priority)
            root = _rotate_left(root);
    } else {
        /* Duplicate key: update value */
        free(root->value);
        root->value = value ? strdup(value) : NULL;
    }

    return root;
}

/* ===================== Search ===================== */

static TreapNode* _search(TreapNode *root, int key, int *traversals) {
    if (!root) return NULL;
    if (traversals) (*traversals)++;

    if (key == root->key) return root;
    if (key  < root->key) return _search(root->left,  key, traversals);
    return                       _search(root->right, key, traversals);
}

/* ===================== Delete ===================== */

static TreapNode* _delete(TreapNode *root, int key, int *count, int *found) {
    if (!root) return NULL;

    if (key < root->key) {
        root->left = _delete(root->left, key, count, found);
    } else if (key > root->key) {
        root->right = _delete(root->right, key, count, found);
    } else {
        /* Found — rotate down to leaf then free */
        *found = 1;
        if (!root->left && !root->right) {
            _free_node(root);
            (*count)--;
            return NULL;
        } else if (!root->left) {
            TreapNode *tmp = _rotate_left(root);
            tmp->left = _delete(tmp->left, key, count, found);
            return tmp;
        } else if (!root->right) {
            TreapNode *tmp = _rotate_right(root);
            tmp->right = _delete(tmp->right, key, count, found);
            return tmp;
        } else {
            /* Rotate towards higher-priority child */
            if (root->left->priority > root->right->priority) {
                TreapNode *tmp = _rotate_right(root);
                tmp->right = _delete(tmp->right, key, count, found);
                return tmp;
            } else {
                TreapNode *tmp = _rotate_left(root);
                tmp->left = _delete(tmp->left, key, count, found);
                return tmp;
            }
        }
    }

    return root;
}

/* ===================== Height ===================== */

static int _height(TreapNode *n) {
    if (!n) return 0;
    int l = _height(n->left);
    int r = _height(n->right);
    return 1 + (l > r ? l : r);
}

/* ===================== Range Query ===================== */

static void _range_collect(TreapNode *n, int start, int end,
                           TreapRangeResult *res, int *traversals) {
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

/* ===================== Print Inorder ===================== */

static void _print_inorder(TreapNode *n) {
    if (!n) return;
    _print_inorder(n->left);
    printf("  [key=%d, pri=%d] => %s\n", n->key, n->priority,
           n->value ? n->value : "(null)");
    _print_inorder(n->right);
}

/* ===================== Destroy ===================== */

static void _destroy_nodes(TreapNode *n) {
    if (!n) return;
    _destroy_nodes(n->left);
    _destroy_nodes(n->right);
    _free_node(n);
}

/* ===================== Public API ===================== */

Treap* treap_create(void) {
    Treap *t = (Treap *)calloc(1, sizeof(Treap));
    return t;
}

void treap_insert(Treap *treap, int key, const char *value) {
    if (!treap) return;
    treap->root = _insert(treap->root, key, value,
                          &treap->node_count, &treap->traversal_count);
}

char* treap_search(Treap *treap, int key, int *traversals) {
    if (!treap) return NULL;
    TreapNode *n = _search(treap->root, key, traversals);
    return n ? n->value : NULL;
}

int treap_delete(Treap *treap, int key) {
    if (!treap) return 0;
    int found = 0;
    treap->root = _delete(treap->root, key, &treap->node_count, &found);
    return found;
}

void treap_print(Treap *treap) {
    if (!treap || !treap->root) {
        printf("(empty treap)\n");
        return;
    }
    printf("Treap (in-order, nodes=%d):\n", treap->node_count);
    _print_inorder(treap->root);
}

void treap_destroy(Treap *treap) {
    if (!treap) return;
    _destroy_nodes(treap->root);
    free(treap);
}

TreapRangeResult* treap_range_query(Treap *treap, int start, int end, int *traversals) {
    TreapRangeResult *res = (TreapRangeResult *)malloc(sizeof(TreapRangeResult));
    if (!res) return NULL;

    res->capacity = 16;
    res->count    = 0;
    res->keys     = (int *)  malloc(res->capacity * sizeof(int));
    res->values   = (char **)malloc(res->capacity * sizeof(char *));

    if (!res->keys || !res->values) {
        free(res->keys); free(res->values); free(res);
        return NULL;
    }

    if (treap) _range_collect(treap->root, start, end, res, traversals);
    return res;
}

void treap_range_result_free(TreapRangeResult *result) {
    if (!result) return;
    free(result->keys);
    free(result->values);
    free(result);
}

int treap_height(Treap *treap) {
    return treap ? _height(treap->root) : 0;
}

int treap_node_count(Treap *treap) {
    return treap ? treap->node_count : 0;
}
