#!/usr/bin/env python3
"""
OpenMP Thread Count Analysis - Generate required graphs for marking criteria
Creates:
1. Number of threads vs Execution time
2. Number of threads vs Speedup
"""

import csv
import matplotlib.pyplot as plt
import numpy as np

def read_openmp_results(csv_file):
    """Read OpenMP results from benchmark CSV"""
    threads = []
    times = []
    
    try:
        with open(csv_file, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                impl = row['Implementation']
                # Extract OpenMP entries only (format: OpenMP-XT where X is thread count)
                if impl.startswith('OpenMP-') and impl.endswith('T'):
                    try:
                        # Extract thread count from "OpenMP-4T" -> 4
                        thread_count = int(impl.split('-')[1].replace('T', ''))
                        time_val = float(row['Time(s)'])
                        threads.append(thread_count)
                        times.append(time_val)
                    except (ValueError, IndexError):
                        continue
    except Exception as e:
        print(f"Error reading CSV: {e}")
        return None, None
    
    # Sort by thread count
    if threads and times:
        sorted_pairs = sorted(zip(threads, times))
        threads, times = zip(*sorted_pairs)
        return list(threads), list(times)
    
    return None, None

def get_serial_time(csv_file):
    """Get serial baseline time for speedup calculation"""
    try:
        with open(csv_file, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                if row['Implementation'] == 'Serial':
                    return float(row['Time(s)'])
    except Exception as e:
        print(f"Error reading serial time: {e}")
    return None

def create_openmp_graphs(threads, times, serial_time):
    """Create OpenMP-specific analysis graphs"""
    
    # Calculate speedup
    speedup = [serial_time / t for t in times]
    
    # Create figure with 2 subplots side by side
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
    fig.suptitle('OpenMP Performance Analysis - Thread Count Scaling', fontsize=16, fontweight='bold')
    
    # Graph 1: Threads vs Execution Time
    ax1.plot(threads, times, marker='o', linewidth=2, markersize=8, color='#3498db')
    ax1.set_xlabel('Number of Threads', fontweight='bold', fontsize=12)
    ax1.set_ylabel('Execution Time (seconds)', fontweight='bold', fontsize=12)
    ax1.set_title('Threads vs Execution Time\n(Lower is Better)', fontsize=13)
    ax1.grid(True, alpha=0.3)
    ax1.set_xticks(threads)
    
    # Add value labels
    for i, (t, time_val) in enumerate(zip(threads, times)):
        ax1.annotate(f'{time_val:.4f}s', 
                     xy=(t, time_val), 
                     xytext=(0, 10), 
                     textcoords='offset points',
                     ha='center',
                     fontweight='bold')
    
    # Graph 2: Threads vs Speedup
    ax2.plot(threads, speedup, marker='s', linewidth=2, markersize=8, color='#2ecc71')
    ax2.axhline(y=1, color='red', linestyle='--', linewidth=2, label='Serial baseline (1×)')
    
    # Add ideal linear speedup line
    ideal_speedup = threads
    ax2.plot(threads, ideal_speedup, linestyle=':', linewidth=2, color='gray', alpha=0.7, label='Ideal linear speedup')
    
    ax2.set_xlabel('Number of Threads', fontweight='bold', fontsize=12)
    ax2.set_ylabel('Speedup vs Serial', fontweight='bold', fontsize=12)
    ax2.set_title('Threads vs Speedup\n(Higher is Better)', fontsize=13)
    ax2.grid(True, alpha=0.3)
    ax2.set_xticks(threads)
    ax2.legend(loc='upper left')
    
    # Add value labels
    for i, (t, sp) in enumerate(zip(threads, speedup)):
        ax2.annotate(f'{sp:.2f}×', 
                     xy=(t, sp), 
                     xytext=(0, 10), 
                     textcoords='offset points',
                     ha='center',
                     fontweight='bold')
    
    plt.tight_layout()
    
    # Save the figure
    output_file = 'results/openmp_thread_analysis.png'
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"✓ OpenMP analysis graph saved: {output_file}")
    
    # Create efficiency graph as bonus
    create_efficiency_graph(threads, speedup)
    
    # Save summary table
    save_openmp_summary(threads, times, speedup, serial_time)

def create_efficiency_graph(threads, speedup):
    """Create efficiency graph (Speedup / Threads * 100)"""
    efficiency = [(sp / t) * 100 for sp, t in zip(speedup, threads)]
    
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(threads, efficiency, marker='D', linewidth=2, markersize=8, color='#e74c3c')
    ax.axhline(y=100, color='green', linestyle='--', linewidth=2, label='100% efficiency (ideal)')
    
    ax.set_xlabel('Number of Threads', fontweight='bold', fontsize=12)
    ax.set_ylabel('Parallel Efficiency (%)', fontweight='bold', fontsize=12)
    ax.set_title('OpenMP Parallel Efficiency', fontsize=14, fontweight='bold')
    ax.grid(True, alpha=0.3)
    ax.set_xticks(threads)
    ax.legend()
    
    # Add value labels
    for t, eff in zip(threads, efficiency):
        ax.annotate(f'{eff:.1f}%', 
                    xy=(t, eff), 
                    xytext=(0, 10), 
                    textcoords='offset points',
                    ha='center',
                    fontweight='bold')
    
    plt.tight_layout()
    
    output_file = 'results/openmp_efficiency.png'
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"✓ Efficiency graph saved: {output_file}")

def save_openmp_summary(threads, times, speedup, serial_time):
    """Save OpenMP analysis summary table"""
    efficiency = [(sp / t) * 100 for sp, t in zip(speedup, threads)]
    
    output_file = 'results/openmp_analysis_summary.txt'
    
    with open(output_file, 'w') as f:
        f.write("=" * 80 + "\n")
        f.write("OPENMP THREAD COUNT ANALYSIS\n")
        f.write("=" * 80 + "\n\n")
        f.write(f"Serial Baseline Time: {serial_time:.6f} seconds\n\n")
        
        f.write(f"{'Threads':<10} {'Time (s)':<15} {'Speedup':<15} {'Efficiency (%)':<15}\n")
        f.write("-" * 80 + "\n")
        
        for t, time_val, sp, eff in zip(threads, times, speedup, efficiency):
            f.write(f"{t:<10} {time_val:<15.6f} {sp:<15.2f}× {eff:<15.1f}%\n")
        
        f.write("\n" + "=" * 80 + "\n")
        f.write("KEY FINDINGS:\n")
        f.write("=" * 80 + "\n")
        
        # Best speedup
        best_idx = speedup.index(max(speedup))
        f.write(f"• Best speedup: {threads[best_idx]} threads → {speedup[best_idx]:.2f}×\n")
        f.write(f"• Best efficiency: {threads[efficiency.index(max(efficiency))]} threads → {max(efficiency):.1f}%\n")
        f.write(f"• Fastest execution: {threads[times.index(min(times))]} threads → {min(times):.6f}s\n")
        
        # Analysis
        f.write("\nSCALING ANALYSIS:\n")
        for i in range(len(threads)-1):
            improvement = ((times[i] - times[i+1]) / times[i]) * 100
            f.write(f"• {threads[i]}T → {threads[i+1]}T: {improvement:+.1f}% improvement\n")
        
        f.write("\n")
    
    print(f"✓ Summary saved: {output_file}")

def main():
    csv_file = 'results/benchmark_results.csv'
    
    print("=" * 60)
    print("OpenMP Thread Count Analysis Tool")
    print("=" * 60)
    print(f"\nReading benchmark results from: {csv_file}")
    
    threads, times = read_openmp_results(csv_file)
    serial_time = get_serial_time(csv_file)
    
    if not threads or not serial_time:
        print("\n❌ Error: Could not find OpenMP or Serial results in CSV!")
        print("   Make sure benchmark.sh has been run first.")
        return
    
    print(f"✓ Found OpenMP results for {len(threads)} thread configurations: {threads}")
    print(f"✓ Serial baseline: {serial_time:.6f}s\n")
    
    print("Generating graphs...")
    create_openmp_graphs(threads, times, serial_time)
    
    print("\n" + "=" * 60)
    print("✓ OpenMP analysis complete!")
    print("=" * 60)
    print("\nGenerated files:")
    print("  1. results/openmp_thread_analysis.png   (Threads vs Time & Speedup)")
    print("  2. results/openmp_efficiency.png        (Efficiency graph)")
    print("  3. results/openmp_analysis_summary.txt  (Detailed table)")
    print("\n")

if __name__ == '__main__':
    main()
