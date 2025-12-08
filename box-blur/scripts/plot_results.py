#!/usr/bin/env python3
"""
Performance Graph Generator
Reads benchmark CSV and creates comparison graphs
"""

import sys
import csv
import matplotlib.pyplot as plt
import numpy as np
from typing import List, Tuple

def normalize_label(label: str) -> str:
    return label.strip()

def order_results(impls: List[str], times: List[float], speeds: List[float]) -> Tuple[List[str], List[float], List[float]]:
    """Order implementations to ensure Serial, OpenMP (1-8), MPI (1-4) appear if present."""
    desired_order = [
        "Serial",
        "OpenMP-1T", "OpenMP-2T", "OpenMP-3T", "OpenMP-4T", "OpenMP-5T", "OpenMP-6T", "OpenMP-7T", "OpenMP-8T",
        "MPI-1P", "MPI-2P", "MPI-3P", "MPI-4P"
    ]

    data = {impl: (t, s) for impl, t, s in zip(impls, times, speeds)}

    ordered_impls = []
    ordered_times = []
    ordered_speeds = []
    missing = []

    for key in desired_order:
        if key in data:
            ordered_impls.append(key)
            t, s = data[key]
            ordered_times.append(t)
            ordered_speeds.append(s)
        else:
            missing.append(key)

    for key in impls:
        if key not in desired_order:
            ordered_impls.append(key)
            ordered_times.append(data[key][0])
            ordered_speeds.append(data[key][1])

    if missing:
        print(f"Warning: missing implementations in CSV: {', '.join(missing)}")

    return ordered_impls, ordered_times, ordered_speeds

def read_results(csv_file):
    """Read benchmark results from CSV file"""
    implementations = []
    times = []
    speeds = []
    
    try:
        with open(csv_file, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                try:
                    impl_label = normalize_label(row['Implementation'])
                    time_val = float(row['Time(s)'])
                    speed_val = float(row['Speed(Mpx/s)'])
                    implementations.append(impl_label)
                    times.append(time_val)
                    speeds.append(speed_val)
                except (ValueError, KeyError):
                    print(f"Skipping invalid row: {row.get('Implementation', 'unknown')}")
                    continue
    except Exception as e:
        print(f"Error reading CSV: {e}")
        return None, None, None
    
    if not implementations:
        return None, None, None

    return order_results(implementations, times, speeds)

def calculate_speedup(times):
    """Calculate speedup relative to serial implementation"""
    if not times or times[0] == 0:
        return []
    baseline = times[0]  # Serial is first
    return [baseline / t for t in times]

def save_bar_plot(values, labels, ylabel, title, outfile, value_fmt, baseline_line=None):
    """Save a single bar plot to file."""
    colors = [
        '#3498db', '#e74c3c', '#2ecc71', '#f39c12', '#9b59b6', '#16a085', '#8e44ad',
        '#1abc9c', '#d35400', '#7f8c8d', '#2c3e50', '#c0392b'
    ]
    x_pos = np.arange(len(labels))

    fig, ax = plt.subplots(figsize=(8, 5))
    ax.bar(x_pos, values, color=colors[:len(labels)], alpha=0.8, edgecolor='black')
    ax.set_xlabel('Implementation', fontweight='bold')
    ax.set_ylabel(ylabel, fontweight='bold')
    ax.set_title(title, fontsize=13, fontweight='bold')
    ax.set_xticks(x_pos)
    ax.set_xticklabels(labels, rotation=35, ha='right')
    ax.grid(axis='y', alpha=0.3)

    if baseline_line is not None:
        ax.axhline(y=baseline_line, color='red', linestyle='--', linewidth=2, label='Serial baseline')
        ax.legend()

    # Add value labels
    offset = max(values) * 0.02 if max(values) else 0.01
    for i, v in enumerate(values):
        ax.text(i, v + offset, value_fmt.format(v), ha='center', va='bottom', fontweight='bold')

    plt.tight_layout()
    plt.savefig(outfile, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f"✓ Saved: {outfile}")


def create_graphs(implementations, times, speeds):
    """Create separate comparison graphs for time, throughput, and speedup."""
    speedup = calculate_speedup(times)

    # Save three separate plots for higher resolution usage in the report
    save_bar_plot(
        times,
        implementations,
        ylabel='Time (seconds)',
        title='Execution Time (Lower is Better)',
        outfile='results/performance_time.png',
        value_fmt='{:.4f}s'
    )

    save_bar_plot(
        speeds,
        implementations,
        ylabel='Throughput (Mpixels/sec)',
        title='Processing Speed (Higher is Better)',
        outfile='results/performance_throughput.png',
        value_fmt='{:.2f}'
    )

    save_bar_plot(
        speedup,
        implementations,
        ylabel='Speedup vs Serial',
        title='Speedup Factor (Higher is Better)',
        outfile='results/performance_speedup.png',
        value_fmt='{:.2f}×',
        baseline_line=1
    )

    # Also create a simple summary table
    create_summary_table(implementations, times, speeds, speedup)

def create_summary_table(implementations, times, speeds, speedup):
    """Create a text summary table"""
    
    output_file = 'results/performance_summary.txt'
    
    with open(output_file, 'w') as f:
        f.write("=" * 80 + "\n")
        f.write("BOX BLUR PERFORMANCE SUMMARY\n")
        f.write("=" * 80 + "\n\n")
        
        f.write(f"{'Implementation':<15} {'Time (s)':<12} {'Speed (Mpx/s)':<15} {'Speedup':<10}\n")
        f.write("-" * 80 + "\n")
        
        for i, impl in enumerate(implementations):
            f.write(f"{impl:<15} {times[i]:<12.6f} {speeds[i]:<15.2f} {speedup[i]:<10.2f}x\n")
        
        f.write("\n" + "=" * 80 + "\n")
        f.write("KEY FINDINGS:\n")
        f.write("=" * 80 + "\n")
        
        # Find best performer
        fastest_idx = times.index(min(times))
        f.write(f"• Fastest: {implementations[fastest_idx]} ({times[fastest_idx]:.6f}s)\n")
        f.write(f"• Best speedup: {speedup[fastest_idx]:.2f}x faster than serial\n")
        
        if len(implementations) > 1:
            serial_time = times[0]
            parallel_times = times[1:]
            avg_parallel = sum(parallel_times) / len(parallel_times)
            avg_speedup = serial_time / avg_parallel
            f.write(f"• Average parallel speedup: {avg_speedup:.2f}x\n")
        
        f.write("\n")
    
    print(f"Summary saved: {output_file}")

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 plot_results.py <benchmark_results.csv>")
        sys.exit(1)
    
    csv_file = sys.argv[1]
    
    print("Reading benchmark results...")
    implementations, times, speeds = read_results(csv_file)
    
    if not implementations:
        print("No data to plot!")
        sys.exit(1)
    
    print(f"Found {len(implementations)} implementations")
    print("Creating graphs...")
    
    create_graphs(implementations, times, speeds)
    
    print("\nDone!")

if __name__ == '__main__':
    main()
