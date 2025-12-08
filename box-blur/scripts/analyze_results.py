#!/usr/bin/env python3
"""
Box Blur Performance Analysis Script

Analyzes benchmark results and generates performance comparison charts
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys
import os

def load_results(filename):
    """Load benchmark results from CSV file"""
    try:
        df = pd.read_csv(filename)
        return df
    except FileNotFoundError:
        print(f"Error: Results file '{filename}' not found.")
        print("Run './scripts/run_benchmarks.sh' first to generate results.")
        sys.exit(1)

def calculate_speedup(df, baseline='Serial'):
    """Calculate speedup relative to serial implementation"""
    results = []
    
    # Get unique image sizes
    images = df['ImageSize'].unique()
    
    for img in images:
        img_data = df[df['ImageSize'] == img]
        
        # Get serial baseline time
        serial_time = img_data[img_data['Implementation'] == baseline]['ExecutionTime(s)'].values
        
        if len(serial_time) == 0:
            continue
            
        serial_time = serial_time[0]
        
        for _, row in img_data.iterrows():
            speedup = serial_time / row['ExecutionTime(s)']
            results.append({
                'Implementation': row['Implementation'],
                'ImageSize': row['ImageSize'],
                'Pixels': row['Pixels'],
                'Threads/Processes': row['Threads/Processes'],
                'ExecutionTime': row['ExecutionTime(s)'],
                'Speedup': speedup
            })
    
    return pd.DataFrame(results)

def plot_execution_times(df, output_dir):
    """Plot execution times for different implementations"""
    fig, ax = plt.subplots(figsize=(12, 6))
    
    images = df['ImageSize'].unique()
    implementations = df['Implementation'].unique()
    
    x = np.arange(len(images))
    width = 0.15
    
    for i, impl in enumerate(implementations):
        impl_data = df[df['Implementation'] == impl]
        times = [impl_data[impl_data['ImageSize'] == img]['ExecutionTime(s)'].mean() 
                 for img in images]
        ax.bar(x + i * width, times, width, label=impl)
    
    ax.set_xlabel('Image Size')
    ax.set_ylabel('Execution Time (seconds)')
    ax.set_title('Execution Time Comparison Across Implementations')
    ax.set_xticks(x + width * (len(implementations) - 1) / 2)
    ax.set_xticklabels(images, rotation=45, ha='right')
    ax.legend()
    ax.grid(axis='y', alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(f'{output_dir}/execution_times.png', dpi=300)
    print(f"Saved: {output_dir}/execution_times.png")
    plt.close()

def plot_speedup(speedup_df, output_dir):
    """Plot speedup comparison"""
    fig, ax = plt.subplots(figsize=(12, 6))
    
    # Focus on a specific image size for clarity
    target_img = speedup_df['ImageSize'].mode()[0]  # Most common image size
    data = speedup_df[speedup_df['ImageSize'] == target_img]
    
    implementations = data['Implementation'].unique()
    x = np.arange(len(implementations))
    
    speedups = [data[data['Implementation'] == impl]['Speedup'].max() 
                for impl in implementations]
    
    bars = ax.bar(x, speedups, color=['#3498db', '#2ecc71', '#e74c3c', '#f39c12'])
    ax.axhline(y=1, color='r', linestyle='--', label='Serial Baseline')
    
    ax.set_xlabel('Implementation')
    ax.set_ylabel('Speedup (vs Serial)')
    ax.set_title(f'Maximum Speedup Comparison ({target_img})')
    ax.set_xticks(x)
    ax.set_xticklabels(implementations)
    ax.legend()
    ax.grid(axis='y', alpha=0.3)
    
    # Add value labels on bars
    for bar in bars:
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height,
                f'{height:.2f}x',
                ha='center', va='bottom', fontweight='bold')
    
    plt.tight_layout()
    plt.savefig(f'{output_dir}/speedup_comparison.png', dpi=300)
    print(f"Saved: {output_dir}/speedup_comparison.png")
    plt.close()

def plot_scaling(df, output_dir):
    """Plot scaling behavior for OpenMP and MPI"""
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
    
    # OpenMP scaling
    openmp_data = df[df['Implementation'] == 'OpenMP']
    if not openmp_data.empty:
        target_img = openmp_data['ImageSize'].mode()[0]
        omp_img_data = openmp_data[openmp_data['ImageSize'] == target_img]
        
        threads = omp_img_data['Threads/Processes'].astype(int).unique()
        threads.sort()
        times = [omp_img_data[omp_img_data['Threads/Processes'] == t]['ExecutionTime(s)'].mean() 
                 for t in threads]
        
        ax1.plot(threads, times, marker='o', linewidth=2, markersize=8)
        ax1.set_xlabel('Number of Threads')
        ax1.set_ylabel('Execution Time (seconds)')
        ax1.set_title(f'OpenMP Strong Scaling ({target_img})')
        ax1.grid(True, alpha=0.3)
        ax1.set_xticks(threads)
    
    # MPI scaling
    mpi_data = df[df['Implementation'] == 'MPI']
    if not mpi_data.empty:
        target_img = mpi_data['ImageSize'].mode()[0]
        mpi_img_data = mpi_data[mpi_data['ImageSize'] == target_img]
        
        procs = mpi_img_data['Threads/Processes'].astype(int).unique()
        procs.sort()
        times = [mpi_img_data[mpi_img_data['Threads/Processes'] == p]['ExecutionTime(s)'].mean() 
                 for p in procs]
        
        ax2.plot(procs, times, marker='s', linewidth=2, markersize=8, color='orange')
        ax2.set_xlabel('Number of Processes')
        ax2.set_ylabel('Execution Time (seconds)')
        ax2.set_title(f'MPI Strong Scaling ({target_img})')
        ax2.grid(True, alpha=0.3)
        ax2.set_xticks(procs)
    
    plt.tight_layout()
    plt.savefig(f'{output_dir}/scaling_analysis.png', dpi=300)
    print(f"Saved: {output_dir}/scaling_analysis.png")
    plt.close()

def generate_summary_table(speedup_df):
    """Generate summary statistics table"""
    print("\n=== Performance Summary ===")
    print("=" * 80)
    
    for impl in speedup_df['Implementation'].unique():
        impl_data = speedup_df[speedup_df['Implementation'] == impl]
        max_speedup = impl_data['Speedup'].max()
        avg_speedup = impl_data['Speedup'].mean()
        
        print(f"\n{impl}:")
        print(f"  Maximum Speedup: {max_speedup:.2f}x")
        print(f"  Average Speedup: {avg_speedup:.2f}x")
        
        if impl in ['OpenMP', 'MPI']:
            best_config = impl_data.loc[impl_data['Speedup'].idxmax()]
            print(f"  Best Configuration: {int(best_config['Threads/Processes'])} "
                  f"{'threads' if impl == 'OpenMP' else 'processes'}")
    
    print("\n" + "=" * 80)

def main():
    results_file = 'results/performance_metrics/benchmark_results.csv'
    output_dir = 'results/performance_metrics'
    
    print("=== Box Blur Performance Analysis ===\n")
    
    # Load results
    print(f"Loading results from: {results_file}")
    df = load_results(results_file)
    print(f"Loaded {len(df)} benchmark records\n")
    
    # Calculate speedup
    print("Calculating speedup metrics...")
    speedup_df = calculate_speedup(df)
    
    # Generate plots
    print("\nGenerating performance charts...")
    plot_execution_times(df, output_dir)
    plot_speedup(speedup_df, output_dir)
    plot_scaling(df, output_dir)
    
    # Generate summary
    generate_summary_table(speedup_df)
    
    # Save detailed results
    speedup_file = f'{output_dir}/speedup_analysis.csv'
    speedup_df.to_csv(speedup_file, index=False)
    print(f"\nDetailed speedup analysis saved to: {speedup_file}")
    
    print("\n✓ Analysis complete!")
    print(f"  - Charts saved in: {output_dir}/")
    print(f"  - View PNG files for visualizations")

if __name__ == '__main__':
    main()
