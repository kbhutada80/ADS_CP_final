#include "bptree.h"
#include "skiplist.h"
#include "avl.h"
#include "bst.h"
#include "splay.h"
#include "treap.h"
#include "trie.h"
#include "benchmark.h"
#include "adaptive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * Comparative Analysis and Adaptive Selection of Data Structures
 * Main CLI Interface — now covering 8 data structures:
 *   B+ Tree, Skip List, AVL Tree, BST, Splay Tree, Treap, Trie, Graph
 */

static AdaptiveDS *active_ds   = NULL;
static DataStructureType active_type = DS_AVL;

/* ===================== Banner / Help ===================== */

static void print_banner(void) {
    printf("\n");
    printf("+------------------------------------------------------------------+\n");
    printf("|   Comparative Analysis & Adaptive Selection of                  |\n");
    printf("|   Data Structures for Efficient Range Queries                   |\n");
    printf("+------------------------------------------------------------------+\n");
    printf("|   Trees: B+Tree | SkipList | AVL | BST | Splay | Treap         |\n");
    printf("|   Other: Trie (prefix tree)                                    |\n");
    printf("|   Type 'help' for available commands                            |\n");
    printf("+------------------------------------------------------------------+\n\n");
}

static void print_help(void) {
    printf("\n");
    printf("+------------------------------------------------------------------+\n");
    printf("|                    AVAILABLE COMMANDS                           |\n");
    printf("+------------------------------------------------------------------+\n");
    printf("|  DATA OPERATIONS (on active tree DS):                          |\n");
    printf("|    insert <key> <value>   Insert key-value pair                |\n");
    printf("|    search <key>           Search for a key                     |\n");
    printf("|    delete <key>           Delete a key                         |\n");
    printf("|    range <start> <end>    Range query (inclusive)              |\n");
    printf("|    print_structure        Print current DS                     |\n");
    printf("|    switch <type>          Switch active DS                     |\n");
    printf("|         types: bptree, skiplist, avl, bst, splay, treap       |\n");
    printf("|    load_sample            Load sample data                     |\n");
    printf("|                                                                |\n");
    printf("|  BENCHMARKING:                                                 |\n");
    printf("|    benchmark              Full benchmark (random, 6 DS)        |\n");
    printf("|    benchmark_sorted       Full benchmark (sorted input)        |\n");
    printf("|    benchmark_quick        Quick benchmark (small sizes)        |\n");
    printf("|    theory                 Theoretical complexity table         |\n");
    printf("|                                                                |\n");
    printf("|  ADAPTIVE SYSTEM:                                              |\n");
    printf("|    auto                   Adaptive selection demo              |\n");
    printf("|    auto_custom            Custom workload profile              |\n");
    printf("|                                                                |\n");
    printf("|  TRIE COMMANDS:                                                |\n");
    printf("|    trie_demo              Run Trie prefix-tree demo            |\n");
    printf("|                                                                |\n");
    printf("|  OTHER:                                                        |\n");
    printf("|    help                   Show this help                       |\n");
    printf("|    exit / quit            Exit program                         |\n");
    printf("+------------------------------------------------------------------+\n\n");
}

/* ===================== DS Switching ===================== */

static void switch_ds(const char *type_str) {
    DataStructureType new_type;

    if      (strcmp(type_str, "bptree")   == 0) new_type = DS_BPTREE;
    else if (strcmp(type_str, "skiplist") == 0) new_type = DS_SKIPLIST;
    else if (strcmp(type_str, "avl")      == 0) new_type = DS_AVL;
    else if (strcmp(type_str, "bst")      == 0) new_type = DS_BST;
    else if (strcmp(type_str, "splay")    == 0) new_type = DS_SPLAY;
    else if (strcmp(type_str, "treap")    == 0) new_type = DS_TREAP;
    else {
        printf("Unknown type: %s\n", type_str);
        printf("Valid types: bptree, skiplist, avl, bst, splay, treap\n");
        return;
    }

    if (active_ds) adaptive_destroy(active_ds);
    active_type = new_type;
    active_ds   = adaptive_create(new_type);
    printf("Switched to: %s\n", ds_type_name(new_type));
}

/* ===================== Sample Data ===================== */

static void load_sample_data(void) {
    printf("Loading sample data into %s...\n", ds_type_name(active_type));
    struct { int key; const char *val; } samples[] = {
        {10,"apple"},{20,"banana"},{30,"cherry"},{40,"date"},{50,"elderberry"},
        {60,"fig"},{70,"grape"},{80,"honeydew"},{90,"iris"},{100,"jackfruit"},
        {5,"avocado"},{15,"blueberry"},{25,"coconut"},{35,"dragonfruit"},
        {45,"eggplant"},{55,"fennel"},{65,"guava"},{75,"hazelnut"},
        {85,"jalapeno"},{95,"kiwi"}
    };
    int count = (int)(sizeof(samples) / sizeof(samples[0]));
    for (int i = 0; i < count; i++)
        adaptive_insert(active_ds, samples[i].key, samples[i].val);
    printf("Loaded %d sample items.\n", count);
}

/* ===================== Range Query Dispatch ===================== */

static void do_range_query(int start, int end) {
    printf("Range query [%d, %d] on %s:\n\n", start, end, ds_type_name(active_type));

    switch (active_type) {
        case DS_BPTREE: {
            BPTree *tree = (BPTree *)active_ds->ds;
            BPTreeLeafNode *leaf = tree->leftmost_leaf;
            int count = 0;
            while (leaf) {
                for (int i = 0; i < leaf->key_count; i++) {
                    if (leaf->keys[i] >= start && leaf->keys[i] <= end)
                        { printf("  %d => %s\n", leaf->keys[i], leaf->values[i]); count++; }
                    if (leaf->keys[i] > end) goto bpt_done;
                }
                leaf = leaf->next;
            }
            bpt_done:
            printf("\nFound %d results.\n", count);
            break;
        }
        case DS_SKIPLIST: {
            SkipList *sl = (SkipList *)active_ds->ds;
            SkipListNode *node = sl->header->forward[0];
            int count = 0;
            while (node) {
                if (node->key >= start && node->key <= end)
                    { printf("  %d => %s\n", node->key, node->value); count++; }
                if (node->key > end) break;
                node = node->forward[0];
            }
            printf("\nFound %d results.\n", count);
            break;
        }
        case DS_AVL: {
            int trav = 0;
            AVLRangeResult *rr = avl_range_query((AVLTree *)active_ds->ds, start, end, &trav);
            if (rr) {
                for (int i = 0; i < rr->count; i++)
                    printf("  %d => %s\n", rr->keys[i], rr->values[i]);
                printf("\nFound %d results (traversals: %d).\n", rr->count, trav);
                avl_range_result_free(rr);
            }
            break;
        }
        case DS_BST: {
            int trav = 0;
            BSTRangeResult *rr = bst_range_query((BSTree *)active_ds->ds, start, end, &trav);
            if (rr) {
                for (int i = 0; i < rr->count; i++)
                    printf("  %d => %s\n", rr->keys[i], rr->values[i]);
                printf("\nFound %d results (traversals: %d).\n", rr->count, trav);
                bst_range_result_free(rr);
            }
            break;
        }
        case DS_SPLAY: {
            int trav = 0;
            SplayRangeResult *rr = splay_range_query((SplayTree *)active_ds->ds, start, end, &trav);
            if (rr) {
                for (int i = 0; i < rr->count; i++)
                    printf("  %d => %s\n", rr->keys[i], rr->values[i]);
                printf("\nFound %d results (traversals: %d).\n", rr->count, trav);
                splay_range_result_free(rr);
            }
            break;
        }
        case DS_TREAP: {
            int trav = 0;
            TreapRangeResult *rr = treap_range_query((Treap *)active_ds->ds, start, end, &trav);
            if (rr) {
                for (int i = 0; i < rr->count; i++)
                    printf("  %d => %s\n", rr->keys[i], rr->values[i]);
                printf("\nFound %d results (traversals: %d).\n", rr->count, trav);
                treap_range_result_free(rr);
            }
            break;
        }
        default:
            printf("Range query not supported for this structure.\n");
    }
}

/* ===================== Trie Demo ===================== */

static void run_trie_demo(void) {
    printf("\n+------------------------------------------------------------------+\n");
    printf("|           TRIE (PREFIX TREE) — FULL OPERATION DEMO             |\n");
    printf("|  Insert | Search | Prefix | Delete | DFS | BFS | Stats | Cmplx|\n");
    printf("+------------------------------------------------------------------+\n\n");

    Trie *trie = trie_create();

    /* === 1. INSERTION === */
    printf("=== [1] INSERTION  O(m) per key ===\n");
    const char *words[] = {
        "apple","application","apply","apt",
        "banana","band","bandwidth",
        "cat","catch","category",
        "data","database","datastructure",
        "green","great","grid","grape"
    };
    const char *vals[] = {
        "fruit","software","action","suitable",
        "fruit","music","internet",
        "animal","grab","class",
        "information","storage","cs-topic",
        "color","adjective","lattice","fruit2"
    };
    int n = (int)(sizeof(words) / sizeof(words[0]));
    for (int i = 0; i < n; i++) {
        trie_insert(trie, words[i], vals[i]);
        printf("  inserted: \"%s\" => %s\n", words[i], vals[i]);
    }
    printf("  -- %d words, %d nodes --\n\n", trie->word_count, trie->node_count);

    /* === 2. SEARCH === */
    printf("=== [2] SEARCH  O(m) exact match ===\n");
    const char *srch[] = {"apple","application","apt","band","missing","cat","xy","gr"};
    for (int i = 0; i < 8; i++) {
        char *v = trie_search(trie, srch[i]);
        printf("  search(\"%s\") = %s\n", srch[i], v ? v : "NOT FOUND");
    }
    printf("\n");

    /* === 3. PREFIX EXISTS === */
    printf("=== [3] PREFIX EXISTS  O(p) ===\n");
    const char *pc[] = {"app","ban","cat","xyz","da","gr","z"};
    for (int i = 0; i < 7; i++)
        printf("  starts_with(\"%s\") = %s\n",
               pc[i], trie_starts_with(trie, pc[i]) ? "YES" : "NO");
    printf("\n");

    /* === 4. PREFIX SEARCH === */
    printf("=== [4] PREFIX SEARCH  O(p + k*m) ===\n");
    const char *pfx[] = {"app","ban","gr","dat","c"};
    for (int i = 0; i < 5; i++) {
        TriePrefixResult *pr = trie_prefix_search(trie, pfx[i]);
        printf("  prefix_search(\"%s\") — %d match(es):\n", pfx[i], pr ? pr->count : 0);
        if (pr) {
            for (int j = 0; j < pr->count; j++)
                printf("    \"%s\" => %s\n", pr->words[j], pr->values[j] ? pr->values[j] : "");
            trie_prefix_result_free(pr);
        }
    }
    printf("\n");

    /* === 5. DFS TRAVERSAL === */
    printf("=== [5] DFS TRAVERSAL  (lexicographic order) ===\n");
    TrieTraversalResult *dfs = trie_traversal_dfs(trie);
    if (dfs) {
        printf("  %d words, %d nodes visited:\n", dfs->count, dfs->nodes_visited);
        for (int i = 0; i < dfs->count; i++)
            printf("    [depth=%2d] \"%s\"\n", dfs->depths[i], dfs->words[i]);
        trie_traversal_result_free(dfs);
    }
    printf("\n");

    /* === 6. BFS TRAVERSAL === */
    printf("=== [6] BFS TRAVERSAL  (shortest words first) ===\n");
    TrieTraversalResult *bfs = trie_traversal_bfs(trie);
    if (bfs) {
        printf("  %d words, %d nodes visited:\n", bfs->count, bfs->nodes_visited);
        for (int i = 0; i < bfs->count; i++)
            printf("    [depth=%2d] \"%s\"\n", bfs->depths[i], bfs->words[i]);
        trie_traversal_result_free(bfs);
    }
    printf("\n");

    /* === 7. DELETION === */
    printf("=== [7] DELETION  O(m) with bottom-up pruning ===\n");
    const char *del[] = {"apple","band","data","missing"};
    for (int i = 0; i < 4; i++) {
        int ok = trie_delete(trie, del[i]);
        printf("  delete(\"%s\") = %s\n", del[i], ok ? "DELETED" : "NOT FOUND");
    }
    printf("  After: %d words, %d nodes\n", trie->word_count, trie->node_count);
    printf("  \"application\" still there: %s\n",
           trie_search(trie, "application") ? "YES" : "NO");
    printf("  \"apple\" gone:              %s\n\n",
           trie_search(trie, "apple") ? "NO (still here!)" : "YES (removed)");

    /* === 8. STATS === */
    printf("=== [8] STATS & OPERATION COUNTERS ===\n");
    trie_print_stats(trie);

    /* === 9. COMPLEXITY TABLE === */
    printf("=== [9] COMPLEXITY ANALYSIS ===\n");
    trie_print_complexity();

    trie_destroy(trie);
    printf("  Trie demo complete.\n\n");
}

/* ===================== Adaptive Demo ===================== */

static void run_adaptive_demo(void) {
    printf("\n+------------------------------------------------------------------+\n");
    printf("|              ADAPTIVE SELECTION SYSTEM DEMO                    |\n");
    printf("+------------------------------------------------------------------+\n\n");

    struct { const char *name; WorkloadProfile profile; } workloads[] = {
        {"Heavy Range Queries (N=10000)",    workload_heavy_range(10000)},
        {"Heavy Inserts - Random (N=10000)", workload_heavy_insert(10000)},
        {"Heavy Searches (N=10000)",         workload_heavy_search(10000)},
        {"Mixed Workload (N=10000)",         workload_mixed(10000)},
        {"Small Dataset (N=500)",            workload_small_dataset()},
        {"Large Range Queries (N=100000)",   workload_heavy_range(100000)},
        {"Temporal Locality (N=10000)",      workload_temporal_locality(10000)},
    };

    int n = (int)(sizeof(workloads) / sizeof(workloads[0]));
    for (int i = 0; i < n; i++) {
        printf("--- Workload: %s ---\n", workloads[i].name);
        print_workload_profile(&workloads[i].profile);
        SelectionResult result = select_data_structure(workloads[i].profile);
        print_selection_result(&result);

        AdaptiveDS *ads = adaptive_create(result.selected);
        printf("  Demo: Inserting 100 items into %s...\n", ds_type_name(result.selected));
        for (int j = 1; j <= 100; j++) {
            char val[16]; snprintf(val, sizeof(val), "item_%d", j);
            adaptive_insert(ads, j, val);
        }
        char *found = adaptive_search(ads, 50);
        printf("  Demo: Search(50) = %s\n\n", found ? found : "NOT FOUND");
        adaptive_destroy(ads);
    }
}

static void run_adaptive_custom(void) {
    WorkloadProfile p;
    printf("\n--- Custom Workload Profile ---\n");
    printf("Enter dataset size: ");
    if (scanf("%d", &p.dataset_size) != 1) { while(getchar()!='\n'); return; }
    printf("Insert ratio (0.0-1.0): ");
    if (scanf("%lf", &p.insert_ratio) != 1) { while(getchar()!='\n'); return; }
    printf("Search ratio (0.0-1.0): ");
    if (scanf("%lf", &p.search_ratio) != 1) { while(getchar()!='\n'); return; }
    printf("Range query ratio (0.0-1.0): ");
    if (scanf("%lf", &p.range_ratio) != 1) { while(getchar()!='\n'); return; }
    p.delete_ratio = 1.0 - p.insert_ratio - p.search_ratio - p.range_ratio;
    if (p.delete_ratio < 0) p.delete_ratio = 0;
    printf("Data randomness (0.0=sorted, 1.0=random): ");
    if (scanf("%lf", &p.data_randomness) != 1) { while(getchar()!='\n'); return; }
    printf("Temporal locality (0.0=uniform, 1.0=hot set): ");
    if (scanf("%lf", &p.temporal_locality) != 1) { while(getchar()!='\n'); return; }
    p.range_query_size = p.dataset_size / 10;
    while(getchar()!='\n');

    print_workload_profile(&p);
    SelectionResult result = select_data_structure(p);
    print_selection_result(&result);
}

/* ===================== Command Execution ===================== */

static void execute_command(const char *command) {
    if (!command || strlen(command) == 0) return;

    char cmd[64]; int key, start_key, end_key; char value[256]; char type_str[32];
    if (sscanf(command, "%63s", cmd) < 1) return;

    if (strcmp(cmd, "insert") == 0) {
        if (sscanf(command, "%*s %d %255s", &key, value) == 2) {
            adaptive_insert(active_ds, key, value);
            printf("[%s] Inserted: %d => %s\n", ds_type_name(active_type), key, value);
        } else printf("Usage: insert <key> <value>\n");

    } else if (strcmp(cmd, "search") == 0) {
        if (sscanf(command, "%*s %d", &key) == 1) {
            char *result = adaptive_search(active_ds, key);
            if (result) printf("[%s] Found: %d => %s\n", ds_type_name(active_type), key, result);
            else        printf("[%s] Key %d not found.\n", ds_type_name(active_type), key);
        } else printf("Usage: search <key>\n");

    } else if (strcmp(cmd, "delete") == 0) {
        if (sscanf(command, "%*s %d", &key) == 1) {
            int ok = adaptive_delete(active_ds, key);
            if (ok) printf("[%s] Deleted key %d.\n", ds_type_name(active_type), key);
            else    printf("[%s] Key %d not found.\n", ds_type_name(active_type), key);
        } else printf("Usage: delete <key>\n");

    } else if (strcmp(cmd, "range") == 0) {
        if (sscanf(command, "%*s %d %d", &start_key, &end_key) == 2)
            do_range_query(start_key, end_key);
        else printf("Usage: range <start> <end>\n");

    } else if (strcmp(cmd, "benchmark") == 0) {
        int sizes[] = {1000, 5000, 10000, 50000, 100000}; int n = 5;
        MultiBenchmarkResult *mbr = run_multi_benchmark(sizes, n, 0);
        if (mbr) { print_multi_results(mbr); export_csv(mbr, "benchmark_random.csv");
                   print_observations(mbr); multi_benchmark_cleanup(mbr); }
        export_trie_csv(sizes, n, "trie_benchmark.csv");

    } else if (strcmp(cmd, "benchmark_sorted") == 0) {
        int sizes[] = {1000, 5000, 10000, 50000}; int n = 4;
        MultiBenchmarkResult *mbr = run_multi_benchmark(sizes, n, 1);
        if (mbr) { print_multi_results(mbr); export_csv(mbr, "benchmark_sorted.csv");
                   print_observations(mbr); multi_benchmark_cleanup(mbr); }
        export_trie_csv(sizes, n, "trie_benchmark.csv");

    } else if (strcmp(cmd, "benchmark_quick") == 0) {
        int sizes[] = {1000, 5000, 10000}; int n = 3;
        MultiBenchmarkResult *mbr = run_multi_benchmark(sizes, n, 0);
        if (mbr) { print_multi_results(mbr); export_csv(mbr, "benchmark_quick.csv");
                   print_observations(mbr); multi_benchmark_cleanup(mbr); }

    } else if (strcmp(cmd, "auto") == 0)           { run_adaptive_demo();
    } else if (strcmp(cmd, "auto_custom") == 0)    { run_adaptive_custom();
    } else if (strcmp(cmd, "theory") == 0)         { print_theoretical_analysis();
    } else if (strcmp(cmd, "trie_demo") == 0)      { run_trie_demo();
    } else if (strcmp(cmd, "trie_benchmark") == 0) {
        int sizes[] = {1000, 5000, 10000, 50000, 100000};
        for (int i = 0; i < 5; i++) benchmark_trie(sizes[i]);

    } else if (strcmp(cmd, "switch") == 0) {
        if (sscanf(command, "%*s %31s", type_str) == 1) switch_ds(type_str);
        else printf("Usage: switch <bptree|skiplist|avl|bst|splay|treap>\n");

    } else if (strcmp(cmd, "print_structure") == 0) { adaptive_print(active_ds);
    } else if (strcmp(cmd, "load_sample") == 0)     { load_sample_data();
    } else if (strcmp(cmd, "help") == 0)            { print_help();
    } else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) { return;
    } else {
        printf("Unknown command: '%s'. Type 'help' for available commands.\n", cmd);
    }
}

/* ===================== Interactive Loop ===================== */

static void interactive_mode(void) {
    char command[1024];
    print_banner();
    active_ds = adaptive_create(active_type);
    printf("Active data structure: %s\n", ds_type_name(active_type));
    printf("Type 'help' for available commands.\n\n");

    while (1) {
        printf("ds> "); fflush(stdout);
        if (fgets(command, sizeof(command), stdin) == NULL) break;
        command[strcspn(command, "\n")] = '\0';
        if (strlen(command) == 0) continue;
        if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0)
            { printf("Goodbye!\n"); break; }
        execute_command(command);
    }

    if (active_ds) { adaptive_destroy(active_ds); active_ds = NULL; }
}

/* ===================== Main ===================== */

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));

    if (argc > 1) {
        if (strcmp(argv[1], "benchmark") == 0) {
            int sizes[] = {1000, 5000, 10000, 50000, 100000};
            MultiBenchmarkResult *mbr = run_multi_benchmark(sizes, 5, 0);
            if (mbr) { print_multi_results(mbr); export_csv(mbr, "benchmark_random.csv");
                       print_observations(mbr); multi_benchmark_cleanup(mbr); }
        } else if (strcmp(argv[1], "benchmark_sorted") == 0) {
            int sizes[] = {1000, 5000, 10000, 50000};
            MultiBenchmarkResult *mbr = run_multi_benchmark(sizes, 4, 1);
            if (mbr) { print_multi_results(mbr); export_csv(mbr, "benchmark_sorted.csv");
                       print_observations(mbr); multi_benchmark_cleanup(mbr); }
        } else if (strcmp(argv[1], "auto") == 0)   { run_adaptive_demo();
        } else if (strcmp(argv[1], "theory") == 0) { print_theoretical_analysis();
        } else if (strcmp(argv[1], "trie") == 0)   { run_trie_demo();
        } else {
            printf("Usage: %s [benchmark|benchmark_sorted|auto|theory|trie]\n", argv[0]);
        }
    } else {
        interactive_mode();
    }

    return EXIT_SUCCESS;
}
