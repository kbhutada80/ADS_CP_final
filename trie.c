#define _POSIX_C_SOURCE 200809L
#include "trie.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Trie (Prefix Tree) — Full Implementation
 *
 * Operations implemented:
 *   1. Insertion        — O(m) per key
 *   2. Search           — O(m) exact match
 *   3. Deletion         — O(m) with bottom-up pruning
 *   4. Prefix Search    — O(p + k*m) returns all matching words
 *   5. DFS Traversal    — lexicographic order
 *   6. BFS Traversal    — level-by-level (shortest words first)
 *   7. Complexity Table — printed summary
 *   8. Stats            — word count, depth, avg length, op counters
 */

/* ============================================================
 * Internal Node Helpers
 * ============================================================ */

static TrieNode* _new_node(int depth) {
    TrieNode *n = (TrieNode *)calloc(1, sizeof(TrieNode));
    if (n) n->depth = depth;
    return n;
}

static void _free_node(TrieNode *n) {
    if (!n) return;
    free(n->value);
    free(n);
}

/* ============================================================
 * 1. CREATE
 * ============================================================ */

Trie* trie_create(void) {
    Trie *t = (Trie *)calloc(1, sizeof(Trie));
    if (!t) return NULL;
    t->root = _new_node(0);
    if (!t->root) { free(t); return NULL; }
    t->node_count = 1;
    return t;
}

/* ============================================================
 * 2. INSERT  —  O(m) where m = strlen(key)
 *
 * Walk the trie character by character.
 * Create a new node for each missing character.
 * Mark the final node as end-of-word and store the value.
 * ============================================================ */

void trie_insert(Trie *trie, const char *key, const char *value) {
    if (!trie || !key) return;

    TrieNode *cur = trie->root;
    int depth = 0;

    for (int i = 0; key[i] != '\0'; i++) {
        unsigned char c = (unsigned char)key[i];
        if (c >= TRIE_ALPHABET_SIZE) continue;

        trie->insert_char_ops++;   /* Track per-character work */

        if (!cur->children[c]) {
            cur->children[c] = _new_node(depth + 1);
            if (!cur->children[c]) return;
            cur->char_count++;
            trie->node_count++;
        }
        cur = cur->children[c];
        depth++;
    }

    if (!cur->is_end) {
        cur->is_end = 1;
        trie->word_count++;
    }
    free(cur->value);
    cur->value = value ? strdup(value) : NULL;
}

/* ============================================================
 * 3. SEARCH  —  O(m)
 *
 * Walk trie character by character.
 * Return the value at the end node if it is marked is_end.
 * Return NULL if any character is missing or node is not end.
 * ============================================================ */

char* trie_search(Trie *trie, const char *key) {
    if (!trie || !key) return NULL;

    TrieNode *cur = trie->root;
    for (int i = 0; key[i] != '\0'; i++) {
        unsigned char c = (unsigned char)key[i];
        trie->search_char_ops++;
        if (c >= TRIE_ALPHABET_SIZE || !cur->children[c]) return NULL;
        cur = cur->children[c];
    }

    return cur->is_end ? cur->value : NULL;
}

/* ============================================================
 * PREFIX EXISTS CHECK  —  O(p)
 * ============================================================ */

int trie_starts_with(Trie *trie, const char *prefix) {
    if (!trie || !prefix) return 0;

    TrieNode *cur = trie->root;
    for (int i = 0; prefix[i] != '\0'; i++) {
        unsigned char c = (unsigned char)prefix[i];
        if (c >= TRIE_ALPHABET_SIZE || !cur->children[c]) return 0;
        cur = cur->children[c];
    }
    return 1;
}

/* ============================================================
 * 4. DELETION  —  O(m)
 *
 * Recursive approach:
 *   - Walk down to the target node.
 *   - On the way back up, unmark is_end and prune leaf nodes
 *     that are no longer needed (bottom-up cleanup).
 * ============================================================ */

static int _is_leaf(TrieNode *n) {
    for (int i = 0; i < TRIE_ALPHABET_SIZE; i++)
        if (n->children[i]) return 0;
    return 1;
}

/* Returns 1 if the caller should delete the pointer to this node */
static int _delete_rec(TrieNode *cur, const char *key, int depth,
                        int *word_count, int *node_count,
                        long long *char_ops) {
    if (!cur) return 0;

    if (key[depth] == '\0') {
        /* Reached end of key */
        if (!cur->is_end) return 0;  /* Key not in trie */
        cur->is_end = 0;
        free(cur->value);
        cur->value = NULL;
        (*word_count)--;
        /* Only prune this node if it has no children */
        return _is_leaf(cur);
    }

    unsigned char c = (unsigned char)key[depth];
    if (c >= TRIE_ALPHABET_SIZE || !cur->children[c]) return 0;

    if (char_ops) (*char_ops)++;

    int should_delete = _delete_rec(cur->children[c], key, depth + 1,
                                    word_count, node_count, char_ops);

    if (should_delete) {
        _free_node(cur->children[c]);
        cur->children[c] = NULL;
        cur->char_count--;
        (*node_count)--;
        /* Prune current node only if it is now a non-end leaf */
        return (!cur->is_end && _is_leaf(cur));
    }

    return 0;
}

int trie_delete(Trie *trie, const char *key) {
    if (!trie || !key) return 0;
    int before = trie->word_count;
    _delete_rec(trie->root, key, 0,
                &trie->word_count, &trie->node_count,
                &trie->delete_char_ops);
    return (trie->word_count < before) ? 1 : 0;
}

/* ============================================================
 * 5. PREFIX SEARCH  —  O(p + k*m)
 *
 * Navigate to the prefix node (O(p)).
 * DFS from that node collecting all complete words (O(k*m)).
 * ============================================================ */

static void _collect_words(TrieNode *node, char *buf, int depth,
                            TriePrefixResult *res,
                            long long *char_ops) {
    if (!node) return;

    if (node->is_end) {
        buf[depth] = '\0';
        if (res->count == res->capacity) {
            res->capacity *= 2;
            res->words  = (char **)realloc(res->words,  res->capacity * sizeof(char *));
            res->values = (char **)realloc(res->values, res->capacity * sizeof(char *));
        }
        res->words[res->count]  = strdup(buf);
        res->values[res->count] = node->value ? strdup(node->value) : NULL;
        res->count++;
    }

    for (int c = 0; c < TRIE_ALPHABET_SIZE; c++) {
        if (node->children[c]) {
            if (char_ops) (*char_ops)++;
            buf[depth] = (char)c;
            _collect_words(node->children[c], buf, depth + 1, res, char_ops);
        }
    }
}

TriePrefixResult* trie_prefix_search(Trie *trie, const char *prefix) {
    TriePrefixResult *res = (TriePrefixResult *)malloc(sizeof(TriePrefixResult));
    if (!res) return NULL;

    res->capacity = 16;
    res->count    = 0;
    res->words    = (char **)malloc(res->capacity * sizeof(char *));
    res->values   = (char **)malloc(res->capacity * sizeof(char *));

    if (!res->words || !res->values) {
        free(res->words); free(res->values); free(res);
        return NULL;
    }

    if (!trie || !prefix) return res;

    /* Navigate to the prefix node — O(p) */
    TrieNode *cur = trie->root;
    for (int i = 0; prefix[i] != '\0'; i++) {
        unsigned char c = (unsigned char)prefix[i];
        trie->prefix_char_ops++;
        if (c >= TRIE_ALPHABET_SIZE || !cur->children[c]) return res;
        cur = cur->children[c];
    }

    /* DFS from prefix node — O(k*m) */
    char buf[TRIE_MAX_WORD_LEN];
    int plen = (int)strlen(prefix);
    memcpy(buf, prefix, plen);
    _collect_words(cur, buf, plen, res, &trie->prefix_char_ops);

    return res;
}

void trie_prefix_result_free(TriePrefixResult *result) {
    if (!result) return;
    for (int i = 0; i < result->count; i++) {
        free(result->words[i]);
        free(result->values[i]);
    }
    free(result->words);
    free(result->values);
    free(result);
}

/* ============================================================
 * 6a. DFS TRAVERSAL  —  O(ALPHA * n) time, O(m) stack
 *
 * Recursively visits all nodes depth-first.
 * Yields words in lexicographic (alphabetical) order because
 * children array is indexed by ASCII value (a < b < c ...).
 * ============================================================ */

static void _dfs_collect(TrieNode *node, char *buf, int depth,
                          TrieTraversalResult *res) {
    if (!node) return;

    res->nodes_visited++;

    if (node->is_end) {
        buf[depth] = '\0';
        if (res->count == res->capacity) {
            res->capacity *= 2;
            res->words  = (char **)realloc(res->words,  res->capacity * sizeof(char *));
            res->depths = (int *)  realloc(res->depths, res->capacity * sizeof(int));
        }
        res->words[res->count]  = strdup(buf);
        res->depths[res->count] = depth;
        res->count++;
    }

    for (int c = 0; c < TRIE_ALPHABET_SIZE; c++) {
        if (node->children[c]) {
            buf[depth] = (char)c;
            _dfs_collect(node->children[c], buf, depth + 1, res);
        }
    }
}

TrieTraversalResult* trie_traversal_dfs(Trie *trie) {
    TrieTraversalResult *res = (TrieTraversalResult *)malloc(sizeof(TrieTraversalResult));
    if (!res) return NULL;

    res->capacity      = 16;
    res->count         = 0;
    res->nodes_visited = 0;
    res->words  = (char **)malloc(res->capacity * sizeof(char *));
    res->depths = (int *)  malloc(res->capacity * sizeof(int));

    if (!res->words || !res->depths) {
        free(res->words); free(res->depths); free(res);
        return NULL;
    }

    if (trie && trie->root) {
        char buf[TRIE_MAX_WORD_LEN];
        _dfs_collect(trie->root, buf, 0, res);
    }

    return res;
}

/* ============================================================
 * 6b. BFS TRAVERSAL  —  O(ALPHA * n) time, O(n) queue
 *
 * Uses a queue of (node, buffer, depth) tuples.
 * Visits nodes level by level → shortest words come first.
 * ============================================================ */

/* BFS queue entry */
typedef struct {
    TrieNode *node;
    char      buf[TRIE_MAX_WORD_LEN];
    int       depth;
} BFSEntry;

TrieTraversalResult* trie_traversal_bfs(Trie *trie) {
    TrieTraversalResult *res = (TrieTraversalResult *)malloc(sizeof(TrieTraversalResult));
    if (!res) return NULL;

    res->capacity      = 16;
    res->count         = 0;
    res->nodes_visited = 0;
    res->words  = (char **)malloc(res->capacity * sizeof(char *));
    res->depths = (int *)  malloc(res->capacity * sizeof(int));

    if (!res->words || !res->depths) {
        free(res->words); free(res->depths); free(res);
        return NULL;
    }

    if (!trie || !trie->root) return res;

    /* Allocate BFS queue — max size = total nodes */
    int qcap = trie->node_count + 1;
    BFSEntry *queue = (BFSEntry *)malloc(qcap * sizeof(BFSEntry));
    if (!queue) { free(res->words); free(res->depths); free(res); return NULL; }

    int head = 0, tail = 0;

    /* Enqueue root */
    queue[tail].node  = trie->root;
    queue[tail].depth = 0;
    queue[tail].buf[0] = '\0';
    tail++;

    while (head < tail) {
        BFSEntry cur = queue[head++];
        res->nodes_visited++;

        if (cur.node->is_end) {
            cur.buf[cur.depth] = '\0';
            if (res->count == res->capacity) {
                res->capacity *= 2;
                res->words  = (char **)realloc(res->words,  res->capacity * sizeof(char *));
                res->depths = (int *)  realloc(res->depths, res->capacity * sizeof(int));
            }
            res->words[res->count]  = strdup(cur.buf);
            res->depths[res->count] = cur.depth;
            res->count++;
        }

        for (int c = 0; c < TRIE_ALPHABET_SIZE; c++) {
            if (cur.node->children[c]) {
                /* Grow queue if needed */
                if (tail >= qcap) {
                    qcap *= 2;
                    queue = (BFSEntry *)realloc(queue, qcap * sizeof(BFSEntry));
                    if (!queue) goto bfs_done;
                }
                queue[tail] = cur;
                queue[tail].node           = cur.node->children[c];
                queue[tail].buf[cur.depth] = (char)c;
                queue[tail].depth          = cur.depth + 1;
                tail++;
            }
        }
    }

    bfs_done:
    free(queue);
    return res;
}

void trie_traversal_result_free(TrieTraversalResult *result) {
    if (!result) return;
    for (int i = 0; i < result->count; i++)
        free(result->words[i]);
    free(result->words);
    free(result->depths);
    free(result);
}

/* ============================================================
 * 7. COMPLEXITY TABLE
 * ============================================================ */

void trie_print_complexity(void) {
    printf("\n");
    printf("+--------------------------------------------------------------------+\n");
    printf("|              TRIE — TIME & SPACE COMPLEXITY                       |\n");
    printf("+--------------------------------------------------------------------+\n");
    printf("|  Variables:                                                        |\n");
    printf("|    m = length of the key/word being processed                     |\n");
    printf("|    p = length of the prefix                                        |\n");
    printf("|    k = number of words matching the prefix                        |\n");
    printf("|    n = total number of nodes in the trie                          |\n");
    printf("|    A = alphabet size (128 for full ASCII)                         |\n");
    printf("+------------------+----------------+----------------+--------------+\n");
    printf("|  Operation       | Best Case      | Avg Case       | Worst Case   |\n");
    printf("+------------------+----------------+----------------+--------------+\n");
    printf("|  Insert          | O(1)*          | O(m)           | O(m)         |\n");
    printf("|  Search          | O(1)*          | O(m)           | O(m)         |\n");
    printf("|  Delete          | O(1)*          | O(m)           | O(m)         |\n");
    printf("|  Prefix Exists   | O(1)*          | O(p)           | O(p)         |\n");
    printf("|  Prefix Search   | O(p)           | O(p + k*m)     | O(p + n)     |\n");
    printf("|  DFS Traversal   | O(A*n)         | O(A*n)         | O(A*n)       |\n");
    printf("|  BFS Traversal   | O(A*n)         | O(A*n)         | O(A*n)       |\n");
    printf("+------------------+----------------+----------------+--------------+\n");
    printf("|  Space (trie)    | O(A*n)         — shared prefixes save memory   |\n");
    printf("|  Space (result)  | O(k*m)         — for prefix search output      |\n");
    printf("+--------------------------------------------------------------------+\n");
    printf("| * Best case: key matches root or very short path (early exit)     |\n");
    printf("| All operations are O(m) — INDEPENDENT of total word count!        |\n");
    printf("| This beats hash maps for prefix queries and BSTs for ordered ops. |\n");
    printf("+--------------------------------------------------------------------+\n\n");

    printf("|  COMPARISON vs OTHER STRUCTURES:                                  |\n");
    printf("+-------------------+----------+----------+----------+----------+\n");
    printf("| Op                | Trie     | HashTable| BST      | SortedArr|\n");
    printf("+-------------------+----------+----------+----------+----------+\n");
    printf("| Exact Search      | O(m)     | O(m)*    | O(m logN)| O(m logN)|\n");
    printf("| Insert            | O(m)     | O(m)*    | O(m logN)| O(N)     |\n");
    printf("| Delete            | O(m)     | O(m)*    | O(m logN)| O(N)     |\n");
    printf("| Prefix Search     | O(p+k*m) | O(N*m)   | O(N*m)   | O(logN+k)|\n");
    printf("| Sorted Traversal  | O(A*n)   | O(N logN)| O(N)     | O(N)     |\n");
    printf("+-------------------+----------+----------+----------+----------+\n");
    printf("| * Hash collision can degrade to O(N) in worst case             |\n\n");
}

/* ============================================================
 * 8. STATS
 * ============================================================ */

static void _compute_stats(TrieNode *node, int depth,
                            int *max_depth, int *total_chars,
                            int *word_count) {
    if (!node) return;
    if (node->is_end) {
        (*word_count)++;
        (*total_chars) += depth;
        if (depth > *max_depth) *max_depth = depth;
    }
    for (int c = 0; c < TRIE_ALPHABET_SIZE; c++)
        if (node->children[c])
            _compute_stats(node->children[c], depth + 1,
                           max_depth, total_chars, word_count);
}

TrieStats trie_get_stats(Trie *trie) {
    TrieStats s = {0};
    if (!trie || !trie->root) return s;

    _compute_stats(trie->root, 0, &s.max_depth, &s.total_chars, &s.word_count);
    s.node_count  = trie->node_count;
    s.avg_word_len = s.word_count > 0
                     ? (double)s.total_chars / s.word_count
                     : 0.0;
    return s;
}

void trie_print_stats(Trie *trie) {
    if (!trie) return;

    TrieStats s = trie_get_stats(trie);

    printf("\n+--------------------------------------------+\n");
    printf("|           TRIE STATISTICS                  |\n");
    printf("+--------------------------------------------+\n");
    printf("|  Words stored:        %-20d|\n", s.word_count);
    printf("|  Total nodes:         %-20d|\n", s.node_count);
    printf("|  Max word depth:      %-20d|\n", s.max_depth);
    printf("|  Avg word length:     %-20.2f|\n", s.avg_word_len);
    printf("+--------------------------------------------+\n");
    printf("|  Operation Counters (character steps):     |\n");
    printf("|    Insert ops:        %-20ld|\n", (long)trie->insert_char_ops);
    printf("|    Search ops:        %-20ld|\n", (long)trie->search_char_ops);
    printf("|    Delete ops:        %-20ld|\n", (long)trie->delete_char_ops);
    printf("|    Prefix ops:        %-20ld|\n", (long)trie->prefix_char_ops);
    printf("+--------------------------------------------+\n\n");
}

/* ============================================================
 * Print (all words, lexicographic order via DFS)
 * ============================================================ */

static void _print_all(TrieNode *node, char *buf, int depth) {
    if (!node) return;

    if (node->is_end) {
        buf[depth] = '\0';
        printf("  \"%s\"", buf);
        if (node->value) printf(" => %s", node->value);
        printf("\n");
    }

    for (int c = 0; c < TRIE_ALPHABET_SIZE; c++) {
        if (node->children[c]) {
            buf[depth] = (char)c;
            _print_all(node->children[c], buf, depth + 1);
        }
    }
}

void trie_print(Trie *trie) {
    if (!trie || !trie->root) { printf("(empty trie)\n"); return; }
    printf("Trie (words=%d, nodes=%d) — lexicographic order:\n",
           trie->word_count, trie->node_count);
    char buf[TRIE_MAX_WORD_LEN];
    _print_all(trie->root, buf, 0);
}

/* ============================================================
 * Destroy
 * ============================================================ */

static void _destroy_nodes(TrieNode *n) {
    if (!n) return;
    for (int i = 0; i < TRIE_ALPHABET_SIZE; i++)
        _destroy_nodes(n->children[i]);
    _free_node(n);
}

void trie_destroy(Trie *trie) {
    if (!trie) return;
    _destroy_nodes(trie->root);
    free(trie);
}

int trie_word_count(Trie *trie) { return trie ? trie->word_count : 0; }
int trie_node_count(Trie *trie) { return trie ? trie->node_count : 0; }
