import os
import pandas as pd
import matplotlib.pyplot as plt

def load_results(result_dir):
    results = {}
    for filename in os.listdir(result_dir):
        if filename.endswith('.csv'):
            filepath = os.path.join(result_dir, filename)
            results[filename] = pd.read_csv(filepath)
    return results

def compare_performance(results):
    comparison = {}
    for key, df in results.items():
        comparison[key] = df['execution_time'].mean()  # Assuming 'execution_time' is a column in the CSV
    return comparison

def plot_results(comparison):
    implementations = list(comparison.keys())
    times = list(comparison.values())

    plt.bar(implementations, times, color=['blue', 'orange', 'green', 'red'])
    plt.xlabel('Implementations')
    plt.ylabel('Average Execution Time (seconds)')
    plt.title('Performance Comparison of Box Blur Implementations')
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig('performance_comparison.png')
    plt.show()

def main():
    result_dir = '../results/performance_metrics'
    results = load_results(result_dir)
    comparison = compare_performance(results)
    plot_results(comparison)

if __name__ == '__main__':
    main()