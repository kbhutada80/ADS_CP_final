#!/usr/bin/env python3
"""
Comprehensive Graph Generation — All 7 Data Structures
=======================================================
Reads:
  - benchmark_random.csv  / benchmark_sorted.csv  (6 int-key DS)
  - trie_benchmark.csv                             (Trie string-key DS)

Generates per input CSV:
  1.  Insert Time vs Dataset Size      (6 int-key DS)
  2.  Search Time vs Dataset Size      (6 int-key DS)
  3.  Range Query Time vs Dataset Size (6 int-key DS)
  4.  Height / Level vs Dataset Size   (6 int-key DS)
  5.  Node Traversals vs Dataset Size  (6 int-key DS)
  6.  Combined 3-panel timing          (6 int-key DS)
  7.  Trie: all operations over sizes  (Trie only)
  8.  Insert comparison: 6 DS + Trie   (all 7 DS side-by-side)
  9.  Search comparison: 6 DS + Trie   (all 7 DS side-by-side)
  10. Radar / spider chart              (7 DS capability fingerprint)
  11. Heatmap: operation scores         (7 DS x 5 operations)

Usage:
    python plot_results.py benchmark_random.csv [trie_benchmark.csv]
    python plot_results.py benchmark_random.csv benchmark_sorted.csv
"""

import sys
import csv
import os
from collections import defaultdict

try:
    import matplotlib
    matplotlib.use('Agg')          # non-interactive backend (works without display)
    import matplotlib.pyplot as plt
    import matplotlib.patches as mpatches
    import numpy as np
    matplotlib.rcParams.update({
        'figure.figsize':  (11, 7),
        'font.size':       12,
        'font.family':     'DejaVu Sans',
        'axes.spines.top': False,
        'axes.spines.right': False,
    })
    HAS_MATPLOTLIB = True
except ImportError:
    HAS_MATPLOTLIB = False
    print("WARNING: matplotlib/numpy not installed.")
    print("Install: pip install matplotlib numpy")

# ─────────────────────────────────────────────
#  Style definitions — 7 data structures
# ─────────────────────────────────────────────

DS_ORDER  = ['BPlusTree', 'SkipList', 'AVLTree', 'BST', 'SplayTree', 'Treap', 'Trie']

COLORS = {
    'BPlusTree': '#1565C0',   # deep blue
    'SkipList':  '#E65100',   # deep orange
    'AVLTree':   '#2E7D32',   # dark green
    'BST':       '#B71C1C',   # dark red
    'SplayTree': '#6A1B9A',   # purple
    'Treap':     '#00838F',   # teal
    'Trie':      '#F9A825',   # amber
}

LABELS = {
    'BPlusTree': 'B+ Tree',
    'SkipList':  'Skip List',
    'AVLTree':   'AVL Tree',
    'BST':       'BST',
    'SplayTree': 'Splay Tree',
    'Treap':     'Treap',
    'Trie':      'Trie',
}

MARKERS = {
    'BPlusTree': 'o',
    'SkipList':  's',
    'AVLTree':   '^',
    'BST':       'D',
    'SplayTree': 'P',
    'Treap':     'X',
    'Trie':      '*',
}

LINE_STYLES = {
    'BPlusTree': '-',
    'SkipList':  '--',
    'AVLTree':   '-.',
    'BST':       ':',
    'SplayTree': '-',
    'Treap':     '--',
    'Trie':      '-.',
}

INT_DS = ['BPlusTree', 'SkipList', 'AVLTree', 'BST', 'SplayTree', 'Treap']

# ─────────────────────────────────────────────
#  Parsers
# ─────────────────────────────────────────────

def parse_int_csv(filename):
    """Parse 6-DS int-key benchmark CSV."""
    data = defaultdict(lambda: defaultdict(list))
    sizes = set()
    with open(filename, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            size   = int(row['DataSize'])
            metric = row['Metric']
            sizes.add(size)
            for ds in INT_DS:
                if ds not in row:
                    continue
                try:
                    val = float(row[ds])
                except (ValueError, KeyError):
                    val = 0.0
                data[metric][ds].append((size, val))
    for metric in data:
        for ds in data[metric]:
            data[metric][ds].sort(key=lambda x: x[0])
    return data, sorted(sizes)


def parse_trie_csv(filename):
    """Parse Trie-specific benchmark CSV."""
    rows = []
    if not os.path.exists(filename):
        return None
    with open(filename, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append({k: float(v) if k != 'DataSize' else int(v)
                         for k, v in row.items()})
    rows.sort(key=lambda r: r['DataSize'])
    return rows

# ─────────────────────────────────────────────
#  Helper: save figure
# ─────────────────────────────────────────────

def _save(fig, path):
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print(f"  Saved: {path}")

# ─────────────────────────────────────────────
#  Plot 1–5: Individual metric plots (6 DS)
# ─────────────────────────────────────────────

def plot_metric(data, metric, title, ylabel, filepath, sizes, ds_list=None):
    if ds_list is None:
        ds_list = INT_DS

    if not HAS_MATPLOTLIB:
        print(f"\n--- {title} ---")
        print(f"{'Size':>8}", end="")
        for ds in ds_list:
            print(f"  {LABELS.get(ds, ds):>12}", end="")
        print()
        for i, size in enumerate(sizes):
            print(f"{size:>8}", end="")
            for ds in ds_list:
                pts = data.get(metric, {}).get(ds, [])
                val = pts[i][1] if i < len(pts) else 0
                print(f"  {val:>12.4f}", end="")
            print()
        return

    fig, ax = plt.subplots(figsize=(11, 6))

    for ds in ds_list:
        pts = data.get(metric, {}).get(ds, [])
        if not pts:
            continue
        x = [p[0] for p in pts]
        y = [p[1] for p in pts]
        ax.plot(x, y,
                color=COLORS.get(ds, '#888'),
                marker=MARKERS.get(ds, 'o'),
                linestyle=LINE_STYLES.get(ds, '-'),
                label=LABELS.get(ds, ds),
                linewidth=2.2,
                markersize=8)

    ax.set_xlabel('Dataset Size (N)', fontsize=13)
    ax.set_ylabel(ylabel, fontsize=13)
    ax.set_title(title, fontsize=15, fontweight='bold', pad=12)
    ax.legend(fontsize=10, loc='best', framealpha=0.9)
    ax.grid(True, alpha=0.25, linestyle='--')
    ax.set_xscale('log')

    _save(fig, filepath)

# ─────────────────────────────────────────────
#  Plot 6: Combined 3-panel timing (6 DS)
# ─────────────────────────────────────────────

def plot_combined(data, prefix):
    if not HAS_MATPLOTLIB:
        return

    fig, axes = plt.subplots(1, 3, figsize=(20, 6))
    panels = [
        ('InsertTime_ms', 'Insert Time',      'Time (ms)'),
        ('SearchTime_ms', 'Search Time',      'Time (ms)'),
        ('RangeTime_ms',  'Range Query Time', 'Time (ms)'),
    ]

    for ax, (metric, title, ylabel) in zip(axes, panels):
        for ds in INT_DS:
            pts = data.get(metric, {}).get(ds, [])
            if not pts:
                continue
            x = [p[0] for p in pts]
            y = [p[1] for p in pts]
            ax.plot(x, y,
                    color=COLORS[ds],
                    marker=MARKERS[ds],
                    linestyle=LINE_STYLES[ds],
                    label=LABELS[ds],
                    linewidth=2, markersize=7)
        ax.set_xlabel('Dataset Size')
        ax.set_ylabel(ylabel)
        ax.set_title(title, fontweight='bold')
        ax.legend(fontsize=8)
        ax.grid(True, alpha=0.25, linestyle='--')
        ax.set_xscale('log')

    fig.suptitle('Performance Comparison — 6 Integer-Key Data Structures',
                 fontsize=14, fontweight='bold', y=1.02)
    plt.tight_layout()
    _save(fig, f'{prefix}_combined.png')

# ─────────────────────────────────────────────
#  Plot 7: Trie-specific operations
# ─────────────────────────────────────────────

def plot_trie_ops(trie_data, prefix):
    if not HAS_MATPLOTLIB or not trie_data:
        return

    sizes = [r['DataSize'] for r in trie_data]
    ops = [
        ('InsertTime_ms',  'Insert',        '#1565C0'),
        ('SearchTime_ms',  'Search',        '#2E7D32'),
        ('PrefixTime_ms',  'Prefix Search', '#E65100'),
        ('DFSTime_ms',     'DFS Traversal', '#6A1B9A'),
        ('BFSTime_ms',     'BFS Traversal', '#00838F'),
        ('DeleteTime_ms',  'Delete',        '#B71C1C'),
    ]

    fig, ax = plt.subplots(figsize=(12, 7))

    for col, label, color in ops:
        vals = [r[col] for r in trie_data]
        ax.plot(sizes, vals, color=color, marker='o', linewidth=2.2,
                markersize=8, label=label)

    ax.set_xlabel('Dataset Size (N)', fontsize=13)
    ax.set_ylabel('Time (ms)', fontsize=13)
    ax.set_title('Trie — All Operations vs Dataset Size',
                 fontsize=15, fontweight='bold')
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.25, linestyle='--')
    ax.set_xscale('log')

    _save(fig, f'{prefix}_trie_ops.png')

    # Also plot node count vs words remaining
    fig2, ax2 = plt.subplots(figsize=(10, 5))
    ax2.bar([str(r['DataSize']) for r in trie_data],
            [r['NodeCount'] for r in trie_data],
            color='#F9A825', alpha=0.85, label='Nodes')
    ax2.bar([str(r['DataSize']) for r in trie_data],
            [r['WordCount'] for r in trie_data],
            color='#1565C0', alpha=0.85, label='Words (after delete)')
    ax2.set_xlabel('Dataset Size (N)')
    ax2.set_ylabel('Count')
    ax2.set_title('Trie — Node Count vs Word Count (after half-deletion)')
    ax2.legend()
    ax2.grid(True, alpha=0.25, axis='y')
    _save(fig2, f'{prefix}_trie_nodes.png')

# ─────────────────────────────────────────────
#  Plot 8 & 9: Insert / Search — all 7 DS
# ─────────────────────────────────────────────

def plot_all7_metric(data, trie_data, metric, trie_col, title, ylabel, filepath):
    """Plot one metric for all 7 DS on the same axes."""
    if not HAS_MATPLOTLIB:
        return

    fig, ax = plt.subplots(figsize=(12, 7))

    # 6 int-key DS
    for ds in INT_DS:
        pts = data.get(metric, {}).get(ds, [])
        if not pts:
            continue
        x = [p[0] for p in pts]
        y = [p[1] for p in pts]
        ax.plot(x, y, color=COLORS[ds], marker=MARKERS[ds],
                linestyle=LINE_STYLES[ds], label=LABELS[ds],
                linewidth=2.2, markersize=8)

    # Trie
    if trie_data:
        tx = [r['DataSize'] for r in trie_data]
        ty = [r[trie_col]   for r in trie_data]
        ax.plot(tx, ty, color=COLORS['Trie'], marker=MARKERS['Trie'],
                linestyle=LINE_STYLES['Trie'], label='Trie',
                linewidth=2.5, markersize=10)

    ax.set_xlabel('Dataset Size (N)', fontsize=13)
    ax.set_ylabel(ylabel, fontsize=13)
    ax.set_title(title, fontsize=15, fontweight='bold')
    ax.legend(fontsize=10, loc='best', framealpha=0.9)
    ax.grid(True, alpha=0.25, linestyle='--')
    ax.set_xscale('log')

    _save(fig, filepath)

# ─────────────────────────────────────────────
#  Plot 10: Radar / Spider chart
# ─────────────────────────────────────────────

def plot_radar(prefix):
    """
    Capability radar chart for all 7 DS.
    Scores are fixed theoretical/empirical values (0-10).
    Axes: Insert, Search, Range, Delete, Prefix, Memory, Sorted-input Safety
    """
    if not HAS_MATPLOTLIB:
        return

    categories = ['Insert', 'Search', 'Range\nQuery', 'Delete',
                  'Prefix\nSearch', 'Memory\nEfficiency', 'Sorted\nInput Safety']
    N = len(categories)

    # Scores [0..10] for each category
    scores = {
        'BPlusTree': [7,  9,  10, 7,  2,  7,  9],
        'SkipList':  [9,  8,  7,  8,  2,  6,  8],
        'AVLTree':   [7,  9,  7,  7,  2,  8,  9],
        'BST':       [8,  7,  5,  7,  2,  9,  2],
        'SplayTree': [8,  8,  7,  8,  2,  8,  7],
        'Treap':     [9,  8,  7,  8,  2,  8,  8],
        'Trie':      [9,  10, 4,  8,  10, 5,  10],
    }

    angles = [n / float(N) * 2 * 3.14159 for n in range(N)]
    angles += angles[:1]

    fig, ax = plt.subplots(figsize=(9, 9), subplot_kw=dict(polar=True))

    for ds in DS_ORDER:
        vals = scores[ds] + scores[ds][:1]
        ax.plot(angles, vals, color=COLORS[ds], linewidth=2,
                linestyle=LINE_STYLES[ds], label=LABELS[ds])
        ax.fill(angles, vals, color=COLORS[ds], alpha=0.08)

    ax.set_xticks(angles[:-1])
    ax.set_xticklabels(categories, fontsize=11)
    ax.set_yticks([2, 4, 6, 8, 10])
    ax.set_yticklabels(['2', '4', '6', '8', '10'], fontsize=8)
    ax.set_ylim(0, 10)
    ax.legend(loc='upper right', bbox_to_anchor=(1.35, 1.15), fontsize=10)
    ax.set_title('Data Structure Capability Fingerprint\n(All 7 Structures)',
                 fontsize=14, fontweight='bold', y=1.08)

    _save(fig, f'{prefix}_radar.png')

# ─────────────────────────────────────────────
#  Plot 11: Heatmap
# ─────────────────────────────────────────────

def plot_heatmap(data, trie_data, sizes, prefix):
    """
    Heatmap: rows = 7 DS, cols = 5 operations.
    Values are normalized insert/search times at the largest size.
    """
    if not HAS_MATPLOTLIB or not sizes:
        return

    try:
        import numpy as np
    except ImportError:
        return

    ops   = ['Insert', 'Search', 'Range', 'Height', 'Traversals']
    cols  = ['InsertTime_ms', 'SearchTime_ms', 'RangeTime_ms', 'Height', 'Traversals']
    ds_rows = DS_ORDER

    large = sizes[-1]

    raw = []
    for ds in ds_rows[:-1]:   # 6 int-key DS
        row = []
        for col in cols:
            pts = data.get(col, {}).get(ds, [])
            val = next((p[1] for p in pts if p[0] == large), 0.0)
            row.append(val)
        raw.append(row)

    # Trie row — map comparable metrics
    if trie_data:
        td = next((r for r in reversed(trie_data) if r['DataSize'] <= large), trie_data[-1])
        trie_row = [
            td.get('InsertTime_ms', 0),
            td.get('SearchTime_ms', 0),
            td.get('PrefixTime_ms', 0),  # closest to range
            td.get('NodeCount', 0) / 1000,   # scale node count
            td.get('DFSTime_ms', 0) * 1000,  # rescale traversal
        ]
    else:
        trie_row = [0] * len(ops)
    raw.append(trie_row)

    matrix = np.array(raw, dtype=float)

    # Normalize each column 0–1
    col_max = matrix.max(axis=0)
    col_max[col_max == 0] = 1
    norm = matrix / col_max

    fig, ax = plt.subplots(figsize=(11, 6))
    im = ax.imshow(norm, cmap='RdYlGn_r', aspect='auto', vmin=0, vmax=1)

    ax.set_xticks(range(len(ops)))
    ax.set_yticks(range(len(ds_rows)))
    ax.set_xticklabels(ops, fontsize=12)
    ax.set_yticklabels([LABELS[ds] for ds in ds_rows], fontsize=12)

    # Annotate cells
    for i in range(len(ds_rows)):
        for j in range(len(ops)):
            raw_val = matrix[i, j]
            txt = f"{raw_val:.1f}" if raw_val >= 1 else f"{raw_val:.3f}"
            ax.text(j, i, txt, ha='center', va='center',
                    fontsize=9,
                    color='white' if norm[i, j] > 0.6 else 'black')

    plt.colorbar(im, ax=ax, label='Relative cost (green=best, red=worst)')
    ax.set_title(f'Performance Heatmap — All 7 DS  (N = {large})',
                 fontsize=14, fontweight='bold', pad=12)
    plt.tight_layout()
    _save(fig, f'{prefix}_heatmap.png')

# ─────────────────────────────────────────────
#  Plot 12: Bar chart — final summary
# ─────────────────────────────────────────────

def plot_summary_bar(data, trie_data, sizes, prefix):
    """Grouped bar chart: Insert + Search time for all 7 DS at largest N."""
    if not HAS_MATPLOTLIB or not sizes:
        return

    large = sizes[-1]
    ins_vals = []
    sch_vals = []

    for ds in INT_DS:
        ins_pts = data.get('InsertTime_ms', {}).get(ds, [])
        sch_pts = data.get('SearchTime_ms', {}).get(ds, [])
        ins_vals.append(next((p[1] for p in ins_pts if p[0] == large), 0))
        sch_vals.append(next((p[1] for p in sch_pts if p[0] == large), 0))

    # Trie
    if trie_data:
        td = next((r for r in reversed(trie_data) if r['DataSize'] <= large), trie_data[-1])
        ins_vals.append(td.get('InsertTime_ms', 0))
        sch_vals.append(td.get('SearchTime_ms', 0))
    else:
        ins_vals.append(0); sch_vals.append(0)

    ds_labels = [LABELS[ds] for ds in DS_ORDER]
    x = range(len(DS_ORDER))

    fig, ax = plt.subplots(figsize=(13, 6))
    width = 0.38
    bars1 = ax.bar([xi - width/2 for xi in x], ins_vals, width,
                   color=[COLORS[ds] for ds in DS_ORDER], alpha=0.9,
                   label='Insert Time (ms)', edgecolor='white', linewidth=0.7)
    bars2 = ax.bar([xi + width/2 for xi in x], sch_vals, width,
                   color=[COLORS[ds] for ds in DS_ORDER], alpha=0.55,
                   label='Search Time (ms)', edgecolor='white', linewidth=0.7,
                   hatch='//')

    ax.set_xticks(list(x))
    ax.set_xticklabels(ds_labels, fontsize=11)
    ax.set_ylabel('Time (ms)', fontsize=12)
    ax.set_title(f'Insert & Search Time — All 7 DS  (N = {large})',
                 fontsize=14, fontweight='bold')
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.25, axis='y', linestyle='--')

    # Value labels on bars
    for bar in bars1:
        h = bar.get_height()
        if h > 0:
            ax.text(bar.get_x() + bar.get_width()/2, h + 0.5,
                    f'{h:.1f}', ha='center', va='bottom', fontsize=8)
    for bar in bars2:
        h = bar.get_height()
        if h > 0:
            ax.text(bar.get_x() + bar.get_width()/2, h + 0.5,
                    f'{h:.1f}', ha='center', va='bottom', fontsize=8)

    _save(fig, f'{prefix}_summary_bar.png')

# ─────────────────────────────────────────────
#  Text fallback
# ─────────────────────────────────────────────

def print_text_table(data, trie_data, sizes):
    print("\n========================================")
    print(" TEXT SUMMARY (matplotlib not available)")
    print("========================================")
    for metric in ['InsertTime_ms', 'SearchTime_ms', 'RangeTime_ms', 'Height']:
        print(f"\n--- {metric} ---")
        print(f"{'Size':>8}", end="")
        for ds in INT_DS:
            print(f"  {LABELS[ds]:>11}", end="")
        print(f"  {'Trie':>11}")
        for size in sizes:
            print(f"{size:>8}", end="")
            for ds in INT_DS:
                pts = data.get(metric, {}).get(ds, [])
                val = next((p[1] for p in pts if p[0] == size), 0)
                print(f"  {val:>11.4f}", end="")
            print()

# ─────────────────────────────────────────────
#  Main orchestrator
# ─────────────────────────────────────────────

def generate_all_plots(csv_file, trie_csv='trie_benchmark.csv'):
    data, sizes = parse_int_csv(csv_file)
    trie_data   = parse_trie_csv(trie_csv)
    prefix      = os.path.splitext(csv_file)[0]

    print(f"\nGenerating graphs from: {csv_file}")
    print(f"Trie CSV: {trie_csv if trie_data else 'NOT FOUND (trie charts skipped)'}")
    print(f"Dataset sizes: {sizes}\n")

    if not HAS_MATPLOTLIB:
        print_text_table(data, trie_data, sizes)
        return

    # ── Plots 1–5: individual metrics (6 DS) ──
    plot_metric(data, 'InsertTime_ms',
                'Insert Time vs Dataset Size', 'Insert Time (ms)',
                f'{prefix}_insert_time.png', sizes)
    plot_metric(data, 'SearchTime_ms',
                'Search Time vs Dataset Size', 'Search Time (ms)',
                f'{prefix}_search_time.png', sizes)
    plot_metric(data, 'RangeTime_ms',
                'Range Query Time vs Dataset Size', 'Range Time (ms)',
                f'{prefix}_range_time.png', sizes)
    plot_metric(data, 'Height',
                'Structure Height vs Dataset Size', 'Height',
                f'{prefix}_height.png', sizes)
    plot_metric(data, 'Traversals',
                'Node Traversals vs Dataset Size', 'Total Traversals',
                f'{prefix}_traversals.png', sizes)

    # ── Plot 6: combined timing 3-panel ──
    plot_combined(data, prefix)

    # ── Plot 7: Trie operations ──
    plot_trie_ops(trie_data, prefix)

    # ── Plots 8 & 9: all 7 DS insert + search ──
    plot_all7_metric(data, trie_data,
                     'InsertTime_ms', 'InsertTime_ms',
                     'Insert Time — All 7 Data Structures', 'Insert Time (ms)',
                     f'{prefix}_all7_insert.png')
    plot_all7_metric(data, trie_data,
                     'SearchTime_ms', 'SearchTime_ms',
                     'Search Time — All 7 Data Structures', 'Search Time (ms)',
                     f'{prefix}_all7_search.png')

    # ── Plot 10: radar ──
    plot_radar(prefix)

    # ── Plot 11: heatmap ──
    plot_heatmap(data, trie_data, sizes, prefix)

    # ── Plot 12: summary bar ──
    plot_summary_bar(data, trie_data, sizes, prefix)

    print(f"\n[DONE] All graphs saved with prefix '{prefix}_*'\n")


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python plot_results.py <csv_file> [trie_csv]")
        print("  csv_file  — benchmark_random.csv or benchmark_sorted.csv")
        print("  trie_csv  — trie_benchmark.csv (optional, auto-detected)")
        sys.exit(1)

    csv_file = sys.argv[1]
    trie_csv = sys.argv[2] if len(sys.argv) > 2 else 'trie_benchmark.csv'

    if not os.path.exists(csv_file):
        print(f"Error: '{csv_file}' not found.")
        print("Run first: .\\dscompare.exe benchmark")
        sys.exit(1)

    generate_all_plots(csv_file, trie_csv)
