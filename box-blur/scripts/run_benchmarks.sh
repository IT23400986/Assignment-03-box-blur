#!/bin/bash

# Box Blur Benchmark Script
# Runs all implementations and collects performance data

RESULTS_DIR="results/performance_metrics"
DATA_DIR="data/sample_images"
OUTPUT_DIR="results/output_images"

# Create directories if they don't exist
mkdir -p "$RESULTS_DIR"
mkdir -p "$OUTPUT_DIR"

# Results file
RESULTS_FILE="$RESULTS_DIR/benchmark_results.csv"

echo "=== Box Blur Benchmark Suite ==="
echo "======================================"
echo ""

# Initialize results file
echo "Implementation,ImageSize,Width,Height,Pixels,Threads/Processes,ExecutionTime(s),Throughput(Mpx/s)" > "$RESULTS_FILE"

# Test images to use
IMAGES=(
    "gradient_256x256.pgm"
    "gradient_512x512.pgm"
    "gradient_1024x1024.pgm"
    "checkerboard_512x512.pgm"
    "circle_512x512.pgm"
    "rings_1024x1024.pgm"
)

# Function to extract execution time from output
extract_time() {
    grep "Execution time:" | awk '{print $3}'
}

# Function to extract image dimensions
get_dimensions() {
    local filename=$1
    echo "$filename" | grep -oP '\d+x\d+' | head -1
}

echo "Running Serial implementation..."
echo "================================"
if [ -f "./serial_box_blur" ]; then
    for img in "${IMAGES[@]}"; do
        if [ -f "$DATA_DIR/$img" ]; then
            echo "Processing $img..."
            output="$OUTPUT_DIR/serial_${img}"
            
            # Run and capture output
            result=$(./serial_box_blur "$DATA_DIR/$img" "$output" 2>&1)
            time=$(echo "$result" | extract_time)
            dims=$(get_dimensions "$img")
            width=$(echo "$dims" | cut -d'x' -f1)
            height=$(echo "$dims" | cut -d'x' -f2)
            pixels=$((width * height))
            throughput=$(echo "scale=2; $pixels / ($time * 1000000)" | bc)
            
            echo "Serial,$img,$width,$height,$pixels,1,$time,$throughput" >> "$RESULTS_FILE"
            echo "  Time: ${time}s"
        fi
    done
    echo ""
else
    echo "Serial implementation not found. Skipping..."
    echo ""
fi

echo "Running OpenMP implementation..."
echo "================================"
if [ -f "./openmp_box_blur" ]; then
    for threads in 1 2 4 8; do
        export OMP_NUM_THREADS=$threads
        echo "Testing with $threads thread(s)..."
        
        for img in "${IMAGES[@]}"; do
            if [ -f "$DATA_DIR/$img" ]; then
                echo "  Processing $img..."
                output="$OUTPUT_DIR/openmp_${threads}t_${img}"
                
                result=$(./openmp_box_blur "$DATA_DIR/$img" "$output" 2>&1)
                time=$(echo "$result" | extract_time)
                dims=$(get_dimensions "$img")
                width=$(echo "$dims" | cut -d'x' -f1)
                height=$(echo "$dims" | cut -d'x' -f2)
                pixels=$((width * height))
                throughput=$(echo "scale=2; $pixels / ($time * 1000000)" | bc)
                
                echo "OpenMP,$img,$width,$height,$pixels,$threads,$time,$throughput" >> "$RESULTS_FILE"
                echo "    Time: ${time}s"
            fi
        done
    done
    echo ""
else
    echo "OpenMP implementation not found. Skipping..."
    echo ""
fi

echo "Running MPI implementation..."
echo "============================="
if [ -f "./mpi_box_blur" ] && command -v mpirun &> /dev/null; then
    for procs in 1 2 4 8; do
        echo "Testing with $procs process(es)..."
        
        for img in "${IMAGES[@]}"; do
            if [ -f "$DATA_DIR/$img" ]; then
                echo "  Processing $img..."
                output="$OUTPUT_DIR/mpi_${procs}p_${img}"
                
                result=$(mpirun -np $procs ./mpi_box_blur "$DATA_DIR/$img" "$output" 2>&1)
                time=$(echo "$result" | extract_time)
                dims=$(get_dimensions "$img")
                width=$(echo "$dims" | cut -d'x' -f1)
                height=$(echo "$dims" | cut -d'x' -f2)
                pixels=$((width * height))
                throughput=$(echo "scale=2; $pixels / ($time * 1000000)" | bc)
                
                echo "MPI,$img,$width,$height,$pixels,$procs,$time,$throughput" >> "$RESULTS_FILE"
                echo "    Time: ${time}s"
            fi
        done
    done
    echo ""
else
    echo "MPI implementation not found or mpirun not available. Skipping..."
    echo ""
fi

echo "Running OpenCL implementation..."
echo "================================"
if [ -f "./opencl_box_blur" ]; then
    for img in "${IMAGES[@]}"; do
        if [ -f "$DATA_DIR/$img" ]; then
            echo "Processing $img..."
            output="$OUTPUT_DIR/opencl_${img}"
            
            result=$(./opencl_box_blur "$DATA_DIR/$img" "$output" 2>&1)
            time=$(echo "$result" | extract_time)
            dims=$(get_dimensions "$img")
            width=$(echo "$dims" | cut -d'x' -f1)
            height=$(echo "$dims" | cut -d'x' -f2)
            pixels=$((width * height))
            throughput=$(echo "scale=2; $pixels / ($time * 1000000)" | bc)
            
            echo "OpenCL,$img,$width,$height,$pixels,GPU,$time,$throughput" >> "$RESULTS_FILE"
            echo "  Time: ${time}s"
        fi
    done
    echo ""
else
    echo "OpenCL implementation not found. Skipping..."
    echo ""
fi

echo "======================================"
echo "Benchmark complete!"
echo "Results saved to: $RESULTS_FILE"
echo ""
echo "Run 'python scripts/compare_results.py' to analyze results"
