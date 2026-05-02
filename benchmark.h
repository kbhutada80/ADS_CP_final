#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "bptree.h"
#include "skiplist.h"
#include "avl.h"
#include "bst.h"
#include "splay.h"
#include "treap.h"

/*
 * Benchmark Module Header File
 *
 * Comprehensive performance analysis and comparison across all six
 * data structures: B+ Tree, Skip List, AVL Tree, BST, Splay Tree, Treap.
 *
 * Metrics Collected:
 * - Insert time (ms)
 * - Search time (ms)
 * - Range query time (ms)
 * - Delete time (ms)
 * - Tree/structure height
 * - Number of node traversals
 *
 * Output:
 * - Structured comparison tables
 * - CSV files for graph generation
 * - Automated observations/insights
 */

#define NUM_STRUCTURES      6
#define MAX_BENCHMARK_SIZES 6

/* Results for a single data structure at one dataset size */
typedef struct {
    double insert_time_ms;
    double search_time_ms;
    double delete_time_ms;
    double range_time_ms;
    int node_traversals;
    int height;
    int node_count;
    const char *name;
} BenchmarkResult;

/* Full comparison at one dataset size */
typedef struct {
    BenchmarkResult results[NUM_STRUCTURES];  /* B+Tree, SkipList, AVL, BST, Splay, Treap */
    int dataset_size;
    int range_size;
    int is_sorted;                            /* 1 if sorted input, 0 if random */
} ComparisonResult;

/* Multi-size benchmark results */
typedef struct {
    ComparisonResult *comparisons;
    int num_sizes;
    int *sizes;
    int is_sorted;
} MultiBenchmarkResult;

/* ====== Core Benchmark Functions ====== */

/* Benchmark individual structures */
void benchmark_bptree(BPTree *tree, int *dataset, int dataset_size, BenchmarkResult *result);
void benchmark_skiplist(SkipList *list, int *dataset, int dataset_size, BenchmarkResult *result);
void benchmark_avl(AVLTree *tree, int *dataset, int dataset_size, BenchmarkResult *result);
void benchmark_bst(BSTree *tree, int *dataset, int dataset_size, BenchmarkResult *result);
void benchmark_splay(SplayTree *tree, int *dataset, int dataset_size, BenchmarkResult *result);
void benchmark_treap(Treap *treap, int *dataset, int dataset_size, BenchmarkResult *result);

/* Trie benchmark (string keys — separate from int-key benchmarks) */
void benchmark_trie(int num_words);

/* Export Trie benchmark across multiple sizes to CSV */
void export_trie_csv(int *sizes, int num_sizes, const char *filename);

/* Run full comparison at one size */
ComparisonResult* run_full_benchmark(int dataset_size, int range_size);

/* Run benchmark across multiple sizes */
MultiBenchmarkResult* run_multi_benchmark(int *sizes, int num_sizes, int sorted);

/* ====== Output Functions ====== */

/* Print formatted tables */
void print_benchmark_results(ComparisonResult *results);
void print_comparison_table(ComparisonResult *results);

/* Print multi-size results */
void print_multi_results(MultiBenchmarkResult *mbr);

/* Generate CSV output for graph plotting */
void export_csv(MultiBenchmarkResult *mbr, const char *filename);

/* ====== Observation Engine ====== */

/* Analyze results and print insights */
void print_observations(MultiBenchmarkResult *mbr);

/* ====== Theoretical Analysis ====== */

/* Print theoretical complexity table */
void print_theoretical_analysis(void);

/* ====== Cleanup ====== */
void benchmark_cleanup(ComparisonResult *results);
void multi_benchmark_cleanup(MultiBenchmarkResult *mbr);

#endif /* BENCHMARK_H */
