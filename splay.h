#ifndef SPLAY_H
#define SPLAY_H

/*
 * Splay Tree Header File
 *
 * A self-adjusting binary search tree where recently accessed elements
 * are moved to the root via splay rotations (zig, zig-zig, zig-zag).
 *
 * Key Properties:
 * - Amortized O(log n) for insert, search, delete
 * - O(1) access to the most recently accessed element
 * - No extra balance information stored per node
 * - Excellent temporal locality performance
 *
 * Time Complexities (amortized):
 *   Insert:      O(log n)
 *   Search:      O(log n)
 *   Delete:      O(log n)
 *   Range Query: O(log n + k)
 *
 * Space Complexity: O(n)
 */

typedef struct SplayNode {
    int key;
    char *value;
    struct SplayNode *left;
    struct SplayNode *right;
} SplayNode;

typedef struct {
    SplayNode *root;
    int node_count;
    int traversal_count;   /* Track traversals for benchmarking */
    int splay_count;       /* Number of splay operations performed */
} SplayTree;

/* Core Splay Tree Operations */
SplayTree* splay_create(void);
void splay_insert(SplayTree *tree, int key, const char *value);
char* splay_search(SplayTree *tree, int key, int *traversals);
int splay_delete(SplayTree *tree, int key);
void splay_print(SplayTree *tree);
void splay_destroy(SplayTree *tree);

/* Range Query */
typedef struct {
    int *keys;
    char **values;
    int count;
    int capacity;
} SplayRangeResult;

SplayRangeResult* splay_range_query(SplayTree *tree, int start, int end, int *traversals);
void splay_range_result_free(SplayRangeResult *result);

/* Utility Functions */
int splay_height(SplayTree *tree);
int splay_node_count(SplayTree *tree);

#endif /* SPLAY_H */
