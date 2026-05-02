#ifndef TREAP_H
#define TREAP_H

/*
 * Treap Header File
 *
 * A randomized data structure that combines a Binary Search Tree (BST)
 * with a Heap. Each node has a key (BST property) and a random priority
 * (Max-Heap property). This guarantees expected O(log n) height.
 *
 * Key Properties:
 * - BST property on keys for ordered operations
 * - Heap property on priorities (randomly assigned) for balance
 * - Expected O(log n) height with high probability
 * - No deterministic worst case — adversary cannot force O(n)
 *
 * Time Complexities (expected):
 *   Insert:      O(log n)
 *   Search:      O(log n)
 *   Delete:      O(log n)
 *   Range Query: O(log n + k)
 *
 * Space Complexity: O(n)
 */

typedef struct TreapNode {
    int key;
    int priority;           /* Random heap priority */
    char *value;
    struct TreapNode *left;
    struct TreapNode *right;
} TreapNode;

typedef struct {
    TreapNode *root;
    int node_count;
    int traversal_count;    /* Track traversals for benchmarking */
} Treap;

/* Core Treap Operations */
Treap* treap_create(void);
void treap_insert(Treap *treap, int key, const char *value);
char* treap_search(Treap *treap, int key, int *traversals);
int treap_delete(Treap *treap, int key);
void treap_print(Treap *treap);
void treap_destroy(Treap *treap);

/* Range Query */
typedef struct {
    int *keys;
    char **values;
    int count;
    int capacity;
} TreapRangeResult;

TreapRangeResult* treap_range_query(Treap *treap, int start, int end, int *traversals);
void treap_range_result_free(TreapRangeResult *result);

/* Utility Functions */
int treap_height(Treap *treap);
int treap_node_count(Treap *treap);

#endif /* TREAP_H */
