#include "benchmark.h"
#include "avl.h"
#include "bst.h"
#include "splay.h"
#include "treap.h"
#include "trie.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ===================== Utility ===================== */

static void _shuffle(int *arr, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
    }
}

static int* _generate_dataset(int size, int sorted) {
    int *data = (int *)malloc(size * sizeof(int));
    if (!data) return NULL;
    for (int i = 0; i < size; i++) data[i] = i + 1;
    if (!sorted) _shuffle(data, size);
    return data;
}

/* ===================== Individual Benchmarks ===================== */

void benchmark_bptree(BPTree *tree, int *dataset, int dataset_size, BenchmarkResult *result) {
    if (!tree || !dataset || !result) return;
    result->name = "B+ Tree";
    clock_t start, end;

    start = clock();
    for (int i = 0; i < dataset_size; i++) {
        char val[32]; snprintf(val, sizeof(val), "v%d", dataset[i]);
        bptree_insert(tree, dataset[i], val);
    }
    end = clock();
    result->insert_time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

    int total_trav = 0;
    start = clock();
    for (int i = 0; i < dataset_size; i++) {
        int dr = 0; bptree_search(tree, dataset[i], &dr); total_trav += dr;
    }
    end = clock();
    result->search_time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    result->node_traversals = total_trav;

    int range_start = dataset_size / 4;
    int range_end   = range_start + dataset_size / 10;
    if (range_end > dataset_size) range_end = dataset_size;

    start = clock();
    BPTreeLeafNode *leaf = tree->leftmost_leaf;
    int rc = 0;
    while (leaf) {
        for (int i = 0; i < leaf->key_count; i++) {
            if (leaf->keys[i] >= range_start && leaf->keys[i] <= range_end) rc++;
            if (leaf->keys[i] > range_end) goto bpt_done;
        }
        leaf = leaf->next;
    }
    bpt_done:
    end = clock();
    result->range_time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    (void)rc;
    result->height     = bptree_height(tree);
    result->node_count = bptree_node_count(tree);
}

void benchmark_skiplist(SkipList *list, int *dataset, int dataset_size, BenchmarkResult *result) {
    if (!list || !dataset || !result) return;
    result->name = "Skip List";
    clock_t start, end;

    start = clock();
    for (int i = 0; i < dataset_size; i++) {
        char val[32]; snprintf(val, sizeof(val), "v%d", dataset[i]);
        skiplist_insert(list, dataset[i], val);
    }
    end = clock();
    result->insert_time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

    int total_trav = 0;
    start = clock();
    for (int i = 0; i < dataset_size; i++) {
        int acc = 0; skiplist_search(list, dataset[i], &acc); total_trav += acc;
    }
    end = clock();
    result->search_time_ms  = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    result->node_traversals = total_trav;

    int range_start = dataset_size / 4;
    int range_end   = range_start + dataset_size / 10;
    start = clock();
    SkipListNode *node = list->header->forward[0];
    int rc = 0;
    while (node) {
        if (node->key >= range_start && node->key <= range_end) rc++;
        if (node->key > range_end) break;
        node = node->forward[0];
    }
    end = clock();
    result->range_time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    (void)rc;
    result->height     = list->level;
    result->node_count = skiplist_node_count(list);
}

void benchmark_avl(AVLTree *tree, int *dataset, int dataset_size, BenchmarkResult *result) {
    if (!tree || !dataset || !result) return;
    result->name = "AVL Tree";
    clock_t start, end;

    start = clock();
    for (int i = 0; i < dataset_size; i++) {
        char val[32]; snprintf(val, sizeof(val), "v%d", dataset[i]);
        avl_insert(tree, dataset[i], val);
    }
    end = clock();
    result->insert_time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

    int total_trav = 0;
    start = clock();
    for (int i = 0; i < dataset_size; i++) {
        int trav = 0; avl_search(tree, dataset[i], &trav); total_trav += trav;
    }
    end = clock();
    result->search_time_ms  = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    result->node_traversals = total_trav;

    int range_start = dataset_size / 4;
    int range_end   = range_start + dataset_size / 10;
    int range_trav  = 0;
    start = clock();
    AVLRangeResult *rr = avl_range_query(tree, range_start, range_end, &range_trav);
    end = clock();
    result->range_time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    if (rr) avl_range_result_free(rr);
    result->height     = avl_height(tree);
    result->node_count = avl_node_count(tree);
}

void benchmark_bst(BSTree *tree, int *dataset, int dataset_size, BenchmarkResult *result) {
    if (!tree || !dataset || !result) return;
    result->name = "BST";
    clock_t start, end;

    start = clock();
    for (int i = 0; i < dataset_size; i++) {
        char val[32]; snprintf(val, sizeof(val), "v%d", dataset[i]);
        bst_insert(tree, dataset[i], val);
    }
    end = clock();
    result->insert_time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

    int total_trav = 0;
    start = clock();
    for (int i = 0; i < dataset_size; i++) {
        int trav = 0; bst_search(tree, dataset[i], &trav); total_trav += trav;
    }
    end = clock();
    result->search_time_ms  = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    result->node_traversals = total_trav;

    int range_start = dataset_size / 4;
    int range_end   = range_start + dataset_size / 10;
    int range_trav  = 0;
    start = clock();
    BSTRangeResult *rr = bst_range_query(tree, range_start, range_end, &range_trav);
    end = clock();
    result->range_time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    if (rr) bst_range_result_free(rr);
    result->height     = bst_height(tree);
    result->node_count = bst_node_count(tree);
}

void benchmark_splay(SplayTree *tree, int *dataset, int dataset_size, BenchmarkResult *result) {
    if (!tree || !dataset || !result) return;
    result->name = "Splay Tree";
    clock_t start, end;

    start = clock();
    for (int i = 0; i < dataset_size; i++) {
        char val[32]; snprintf(val, sizeof(val), "v%d", dataset[i]);
        splay_insert(tree, dataset[i], val);
    }
    end = clock();
    result->insert_time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

    int total_trav = 0;
    start = clock();
    for (int i = 0; i < dataset_size; i++) {
        int trav = 0; splay_search(tree, dataset[i], &trav); total_trav += trav;
    }
    end = clock();
    result->search_time_ms  = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    result->node_traversals = total_trav;

    int range_start = dataset_size / 4;
    int range_end   = range_start + dataset_size / 10;
    int range_trav  = 0;
    start = clock();
    SplayRangeResult *rr = splay_range_query(tree, range_start, range_end, &range_trav);
    end = clock();
    result->range_time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    if (rr) splay_range_result_free(rr);
    result->height     = splay_height(tree);
    result->node_count = splay_node_count(tree);
}

void benchmark_treap(Treap *treap, int *dataset, int dataset_size, BenchmarkResult *result) {
    if (!treap || !dataset || !result) return;
    result->name = "Treap";
    clock_t start, end;

    start = clock();
    for (int i = 0; i < dataset_size; i++) {
        char val[32]; snprintf(val, sizeof(val), "v%d", dataset[i]);
        treap_insert(treap, dataset[i], val);
    }
    end = clock();
    result->insert_time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

    int total_trav = 0;
    start = clock();
    for (int i = 0; i < dataset_size; i++) {
        int trav = 0; treap_search(treap, dataset[i], &trav); total_trav += trav;
    }
    end = clock();
    result->search_time_ms  = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    result->node_traversals = total_trav;

    int range_start = dataset_size / 4;
    int range_end   = range_start + dataset_size / 10;
    int range_trav  = 0;
    start = clock();
    TreapRangeResult *rr = treap_range_query(treap, range_start, range_end, &range_trav);
    end = clock();
    result->range_time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    if (rr) treap_range_result_free(rr);
    result->height     = treap_height(treap);
    result->node_count = treap_node_count(treap);
}

/* ===================== Full Benchmark ===================== */

ComparisonResult* run_full_benchmark(int dataset_size, int range_size) {
    printf("\n================================================================\n");
    printf("   BENCHMARK: All 6 Data Structures | N = %d\n", dataset_size);
    printf("================================================================\n");

    int *dataset = _generate_dataset(dataset_size, 0);
    if (!dataset) return NULL;

    ComparisonResult *cmp = (ComparisonResult *)calloc(1, sizeof(ComparisonResult));
    if (!cmp) { free(dataset); return NULL; }
    cmp->dataset_size = dataset_size;
    cmp->range_size   = range_size;
    cmp->is_sorted    = 0;

    BPTree    *bpt  = bptree_create(4);  benchmark_bptree(bpt, dataset, dataset_size, &cmp->results[0]);  bptree_destroy(bpt);
    SkipList  *sl   = skiplist_create(); benchmark_skiplist(sl, dataset, dataset_size, &cmp->results[1]); skiplist_destroy(sl);
    AVLTree   *avl  = avl_create();      benchmark_avl(avl, dataset, dataset_size, &cmp->results[2]);    avl_destroy(avl);
    BSTree    *bst  = bst_create();      benchmark_bst(bst, dataset, dataset_size, &cmp->results[3]);    bst_destroy(bst);
    SplayTree *spt  = splay_create();    benchmark_splay(spt, dataset, dataset_size, &cmp->results[4]);  splay_destroy(spt);
    Treap     *trp  = treap_create();    benchmark_treap(trp, dataset, dataset_size, &cmp->results[5]);  treap_destroy(trp);

    free(dataset);
    return cmp;
}

MultiBenchmarkResult* run_multi_benchmark(int *sizes, int num_sizes, int sorted) {
    MultiBenchmarkResult *mbr = (MultiBenchmarkResult *)malloc(sizeof(MultiBenchmarkResult));
    if (!mbr) return NULL;

    mbr->num_sizes   = num_sizes;
    mbr->is_sorted   = sorted;
    mbr->sizes       = (int *)malloc(num_sizes * sizeof(int));
    mbr->comparisons = (ComparisonResult *)calloc(num_sizes, sizeof(ComparisonResult));

    if (!mbr->sizes || !mbr->comparisons) {
        free(mbr->sizes); free(mbr->comparisons); free(mbr); return NULL;
    }
    memcpy(mbr->sizes, sizes, num_sizes * sizeof(int));

    printf("\n+---------------------------------------------------------------+\n");
    printf("|         MULTI-SIZE BENCHMARK (%s input)               |\n",
           sorted ? "SORTED" : "RANDOM");
    printf("|         Testing sizes:");
    for (int i = 0; i < num_sizes; i++) printf(" %d", sizes[i]);
    printf("\n+---------------------------------------------------------------+\n");

    for (int s = 0; s < num_sizes; s++) {
        int *dataset = _generate_dataset(sizes[s], sorted);
        if (!dataset) continue;

        ComparisonResult *cmp = &mbr->comparisons[s];
        cmp->dataset_size = sizes[s];
        cmp->range_size   = sizes[s] / 10;
        cmp->is_sorted    = sorted;

        printf("\n--- Dataset Size: %d ---\n", sizes[s]);

        BPTree    *bpt = bptree_create(4);  benchmark_bptree(bpt, dataset, sizes[s], &cmp->results[0]);  bptree_destroy(bpt);
        SkipList  *sl  = skiplist_create(); benchmark_skiplist(sl, dataset, sizes[s], &cmp->results[1]); skiplist_destroy(sl);
        AVLTree   *avl = avl_create();      benchmark_avl(avl, dataset, sizes[s], &cmp->results[2]);    avl_destroy(avl);
        BSTree    *bst = bst_create();      benchmark_bst(bst, dataset, sizes[s], &cmp->results[3]);    bst_destroy(bst);
        SplayTree *spt = splay_create();    benchmark_splay(spt, dataset, sizes[s], &cmp->results[4]);  splay_destroy(spt);
        Treap     *trp = treap_create();    benchmark_treap(trp, dataset, sizes[s], &cmp->results[5]);  treap_destroy(trp);

        free(dataset);
    }
    return mbr;
}

/* ===================== Output Functions ===================== */

void print_benchmark_results(ComparisonResult *results) {
    if (!results) return;
    printf("\n================================================================\n");
    printf("   DETAILED RESULTS | N = %d\n", results->dataset_size);
    printf("================================================================\n\n");
    for (int i = 0; i < NUM_STRUCTURES; i++) {
        BenchmarkResult *r = &results->results[i];
        printf("  %s:\n", r->name);
        printf("    Insert Time:      %8.3f ms\n", r->insert_time_ms);
        printf("    Search Time:      %8.3f ms\n", r->search_time_ms);
        printf("    Range Query Time: %8.3f ms\n", r->range_time_ms);
        printf("    Node Traversals:  %8d\n",      r->node_traversals);
        printf("    Height:           %8d\n\n",    r->height);
    }
}

void print_comparison_table(ComparisonResult *results) {
    if (!results) return;
    const char *names[NUM_STRUCTURES] = {"B+Tree","SkipList","AVL","BST","Splay","Treap"};

    printf("\n+--N=%-6d | Insert(ms) | Search(ms) | Range(ms)  | Height | Traversals |\n",
           results->dataset_size);
    printf("+------------|------------|------------|------------|--------|------------|\n");

    for (int i = 0; i < NUM_STRUCTURES; i++) {
        BenchmarkResult *r = &results->results[i];
        printf("| %-10s | %10.3f | %10.3f | %10.3f | %6d | %10d |\n",
               names[i], r->insert_time_ms, r->search_time_ms,
               r->range_time_ms, r->height, r->node_traversals);
    }
    printf("+------------|------------|------------|------------|--------|------------+\n\n");
}

void print_multi_results(MultiBenchmarkResult *mbr) {
    if (!mbr) return;
    for (int s = 0; s < mbr->num_sizes; s++)
        print_comparison_table(&mbr->comparisons[s]);
}

/* ===================== CSV Export ===================== */

void export_csv(MultiBenchmarkResult *mbr, const char *filename) {
    if (!mbr || !filename) return;
    FILE *fp = fopen(filename, "w");
    if (!fp) { printf("Error: Cannot create CSV '%s'\n", filename); return; }

    fprintf(fp, "DataSize,Metric,BPlusTree,SkipList,AVLTree,BST,SplayTree,Treap\n");

    for (int s = 0; s < mbr->num_sizes; s++) {
        ComparisonResult *c = &mbr->comparisons[s];
        int n = mbr->sizes[s];

        fprintf(fp, "%d,InsertTime_ms,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n", n,
                c->results[0].insert_time_ms, c->results[1].insert_time_ms,
                c->results[2].insert_time_ms, c->results[3].insert_time_ms,
                c->results[4].insert_time_ms, c->results[5].insert_time_ms);

        fprintf(fp, "%d,SearchTime_ms,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n", n,
                c->results[0].search_time_ms, c->results[1].search_time_ms,
                c->results[2].search_time_ms, c->results[3].search_time_ms,
                c->results[4].search_time_ms, c->results[5].search_time_ms);

        fprintf(fp, "%d,RangeTime_ms,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n", n,
                c->results[0].range_time_ms, c->results[1].range_time_ms,
                c->results[2].range_time_ms, c->results[3].range_time_ms,
                c->results[4].range_time_ms, c->results[5].range_time_ms);

        fprintf(fp, "%d,Height,%d,%d,%d,%d,%d,%d\n", n,
                c->results[0].height, c->results[1].height,
                c->results[2].height, c->results[3].height,
                c->results[4].height, c->results[5].height);

        fprintf(fp, "%d,Traversals,%d,%d,%d,%d,%d,%d\n", n,
                c->results[0].node_traversals, c->results[1].node_traversals,
                c->results[2].node_traversals, c->results[3].node_traversals,
                c->results[4].node_traversals, c->results[5].node_traversals);
    }

    fclose(fp);
    printf("\n[CSV] Benchmark results exported to: %s\n", filename);
}

/* ===================== Observation Engine ===================== */

void print_observations(MultiBenchmarkResult *mbr) {
    if (!mbr || mbr->num_sizes < 2) return;
    int last = mbr->num_sizes - 1;
    ComparisonResult *large = &mbr->comparisons[last];

    printf("\n+---------------------------------------------------------------+\n");
    printf("|                OBSERVATION ENGINE                            |\n");
    printf("|        Automated Insights from Benchmark Results             |\n");
    printf("+---------------------------------------------------------------+\n\n");

    int obs = 0;

    /* Best range */
    int best_rng = 0;
    for (int i = 1; i < NUM_STRUCTURES; i++)
        if (large->results[i].range_time_ms < large->results[best_rng].range_time_ms) best_rng = i;
    printf("  [%d] Best range query: %s (%.3f ms at N=%d)\n",
           ++obs, large->results[best_rng].name,
           large->results[best_rng].range_time_ms, large->dataset_size);
    if (best_rng == 0)
        printf("       >> B+ Tree wins via linked leaf sequential scan.\n\n");
    else
        printf("       >> B+ Tree range: %.3f ms for comparison.\n\n", large->results[0].range_time_ms);

    /* Best insert */
    int best_ins = 0;
    for (int i = 1; i < NUM_STRUCTURES; i++)
        if (large->results[i].insert_time_ms < large->results[best_ins].insert_time_ms) best_ins = i;
    printf("  [%d] Fastest insert: %s (%.3f ms at N=%d)\n",
           ++obs, large->results[best_ins].name,
           large->results[best_ins].insert_time_ms, large->dataset_size);
    printf("       >> Treap/SkipList avoid deterministic rotation overhead.\n\n");

    /* BST sorted degradation */
    if (mbr->is_sorted) {
        printf("  [%d] BST degrades to O(n) for sorted input!\n", ++obs);
        printf("       >> Height at N=%d: BST=%d, AVL=%d, Splay=%d, Treap=%d\n\n",
               large->dataset_size, large->results[3].height,
               large->results[2].height, large->results[4].height, large->results[5].height);
    }

    /* Height comparison */
    printf("  [%d] Height at N=%d: B+T=%d | Skip=%d | AVL=%d | BST=%d | Splay=%d | Treap=%d\n",
           ++obs, large->dataset_size,
           large->results[0].height, large->results[1].height, large->results[2].height,
           large->results[3].height, large->results[4].height, large->results[5].height);
    printf("       >> B+ Tree lowest height via high fan-out. Splay height varies with access.\n\n");

    /* Best search */
    int best_sch = 0;
    for (int i = 1; i < NUM_STRUCTURES; i++)
        if (large->results[i].search_time_ms < large->results[best_sch].search_time_ms) best_sch = i;
    printf("  [%d] Fastest search: %s (%.3f ms, %d traversals)\n\n",
           ++obs, large->results[best_sch].name,
           large->results[best_sch].search_time_ms,
           large->results[best_sch].node_traversals);

    printf("  Total observations: %d\n\n", obs);
}

/* ===================== Theoretical Analysis ===================== */

void print_theoretical_analysis(void) {
    printf("\n+-------------------------------------------------------------------------------------------+\n");
    printf("|                        THEORETICAL COMPLEXITY ANALYSIS                                   |\n");
    printf("+-------------------------------------------------------------------------------------------+\n\n");

    printf("  +------------+----------+----------+----------+--------+----------+----------+\n");
    printf("  | Operation  | B+ Tree  | SkipList | AVL Tree | BST    | Splay    | Treap    |\n");
    printf("  +------------+----------+----------+----------+--------+----------+----------+\n");
    printf("  | Insert avg | O(log n) | O(log n) | O(log n) |O(log n)| O(log n)*| O(log n) |\n");
    printf("  | Insert wst | O(log n) | O(n)*    | O(log n) | O(n)   | O(n)*    | O(log n) |\n");
    printf("  | Search avg | O(log n) | O(log n) | O(log n) |O(log n)| O(log n)*| O(log n) |\n");
    printf("  | Search wst | O(log n) | O(n)*    | O(log n) | O(n)   | O(n)*    | O(log n) |\n");
    printf("  | Delete avg | O(log n) | O(log n) | O(log n) |O(log n)| O(log n)*| O(log n) |\n");
    printf("  | Range      |O(log n+k)|O(log n+k)|O(log n+k)| O(n+k) |O(log n+k)|O(log n+k)|\n");
    printf("  | Space      | O(n)     |O(n log n)| O(n)     | O(n)   | O(n)     | O(n)     |\n");
    printf("  +------------+----------+----------+----------+--------+----------+----------+\n");
    printf("  * Splay worst case O(n) but amortized O(log n).\n");
    printf("  * SkipList/Treap worst case with negligible probability.\n\n");

    printf("  SPLAY TREE NOTE: Excels when access patterns have temporal locality.\n");
    printf("  Repeatedly accessing the same key costs O(1) after first splay.\n\n");
    printf("  TREAP NOTE: Random priorities mean no adversary can force O(n) behavior.\n");
    printf("  Expected height is O(log n) with high probability.\n\n");

    printf("+-------------------------------------------------------------------------------------------+\n\n");
}

/* ===================== Cleanup ===================== */

void benchmark_cleanup(ComparisonResult *results)    { free(results); }

void multi_benchmark_cleanup(MultiBenchmarkResult *mbr) {
    if (!mbr) return;
    free(mbr->sizes);
    free(mbr->comparisons);
    free(mbr);
}

/* ===================== Trie Benchmark ===================== */

/*
 * Trie benchmark is separate from the int-key benchmark because
 * Trie operates on string keys. We measure:
 *   - Insert time for N words of varying length
 *   - Exact search time (N lookups)
 *   - Prefix search time (N/10 prefix queries)
 *   - DFS traversal time
 *   - BFS traversal time
 *   - Delete time (N/2 deletions)
 *   - Node count and word count after all ops
 */
void benchmark_trie(int num_words) {
    printf("\n");
    printf("+--------------------------------------------------------------+\n");
    printf("|         TRIE BENCHMARK  |  N = %-6d words               |\n", num_words);
    printf("+--------------------------------------------------------------+\n");

    /* Generate synthetic words: "word_00001" ... "word_NNNNN" */
    char **words = (char **)malloc(num_words * sizeof(char *));
    if (!words) return;
    for (int i = 0; i < num_words; i++) {
        words[i] = (char *)malloc(32);
        snprintf(words[i], 32, "word_%05d", i + 1);
    }

    /* Generate prefix queries — first 6 chars of every 10th word */
    int num_pfx = num_words / 10;
    if (num_pfx < 1) num_pfx = 1;
    char **prefixes = (char **)malloc(num_pfx * sizeof(char *));
    for (int i = 0; i < num_pfx; i++) {
        prefixes[i] = (char *)malloc(16);
        /* "word_" is shared by all; vary last 2 digits of 5-digit number */
        snprintf(prefixes[i], 16, "word_%02d", (i % 100));
    }

    Trie *trie = trie_create();
    clock_t start, end;
    double ms;

    /* --- Insert --- */
    start = clock();
    for (int i = 0; i < num_words; i++)
        trie_insert(trie, words[i], "val");
    end = clock();
    ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    printf("  Insert  (%6d words):     %8.3f ms   [O(m) per key]\n", num_words, ms);

    /* --- Exact Search --- */
    start = clock();
    for (int i = 0; i < num_words; i++)
        trie_search(trie, words[i]);
    end = clock();
    ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    printf("  Search  (%6d lookups):   %8.3f ms   [O(m) per lookup]\n", num_words, ms);

    /* --- Prefix Search --- */
    start = clock();
    for (int i = 0; i < num_pfx; i++) {
        TriePrefixResult *pr = trie_prefix_search(trie, prefixes[i]);
        if (pr) trie_prefix_result_free(pr);
    }
    end = clock();
    ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    printf("  PfxSrch (%6d queries):  %8.3f ms   [O(p + k*m) per query]\n", num_pfx, ms);

    /* --- DFS Traversal --- */
    start = clock();
    TrieTraversalResult *dfs = trie_traversal_dfs(trie);
    end = clock();
    ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    printf("  DFS Traversal:            %8.3f ms   [O(A*n), %d nodes]\n",
           ms, dfs ? dfs->nodes_visited : 0);
    if (dfs) trie_traversal_result_free(dfs);

    /* --- BFS Traversal --- */
    start = clock();
    TrieTraversalResult *bfs = trie_traversal_bfs(trie);
    end = clock();
    ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    printf("  BFS Traversal:            %8.3f ms   [O(A*n), %d nodes]\n",
           ms, bfs ? bfs->nodes_visited : 0);
    if (bfs) trie_traversal_result_free(bfs);

    /* --- Delete (first half of words) --- */
    int del_count = num_words / 2;
    start = clock();
    for (int i = 0; i < del_count; i++)
        trie_delete(trie, words[i]);
    end = clock();
    ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    printf("  Delete  (%6d words):     %8.3f ms   [O(m) per key]\n", del_count, ms);

    /* --- Final stats --- */
    printf("\n");
    TrieStats st = trie_get_stats(trie);
    printf("  After ops: words=%d, nodes=%d, max_depth=%d, avg_len=%.2f\n",
           st.word_count, st.node_count, st.max_depth, st.avg_word_len);
    printf("  Char-step counters:");
    printf("  insert=%ld  search=%ld  delete=%ld  prefix=%ld\n",
           (long)trie->insert_char_ops, (long)trie->search_char_ops,
           (long)trie->delete_char_ops, (long)trie->prefix_char_ops);
    printf("+--------------------------------------------------------------+\n");

    /* Cleanup */
    trie_destroy(trie);
    for (int i = 0; i < num_words; i++) free(words[i]);
    free(words);
    for (int i = 0; i < num_pfx; i++) free(prefixes[i]);
    free(prefixes);
}

/* ===================== Trie CSV Export ===================== */

/*
 * Run Trie benchmark across multiple sizes and write a CSV that can be
 * merged with the int-key benchmark CSVs by the Python plotter.
 *
 * CSV columns: DataSize,InsertTime_ms,SearchTime_ms,PrefixTime_ms,
 *              DFSTime_ms,BFSTime_ms,DeleteTime_ms,NodeCount,WordCount
 */
void export_trie_csv(int *sizes, int num_sizes, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) { printf("Error: Cannot create '%s'\n", filename); return; }

    fprintf(fp, "DataSize,InsertTime_ms,SearchTime_ms,PrefixTime_ms,"
                "DFSTime_ms,BFSTime_ms,DeleteTime_ms,NodeCount,WordCount\n");

    for (int s = 0; s < num_sizes; s++) {
        int num_words = sizes[s];

        /* Build word list */
        char **words = (char **)malloc(num_words * sizeof(char *));
        if (!words) continue;
        for (int i = 0; i < num_words; i++) {
            words[i] = (char *)malloc(32);
            snprintf(words[i], 32, "word_%05d", i + 1);
        }

        int num_pfx = num_words / 10;
        if (num_pfx < 1) num_pfx = 1;
        char **prefixes = (char **)malloc(num_pfx * sizeof(char *));
        for (int i = 0; i < num_pfx; i++) {
            prefixes[i] = (char *)malloc(16);
            snprintf(prefixes[i], 16, "word_%02d", (i % 100));
        }

        Trie *trie = trie_create();
        clock_t start, end;

        /* Insert */
        start = clock();
        for (int i = 0; i < num_words; i++) trie_insert(trie, words[i], "v");
        end = clock();
        double ins_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

        /* Search */
        start = clock();
        for (int i = 0; i < num_words; i++) trie_search(trie, words[i]);
        end = clock();
        double sch_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

        /* Prefix search */
        start = clock();
        for (int i = 0; i < num_pfx; i++) {
            TriePrefixResult *pr = trie_prefix_search(trie, prefixes[i]);
            if (pr) trie_prefix_result_free(pr);
        }
        end = clock();
        double pfx_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

        /* DFS */
        start = clock();
        TrieTraversalResult *dfs = trie_traversal_dfs(trie);
        end = clock();
        double dfs_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
        if (dfs) trie_traversal_result_free(dfs);

        /* BFS */
        start = clock();
        TrieTraversalResult *bfs = trie_traversal_bfs(trie);
        end = clock();
        double bfs_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
        if (bfs) trie_traversal_result_free(bfs);

        /* Delete half */
        int del_count = num_words / 2;
        start = clock();
        for (int i = 0; i < del_count; i++) trie_delete(trie, words[i]);
        end = clock();
        double del_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

        fprintf(fp, "%d,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%d,%d\n",
                num_words, ins_ms, sch_ms, pfx_ms, dfs_ms, bfs_ms, del_ms,
                trie->node_count, trie->word_count);

        printf("  [Trie CSV] N=%d done\n", num_words);

        trie_destroy(trie);
        for (int i = 0; i < num_words; i++) free(words[i]);
        free(words);
        for (int i = 0; i < num_pfx; i++) free(prefixes[i]);
        free(prefixes);
    }

    fclose(fp);
    printf("[CSV] Trie benchmark exported to: %s\n", filename);
}
