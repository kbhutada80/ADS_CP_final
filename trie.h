#ifndef TRIE_H
#define TRIE_H

/*
 * Trie (Prefix Tree) Header File
 *
 * A tree data structure where each node represents one character.
 * Keys are distributed along edges (paths from root to leaf).
 *
 * ================================================================
 *  COMPLEXITY SUMMARY
 * ================================================================
 *  Operation       | Time           | Space
 * -----------------|----------------|-----------------------------
 *  Insert          | O(m)           | O(m) new nodes worst case
 *  Search          | O(m)           | O(1)
 *  Delete          | O(m)           | O(1) (frees nodes bottom-up)
 *  Prefix Search   | O(p + k*m)     | O(k*m) for result
 *  Traversal (DFS) | O(ALPHA * n)   | O(m) stack depth
 *  Traversal (BFS) | O(ALPHA * n)   | O(n) queue
 *
 *  m = key length, p = prefix length, k = number of matching words
 *  n = total number of nodes, ALPHA = alphabet size (128)
 * ================================================================
 *
 * Space Complexity: O(ALPHABET_SIZE * n)  worst case
 *                   Much better in practice due to shared prefixes
 */

#define TRIE_ALPHABET_SIZE 128    /* Full ASCII (printable + control) */
#define TRIE_MAX_WORD_LEN  512    /* Maximum supported key length */

/* ===================== Node Structure ===================== */

typedef struct TrieNode {
    struct TrieNode *children[TRIE_ALPHABET_SIZE];
    int   is_end;        /* 1 if this node marks end of a complete word */
    char *value;         /* Optional value stored at end-of-word node */
    int   char_count;    /* Number of non-NULL children (branch degree) */
    int   depth;         /* Depth of this node from root (= char index) */
} TrieNode;

/* ===================== Trie Stats ===================== */

typedef struct {
    int word_count;       /* Total complete words stored */
    int node_count;       /* Total nodes (including root) */
    int max_depth;        /* Depth of deepest word */
    int total_chars;      /* Sum of lengths of all stored words */
    double avg_word_len;  /* Average word length */
} TrieStats;

/* ===================== Trie Structure ===================== */

typedef struct {
    TrieNode *root;
    int word_count;
    int node_count;
    /* Metrics tracked during operations */
    long long insert_char_ops;   /* Total character steps in all inserts */
    long long search_char_ops;   /* Total character steps in all searches */
    long long delete_char_ops;   /* Total character steps in all deletes */
    long long prefix_char_ops;   /* Total character steps in all prefix searches */
} Trie;

/* ===================== Result Types ===================== */

/* Result for prefix search — list of matching words */
typedef struct {
    char **words;
    char **values;
    int    count;
    int    capacity;
} TriePrefixResult;

/* Result for BFS traversal — level-by-level order */
typedef struct {
    char **words;          /* All complete words found during traversal */
    int   *depths;         /* Depth at which each word ends */
    int    count;
    int    capacity;
    int    nodes_visited;  /* Total nodes visited (including non-end) */
} TrieTraversalResult;

/* ===================== Core Operations ===================== */

Trie* trie_create(void);

/* Insert key with optional value. Overwrites value if key exists. O(m) */
void trie_insert(Trie *trie, const char *key, const char *value);

/* Search exact key. Returns value if found, NULL otherwise. O(m) */
char* trie_search(Trie *trie, const char *key);

/* Delete key. Returns 1 if deleted, 0 if not found. O(m) */
int trie_delete(Trie *trie, const char *key);

/* Returns 1 if any word starts with prefix, 0 otherwise. O(p) */
int trie_starts_with(Trie *trie, const char *prefix);

/* Print all words in lexicographic order */
void trie_print(Trie *trie);

/* Destroy the trie and free all memory */
void trie_destroy(Trie *trie);

/* ===================== Prefix Search ===================== */

/* Find all words that start with the given prefix. O(p + k*m) */
TriePrefixResult* trie_prefix_search(Trie *trie, const char *prefix);
void trie_prefix_result_free(TriePrefixResult *result);

/* ===================== Traversal ===================== */

/* DFS traversal — collects words in lexicographic (alphabetical) order */
TrieTraversalResult* trie_traversal_dfs(Trie *trie);

/* BFS traversal — collects words level by level (shortest words first) */
TrieTraversalResult* trie_traversal_bfs(Trie *trie);

void trie_traversal_result_free(TrieTraversalResult *result);

/* ===================== Complexity / Stats ===================== */

/* Print a formatted complexity table for Trie operations */
void trie_print_complexity(void);

/* Compute and return stats about the current trie */
TrieStats trie_get_stats(Trie *trie);

/* Print formatted stats and operation counters */
void trie_print_stats(Trie *trie);

/* ===================== Utility ===================== */

int trie_word_count(Trie *trie);
int trie_node_count(Trie *trie);

#endif /* TRIE_H */
